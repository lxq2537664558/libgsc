package net;

import java.io.IOException;
import java.net.StandardSocketOptions;
import java.nio.ByteBuffer;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.Pipe;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.nio.channels.ServerSocketChannel;
import java.nio.channels.SocketChannel;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map.Entry;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.function.Consumer;

import misc.Log;
import misc.Misc;
import misc.Net;
import stmp.Stmp;
import stmp.StmpDec;
import stmp.StmpNode;
import actor.Actor;
import actor.ActorNet;
import actor.ActorNet.NetProtocol;
import actor.H2N;
import actor.N2H;
import core.Cfg;
import core.Gsc;

/**
 * 
 * 描述了一个libgsc的服务端工作线程.
 * 
 * @author xuzewen
 * @time 2015年1月12日 上午11:01:40
 *
 */
public class Gworker extends Actor
{
	/** 线程上的多路复用器. */
	public Selector slt = null;
	/** 用于线程间通信的管道. */
	public Pipe pipe = null;
	/** 用于线程间通信的管道上的缓冲区. */
	public ByteBuffer bb = null;
	/** 工作线程上所有的网络连接. */
	public HashMap<Integer /* SocketChannel.hashCode(). */, ActorNet> ans = new HashMap<>();
	/** 等待处理的Consumer. */
	public ConcurrentLinkedQueue<Consumer<Void>> cs = new ConcurrentLinkedQueue<>();
	/** 通知Gworker的信号. */
	public final ByteBuffer signal = ByteBuffer.allocate(1);
	/** 线程忙. */
	public volatile boolean busy = false;
	/** 用于轮询分配工作线程. */
	public static final AtomicInteger rb = new AtomicInteger(0);

	public Gworker(int wk, Selector slt)
	{
		super(ActorType.ITC);
		this.wk = wk;
		this.slt = slt;
		Gworker g = this;
		new Thread(new Runnable()
		{
			public void run()
			{
				g.run();
			}
		}).start();
	}

	/** 线程入口. */
	public void run()
	{
		this.pipe = this.initPipe();
		if (pipe == null)
			System.exit(1);
		Log.info("libgsc worker thread started successfully, index: %02X, tid: %d", this.wk, Thread.currentThread().getId());
		while (true)
		{
			try
			{
				this.slt.select(); /* wait for event. */
				this.busy = true;
				Iterator<SelectionKey> it = this.slt.selectedKeys().iterator();
				while (it.hasNext())
				{
					SelectionKey key = it.next();
					if (!key.isValid())
						Log.error("it`s a invalid key: %s", key);
					else if (key.isAcceptable())
						this.evnAccept(key);
					else if (key.isReadable())
						this.evnRead(key);
					it.remove();
				}
				if (this.cs.size() > 0)
				{
					Consumer<Void> c = this.cs.poll();
					while (c != null)
					{
						c.accept(null);
						c = this.cs.poll();
					}
				}
				this.busy = false;
			} catch (Exception e)
			{
				Log.error(Log.trace(e));
				Misc.sleep(100);
			}
		}
	}

	/** 前转一个Consumer到当前工作线程. */
	public final void push(Consumer<Void> c)
	{
		try
		{
			this.cs.add(c);
			if (this.busy)
				return;
			synchronized (this)
			{
				this.signal.position(0);
				this.pipe.sink().write(this.signal);
			}
		} catch (IOException e)
		{
			Log.error(Log.trace(e));
		}
	}

	/** 将libgsc服务端套接字句柄注册到当前工作线程. */
	public final boolean regServerSocketChannel(ServerSocketChannel ssc)
	{
		try
		{
			ssc.register(this.slt, SelectionKey.OP_ACCEPT);
			Log.info("registered server-socket channel into gworker[%d] successfully.", this.wk);
			return true;
		} catch (ClosedChannelException e)
		{
			Log.error(Log.trace(e));
			return false;
		}
	}

	/** 连接到达. */
	private final void evnAccept(SelectionKey key)
	{
		try
		{
			ServerSocketChannel ssc = (ServerSocketChannel) key.channel();
			while (true)
			{
				SocketChannel sc = ssc.accept();
				if (sc == null)
					break;
				/**
				 * 
				 * 同C的实现保持一致, 只有一个Gworker处理accept.
				 * 
				 * 但这里无法像C一样拿到套接字的描述字, 亦做不到将某个描述字分配在固定的Gworker上.
				 * 
				 * 这里使用的方法是对所有的Gworker线程进行轮询.
				 * 
				 */
				N2H na = new N2H(sc, ByteBuffer.allocate(Cfg.libgsc_peer_mtu));
				na.wk = Gsc.rrWk(); /* 轮询. */
				Gworker g = Gsc.wks[na.wk];
				g.future(x -> {
					if (Log.isDebug())
						Log.debug("got a connection from: %s", Net.getRemoteAddr(sc));
					try
					{
						g.setSocketOpt(sc);
						sc.register(g.slt, SelectionKey.OP_READ); /* 只关心读事件. */
						g.addActorNet(na);
					} catch (Exception e)
					{
						Log.error(Log.trace(e));
					}
				});
			}
		} catch (IOException e)
		{
			Log.error(Log.trace(e));
		}
	}

	/** 套接字可读. */
	private final void evnRead(SelectionKey key)
	{
		int p = key.channel().hashCode();
		if (p == this.pipe.source().hashCode()) /* 管道上的消息. */
			this.evnReadPipe((Pipe.SourceChannel) key.channel());
		else
			this.evnReadSocket(this.ans.get(key.channel().hashCode()), key);/* 网络报文送达. */
	}

	/** 处理管道上的Consumer送达. */
	private final void evnReadPipe(Pipe.SourceChannel source)
	{
		try
		{
			int ret = source.read(this.bb);
			while (ret > 0)
			{
				this.bb.position(0);
				ret = source.read(this.bb);
			}
			if (ret != 0)
				Log.fault("it`s a bug.");
			Consumer<Void> c = this.cs.poll();
			while (c != null)
			{
				c.accept(null);
				c = this.cs.poll();
			}
		} catch (IOException e)
		{
			Log.fault(Log.trace(e));
		}
	}

	/** 处理网络报文送达. */
	private final void evnReadSocket(ActorNet an, SelectionKey key)
	{
		if (an == null)
		{
			Log.fault("it`s a bug."); /* 这里不应该找不到. */
			return;
		}
		SocketChannel sc = (SocketChannel) key.channel();
		if (an.type == ActorType.N2H)
		{
			if (((N2H) an).lz) /* 一个要延迟关闭的N2H, 不再处理它的上行消息. */
			{
				Net.readAndDicard(sc, an.bb);
				return;
			}
		}
		boolean flag = true;
		try
		{
			while (flag)
			{
				int ret = sc.read(an.bb);
				if (ret == -1 || !this.evnReadMsg(an)) /* 连接已断开或消息处理失败. */
				{
					flag = false;
					break;
				}
				if (!an.est) /* 连接可能已被重置. */
				{
					flag = false;
					break;
				}
				if (ret == 0) /* no more bytes can be read. */
					break;
			}
		} catch (IOException e)
		{
			flag = false;
			if (Log.isTrace())
				Log.trace(Log.trace(e));
		}
		if (!flag)
		{
			Log.debug("have a client disconnected: %s", an);
			if (an.est)
			{
				this.removeActorNet(an);
				an.evnDis();
			} else
				; /* 如果连接已经不在, 则一定是已经调用过evnDis和上面的removeActorNet. */
		}
	}

	/** 处理网络报文送达. */
	private final boolean evnReadMsg(ActorNet an)
	{
		byte by[] = an.bb.array();
		int len = an.bb.position();
		int ofst = 0;
		for (;;)
		{
			if (len < 1)
				break;
			int size = -1;
			if (an.np == NetProtocol.NP_NONE)
			{
				if (by[0] >= Stmp.STMP_TAG_TRANS_BEGIN && by[0] <= Stmp.STMP_TAG_TRANS_UNI) /* STMP-protocol. */
				{
					an.np = NetProtocol.NP_STMP;
					size = this.evnReadMsgStmp(an, by, ofst, len);
				} else if (by[0] == 'G') /** GET. */
				{
					an.np = NetProtocol.NP_WEBSOCKET;
					size = this.evnReadMsgWebSocket(an, by, ofst, len);
				}
			} else if (an.np == NetProtocol.NP_STMP)
				size = this.evnReadMsgStmp(an, by, ofst, len);
			else if (an.np == NetProtocol.NP_WEBSOCKET)
				size = this.evnReadMsgWebSocket(an, by, ofst, len);
			if (size == -1) /* 消息处理异常. */
				return false;
			if (size == 0) /* 不是一个完整的包. */
				break;
			ofst += size;
			len -= size;
		}
		if (len != an.bb.position())
		{
			for (int i = 0; i < len; ++i)
				by[i] = by[i + ofst];
			an.bb.position(len);
		}
		return true;
	}

	/** STMP协议报文. */
	private final int evnReadMsgStmp(ActorNet an, byte by[], int _ofst_, int _len_)
	{
		int ofst = _ofst_;
		int len = _len_;
		for (;;)
		{
			if (len < 3) /* 至少有一个tlv. */
				break;
			if (by[ofst + 1] == 0xFF)
			{
				if (Log.isDebug())
					Log.debug("unsupported over 64K PDU.");
				return -1;
			}
			int l = (by[ofst + 1] == 0xFE ? 3 : 1); /* 获得len字段的长度. */
			if (len < 1 + l) /* 不够一个tag + len, 如65 30, 或65 FE 01 FF. */
				break;
			int size = 0;
			if (((short) (by[ofst + 1] & 0x00FF)) < 0x00FE) /* 一个字节表示长度. */
				size = 1/* tag */+ 1 /* len字段本身 */+ ((short) (by[ofst + 1] & 0x00FF))/* val */;
			else
			{
				int s = Net.byte2short(by, ofst + 2) & 0x0000FFFF; /* 有两字节表示长度. */
				size = 1 + l + s;
			}
			if (size > Cfg.libgsc_peer_mtu)
			{
				if (Log.isDebug())
					Log.debug("packet format error(over the LIBGSC_PEER_MTU), we will close this connection, peer: %s, size: %08X", Net.getRemoteAddr(an.sc), size);
				return -1;
			}
			if (len < size) /* 还未到齐. */
				break;
			//
			StmpNode root = StmpDec.unpack(by, ofst, size);
			if (root == null)
			{
				if (Log.isDebug())
					Log.debug("STMP protocol error, we will close this connection, peer: %s, size: %08X", Net.getRemoteAddr(an.sc), size);
				return -1;
			}
			if (Log.isRecord())
				Log.record("\n  <-- PEER: %s\n%s", Net.getRemoteAddr(an.sc), StmpDec.print2Str(by, ofst, size));
			an.mts = System.currentTimeMillis();
			if (!an.evnMsg(root))
				return -1;
			ofst += size;
			len -= size;
		}
		return _len_ - len;
	}

	/** WebSocket协议报文. */
	private final int evnReadMsgWebSocket(ActorNet an, byte by[], int ofst, int len)
	{
		int o = ofst;
		int l = len;
		if (!an.wshs) /* 还未完成握手. */
		{
			int ret = WebSocket.checkHandShake(an, by, ofst, len); /* 检查握手报文是否完整. */
			if (ret == 0) /* 不是一个完整的握手. */
				return 0;
			if (ret < 1) /* 消息异常. */
				return -1;
			an.wshs = true; /* 握手完成. */
			if (len == ret) /* 刚好一个握手报文. */
				return ret;
			o += ret;
			l -= ret;
		}
		int ret = WebSocket.unpack(an, by, o, l);
		if (ret == 0)
			return o - ofst;
		if (ret == -1)
			return -1;
		return ret + (o - ofst);
	}

	/** 添加一个ActorNet连接. */
	private final void addActorNet(ActorNet an)
	{
		ActorNet old = this.ans.get(an.sc.hashCode());
		if (old != null)
		{
			Log.fault("it`s a bug, an: %s", old);
			return;
		}
		if (an.type == ActorType.H2N)
			this.h2ns.add((H2N) an);
		this.ans.put(an.sc.hashCode(), an);
	}

	/** 添加一个要延迟关闭的N2H. */
	public final void addLazyCloseN2h(N2H n2h)
	{
		this.lazyCloseN2hs.put(n2h.sc.hashCode(), n2h);
	}

	/** 初始化工作线程管道. */
	private final Pipe initPipe()
	{
		try
		{
			Pipe pipe = Pipe.open();
			pipe.source().configureBlocking(false);
			pipe.source().register(this.slt, SelectionKey.OP_READ);
			pipe.sink().configureBlocking(true); /* 写阻塞. */
			this.bb = ByteBuffer.allocateDirect(64 * 1024);
			Gsc.currgwk.set(this);
			return pipe;
		} catch (Exception e)
		{
			Log.error(Log.trace(e));
			return null;
		}
	}

	/** 设置libgsc上的客户端套接字选项. */
	public final void setSocketOpt(SocketChannel sc) throws IOException
	{
		sc.configureBlocking(false);
		sc.setOption(StandardSocketOptions.SO_LINGER, -1);
		sc.setOption(StandardSocketOptions.TCP_NODELAY, true);
		sc.setOption(StandardSocketOptions.SO_RCVBUF, Cfg.libgsc_peer_rcvbuf);
		sc.setOption(StandardSocketOptions.SO_SNDBUF, Cfg.libgsc_peer_sndbuf);
	}

	/** 关闭ActorNet. */
	public final void removeActorNet(ActorNet an)
	{
		if (an.type == ActorType.N2H)
			this.lazyCloseN2hs.remove(an.sc.hashCode());
		this.ans.remove(an.sc.hashCode());
		an.sc.keyFor(this.slt).cancel(); /* 从selector处注销. */
		Net.closeSocketChannel(an.sc); /* 关闭套接字. */
		an.sc = null;
		an.est = false;
	}

	/** ---------------------------------------------------------------- */
	/**                                                                  */
	/**  */
	/**                                                                  */
	/** ---------------------------------------------------------------- */
	/** 缓存了要延时关闭的N2H, 从最后收到消息的时间戳开始计时. */
	private HashMap<Integer /* SocketChannel.hashCode(). */, N2H> lazyCloseN2hs = new HashMap<>();
	/** 线程上所有的H2N, 用于H2N自身检查事务超时和心跳发送 */
	public ArrayList<H2N> h2ns = new ArrayList<>();
	/** 检查ans间隔时间(毫秒). */
	private long ansCheckInterval = 1000L;
	/** 上次检查ans的时间. */
	private long ansLastCheck = 0L;
	/** 向H2N发送通知的间隔. */
	private long h2nsCheckInterval = 1000L;
	/** 上次向H2N发送通知的时间. */
	private long h2nsLastNot = 0L;
	/** 检查lazyCloseN2hs超时的间隔时间. */
	private long lazyCloseN2hsCheckInterval = 1000L;
	/** 上次检查lazyCloseN2hs的时间. */
	private long lazyCloseN2hsLastCheck = 0L;

	/** 定时任务. */
	public final void check(long now)
	{
		this.checkN2h(now);
		this.checkH2n(now);
		this.checkLazyCloseN2hs(now);
	}

	/** 检查ans中的N2H连接. */
	private final void checkN2h(long now)
	{
		if (now - this.ansLastCheck < this.ansCheckInterval)
			return;
		this.ansLastCheck = now;
		Iterator<Entry<Integer, ActorNet>> it = this.ans.entrySet().iterator();
		while (it.hasNext())
		{
			ActorNet an = it.next().getValue();
			if (an.type == ActorType.H2N) /* 不关心对外的连接. */
				continue;
			N2H n2h = (N2H) an;
			if (n2h.lz) /* 不关心要延迟关闭的连接. */
				continue;
			/** -------------------------------- */
			/**                                  */
			/** 僵尸连接 */
			/**                                  */
			/** -------------------------------- */
			if (n2h.lts == 0) /** 新连接. */
			{
				if (now - n2h.gts >= Cfg.libgsc_n2h_zombie)
				{
					if (Log.isDebug())
						Log.debug("got a zombie n2h connection: %s, elap: %dmsec", n2h, now - n2h.gts);
					it.remove();
					this.lazyCloseN2hs.remove(n2h.sc.hashCode());
					n2h.sc.keyFor(this.slt).cancel();
					Net.closeSocketChannel(n2h.sc);
					n2h.evnDis();
				}
				continue;
			}
			/** -------------------------------- */
			/**                                  */
			/** 超出心跳时间 */
			/**                                  */
			/** -------------------------------- */
			if (now - n2h.lts >= Cfg.libgsc_peer_heartbeat * 2)
			{
				if (Log.isDebug())
					Log.debug("have a n2h connection lost heart-beat: %s", n2h);
				it.remove();
				this.lazyCloseN2hs.remove(n2h.sc.hashCode());
				n2h.sc.keyFor(this.slt).cancel();
				Net.closeSocketChannel(n2h.sc);
				n2h.evnDis();
			}
		}
	}

	/** H2N定时器振荡. */
	private final void checkH2n(long now)
	{
		if (now - this.h2nsCheckInterval < this.h2nsLastNot)
			return;
		this.h2nsLastNot = now;
		this.h2ns.forEach(o -> o.check(now));
	}

	/** 检查要延迟关闭的N2H连接. */
	private final void checkLazyCloseN2hs(long now)
	{
		if (now - this.lazyCloseN2hsLastCheck < this.lazyCloseN2hsCheckInterval)
			return;
		this.lazyCloseN2hsLastCheck = now;
		Iterator<Entry<Integer, N2H>> it = this.lazyCloseN2hs.entrySet().iterator();
		while (it.hasNext())
		{
			N2H n2h = it.next().getValue();
			if (now < n2h.lts)
				continue;
			if (Log.isDebug())
				Log.debug("N2H lazy-close, elap: %dmsec, actor: %s", now - n2h.lts, n2h);
			it.remove();
			this.ans.remove(n2h.sc.hashCode());
			Net.closeSocketChannel(n2h.sc);
			n2h.evnDis();
		}
	}
}
