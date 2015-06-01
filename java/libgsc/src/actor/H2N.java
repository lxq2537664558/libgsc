package actor;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.SelectionKey;
import java.nio.channels.SocketChannel;
import java.util.HashMap;
import java.util.Iterator;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.function.Consumer;

import misc.Log;
import misc.Misc;
import misc.Net;
import net.Pro;
import stmp.Stmp;
import stmp.Stmp.Ret;
import stmp.StmpDec;
import stmp.StmpEnc;
import stmp.StmpNode;
import stmp.StmpPdu;

import com.google.protobuf.InvalidProtocolBufferException;
import com.google.protobuf.Message;
import com.google.protobuf.ProtocolMessageEnum;

import core.Cb;
import core.Cfg;
import core.Gend;
import core.Gh2ntrans;
import core.Gsc;

/**
 * 
 * 描述了一个libgsc向网络发起的TCP连接.
 * 
 * 只允许上行GPDU_RSP/GPDU_NOT和下行GPDU_REQ.
 * 
 * @created on: 2014年11月29日 下午8:07:13
 * @author: xuzewen
 */
public abstract class H2N extends ActorNet
{
	/** H2N下行消息回调. */
	public HashMap<Integer, Cb> evns = new HashMap<>();
	/** 事务ID发生器. */
	private AtomicInteger seq = new AtomicInteger(Misc.randInt());
	/** H2N的网络状态. */
	public AtomicBoolean status = new AtomicBoolean(H2N.DISC);
	/** 缓存发出了请求, 但未收到响应的事务. */
	private HashMap<Integer, Gh2ntrans> gts = new HashMap<>();
	/** 是否第一次发起连接. */
	private boolean needWait = true;
	/** 要连接的远端地址. */
	public InetSocketAddress addr = null;
	/** 是否鉴权通过. */
	public boolean ready = false;
	private static final boolean ESTB = true;
	private static final boolean DISC = false;

	public H2N(InetSocketAddress addr)
	{
		super(ActorType.H2N, null, ByteBuffer.allocate(Cfg.libgsc_peer_mtu));
		this.wk = Gsc.hashWk(this.hashCode()); /* 随机选取一个工作线程. */
		this.addr = addr;
		this.getGworker().h2ns.add(this); /* 添加到H2N列表. */
		if (this.regEvns())
			this.connect();
		else
		{
			Log.error("reg evn failed.");
			System.exit(1);
		}
	}

	/** ---------------------------------------------------------------- */
	/**                                                                  */
	/**  */
	/**                                                                  */
	/** ---------------------------------------------------------------- */
	/** 心跳消息请求. */
	public abstract HbStub heartbeatreq();

	/** 心跳响应. */
	public abstract void heartbeatrsp(short ret, Message rsp, byte[] ext, boolean tm /* 是否超时. */);

	/** 注册网络消息回调. */
	public abstract boolean regEvns();

	/** 连接已建立. */
	public abstract void estb();

	/** 连接已失去. */
	public abstract void disc();

	/** 消息出栈时的加密函数. */
	public abstract byte[] encryp(byte[] by);

	/** 消息入栈时的解密函数. */
	public abstract byte[] decryp(byte[] by);

	/** ---------------------------------------------------------------- */
	/**                                                                  */
	/**  */
	/**                                                                  */
	/** ---------------------------------------------------------------- */
	/** 连接断开事件. */
	public final void evnDis()
	{
		this.ready = false;
		this.est = false;
		this.sc = null;
		this.status.set(H2N.DISC);
		this.disc();
		this.connect();
	}

	/** 网络消息送达事件(STMP). */
	public final boolean evnMsg(StmpNode root)
	{
		/**
		 * H2N只支持下行(发现)BEGIN/UNI, 上行(收到)END/UNI.
		 */
		switch (root.self.t)
		{
		case Stmp.STMP_TAG_TRANS_END:
			return this.evnEnd(root);
		case Stmp.STMP_TAG_TRANS_UNI:
			return this.evnUni(root);
		default:
			if (Log.isDebug())
				Log.debug("unsupported STMP transaction: %02X", root.self.t);
			return false;
		}
	}

	/** STMP-END. */
	private final boolean evnEnd(StmpNode root)
	{
		try
		{
			return this.ready ? this.evnEndOnGusr(root) : this.evnEndNoGusr(root);
		} catch (Exception e)
		{
			if (Log.isDebug())
				Log.debug(Log.trace(e));
			return false;
		}
	}

	/** END(鉴权通过前). */
	private final boolean evnEndNoGusr(StmpNode root) throws InvalidProtocolBufferException, IllegalAccessException, IllegalArgumentException, InvocationTargetException
	{
		Integer tid = StmpDec.getInt(root, Stmp.STMP_TAG_DTID);
		if (tid == null)
		{
			Log.warn("missing required field: STMP_TAG_DTID");
			return false;
		}
		Short ret = StmpDec.getShort(root, Stmp.STMP_TAG_RET);
		if (ret == null)
		{
			Log.warn("missing required field: STMP_TAG_RET");
			return false;
		}
		Gh2ntrans gt = this.gts.remove(tid);
		if (gt == null) /* 找不到事务, 可能已经超时. */
		{
			Log.warn("can not found Gh2ntrans for tid: %08X, may be it was timeout.", tid);
			return true;
		}
		gt.end.ret = ret;
		Cb cb = this.evns.get(gt.cmd.getNumber());
		if (cb == null) /* 找不到回调. */
		{
			Log.fault("it`s a bug, cmd: %s", gt.cmd);
			return false;
		}
		if (cb.endBuilder == null)
		{
			Log.warn("it`s an unexpected cmd: %s", cb.cmd);
			return false;
		}
		byte[] pb = StmpDec.getBin(root, Stmp.STMP_TAG_DAT);
		if (pb != null)
			gt.end.end = cb.newEndBulider().mergeFrom(pb).build();
		byte[] att = StmpDec.getBin(root, Stmp.STMP_TAG_ATT);
		if (att != null)
			gt.end.extend = att;
		if (Log.isTrace())
		{
			Log.trace("\n  <-- END(%s): {%s}, BEGIN(%s): {%s}, RET: %04X, CMD: %s, TID: %08X, PEER: %s\n", //
					cb.endName, gt.end.end == null ? "NULL" : Misc.pb2str(gt.end.end),//
					cb.beginName, gt.begin == null ? "NULL" : Misc.pb2str(gt.begin),//
					gt.end.ret, gt.cmd, gt.tid, Net.getAddr(this.addr));
		}
		gt.endCb.accept(gt.end);
		if (Gsc.ec != null)
			Gsc.ec.gh2nTrans(gt); /* 事务输出. */
		return true;
	}

	/** END(鉴权通过后). */
	private final boolean evnEndOnGusr(StmpNode root) throws InvalidProtocolBufferException, IllegalAccessException, IllegalArgumentException, InvocationTargetException
	{
		Integer tid = StmpDec.getInt(root, Stmp.STMP_TAG_DTID);
		if (tid == null)
		{
			Log.warn("missing required field: STMP_TAG_DTID");
			return false;
		}
		Gh2ntrans gt = this.gts.remove(tid);
		if (gt == null) /* 找不到事务, 可能已经超时. */
		{
			Log.warn("can not found Gh2ntrans for tid: %08X, may be it was timeout.", tid);
			return true;
		}
		byte[] sec = StmpDec.getBin(root, Stmp.STMP_TAG_DAT);
		if (sec == null)
		{
			Log.warn("missing required field: STMP_TAG_DAT.");
			return false;
		}
		byte[] dat = this.decryp(sec);
		StmpNode node = StmpDec.unpack(dat);
		if (node == null)
		{
			Log.warn("STMP protocol error.");
			return false;
		}
		Short ret = StmpDec.getShort(node, Stmp.STMP_TAG_RET);
		if (ret == null)
		{
			Log.warn("missing required field: STMP_TAG_RET");
			return false;
		}
		gt.end.ret = ret;
		Cb cb = this.evns.get(gt.cmd.getNumber());
		if (cb == null) /* 找不到回调. */
		{
			Log.fault("it`s a bug, cmd: %s", gt.cmd);
			return false;
		}
		if (cb.endBuilder == null)
		{
			Log.warn("it`s an unexpected cmd: %s", cb.cmd);
			return false;
		}
		byte[] pb = StmpDec.getBin(node, Stmp.STMP_TAG_DAT);
		if (pb != null)
			gt.end.end = cb.newEndBulider().mergeFrom(pb).build();
		byte[] att = StmpDec.getBin(node, Stmp.STMP_TAG_ATT);
		if (att != null)
			gt.end.extend = att;
		if (Log.isTrace())
		{
			Log.trace("\n  <-- END(%s): {%s}, BEGIN(%s): {%s}, RET: %04X, CMD: %s, TID: %08X, PEER: %s\n", //
					cb.endName, gt.end.end == null ? "NULL" : Misc.pb2str(gt.end.end),//
					cb.beginName, gt.begin == null ? "NULL" : Misc.pb2str(gt.begin),//
					gt.end.ret, gt.cmd, gt.tid, Net.getAddr(this.addr));
		}
		gt.endCb.accept(gt.end);
		if (Gsc.ec != null)
			Gsc.ec.gh2nTrans(gt); /* 事务输出. */
		return true;
	}

	/** STMP-UNI. */
	private final boolean evnUni(StmpNode root)
	{
		try
		{
			return this.ready ? this.evnUniOnGusr(root) : this.evnUniNoGusr(root);
		} catch (Exception e)
		{
			if (Log.isDebug())
				Log.debug(Log.trace(e));
			return false;
		}
	}

	/** UNI(鉴权通过前). */
	private final boolean evnUniNoGusr(StmpNode root) throws InvalidProtocolBufferException, IllegalAccessException, IllegalArgumentException, InvocationTargetException
	{
		Short cmd = StmpDec.getShort(root, Stmp.STMP_TAG_CMD);
		if (cmd == null)
		{
			Log.warn("missing required field: STMP_TAG_CMD");
			return false;
		}
		Cb cb = this.evns.get(cmd.intValue());
		if (cb == null) /* 找不到回调. */
		{
			Log.warn("can not found call back for this cmd: %04X", cmd); /* 不支持的命令字. */
			return false;
		}
		if (cb.uniBuilder == null || cb.cb == null)
		{
			Log.warn("it`s an unexpected cmd: %s", cb.cmd);
			return false;
		}
		Message uni = null;
		byte pb[] = StmpDec.getBin(root, Stmp.STMP_TAG_DAT);
		if (pb != null)
			uni = cb.newUniBulider().mergeFrom(pb).build();
		byte[] ext = StmpDec.getBin(root, Stmp.STMP_TAG_ATT);
		if (Log.isTrace())
			Log.trace("\n  <-- UNI(%s): {%s}, CMD: %s, PEER: %s\n", cb.uniName, uni == null ? "NULL" : Misc.pb2str(uni), cb.cmd, Net.getAddr(this.addr));
		cb.cb.invoke(null, this, uni, ext);
		if (Gsc.ec != null)
			Gsc.ec.gh2nRecvUni(this, cb.cmd, uni, ext);
		return true;
	}

	/** UNI(鉴权通过后). */
	private final boolean evnUniOnGusr(StmpNode root) throws InvalidProtocolBufferException, IllegalAccessException, IllegalArgumentException, InvocationTargetException
	{
		byte[] sec = StmpDec.getBin(root, Stmp.STMP_TAG_DAT);
		if (sec == null)
		{
			Log.warn("missing required field: STMP_TAG_DAT.");
			return false;
		}
		byte[] dat = this.decryp(sec);
		StmpNode node = StmpDec.unpack(dat);
		if (node == null)
		{
			Log.warn("STMP protocol error.");
			return false;
		}
		return this.evnUniNoGusr(node);
	}

	/** ---------------------------------------------------------------- */
	/**                                                                  */
	/**  */
	/**                                                                  */
	/** ---------------------------------------------------------------- */
	/** 开启一个H2N事务. */
	public final void future(ProtocolMessageEnum cmd, Message req, Consumer<Gend> rspCb, Consumer<Void> tmCb, byte[] extreq)
	{
		super.future(v -> this.sendBegin(new Gh2ntrans(cmd, this.nextTid(), this, req, rspCb, tmCb, extreq)));
	}

	/** 向远端地址发送一个BEGIN消息. */
	public final void sendBegin(Gh2ntrans gt)
	{
		if (Log.isTrace())
		{
			Cb cb = this.evns.get(gt.cmd.getNumber());
			if (cb == null)
			{
				Log.fault("it`s a bug, cmd: %s", gt.cmd);
				return;
			}
			Log.trace("\n  --> BEGIN(%s): {%s}, END(%s), CMD: %s, TID: %08X, PEER: %s\n", //
					cb.beginName, gt.begin == null ? "NULL" : Misc.pb2str(gt.begin),//
					cb.endName, gt.cmd, gt.tid, Net.getAddr(this.addr));
		}
		this.gts.put(gt.tid, gt); /* 总是缓存. */
		if (!this.status.get()) /* 连接还未建立. */
			return;
		if (!this.ready) /* 还未鉴权通过. */
			this.send(Pro.pkgBegin(gt.cmd, gt.tid, gt.begin == null ? null : gt.begin.toByteArray(), gt.extbegin));
		else
		{
			byte[] dat = this.encryp(Pro.pkgStmpBeginSecDat(gt.cmd, gt.begin == null ? null : gt.begin.toByteArray(), gt.extbegin)); /* 加密. */
			StmpPdu sp = new StmpPdu(dat.length);
			StmpEnc.addBin(sp, Stmp.STMP_TAG_DAT, dat);
			StmpEnc.addInt(sp, Stmp.STMP_TAG_STID, gt.tid);
			StmpEnc.addTag(sp, Stmp.STMP_TAG_TRANS_BEGIN);
			this.send(sp.bytes());
		}
	}

	/** 尝试向remote发送一个STMP-UNI. */
	public final void sendUni(ProtocolMessageEnum cmd, Message uni, byte[] ext)
	{
		/** 未实现. */
		if (Gsc.ec != null)
			Gsc.ec.gh2nSendUni(this, cmd, uni, ext);
	}

	/** ---------------------------------------------------------------- */
	/**                                                                  */
	/**  */
	/**                                                                  */
	/** ---------------------------------------------------------------- */
	/** 尝试连接到远端服务器. */
	private final void connect()
	{
		H2N h2n = this;
		new Thread(new Runnable()
		{
			public void run()
			{
				if (h2n.needWait) /* 仅第一次无需等待. */
					h2n.needWait = false;
				else
					Misc.sleep(Cfg.libgsc_h2n_reconn);
				try
				{
					SocketChannel sc = SocketChannel.open(h2n.addr);
					h2n.future(x -> {
						if (h2n.regSocketChannel(sc))
						{
							Log.info("connect to remote server successfully, addr: %s", Net.getAddr(h2n.addr));
							h2n.sc = sc;
							h2n.est = true;
							if (!h2n.status.compareAndSet(H2N.DISC, H2N.ESTB))
								Log.fault("it`s a bug: %s", h2n.status.get());
							h2n.estb();
						} else
						{
							Log.info("can not connect to remote-addr: %s", Net.getAddr(h2n.addr));
							Net.closeSocketChannel(sc);
							h2n.connect();
						}
					});
				} catch (Exception e)
				{
					Log.info("can not connect to remote-addr(%s): %s", h2n.name, Net.getAddr(h2n.addr));
					if (Log.isTrace())
						Log.trace(Log.trace(e));
					h2n.connect();
				}
			}
		}).start();
	}

	/** 将SocketChannel注册的当前线程. */
	private final boolean regSocketChannel(SocketChannel sc)
	{
		try
		{
			this.getGworker().setSocketOpt(sc);
			sc.register(this.getGworker().slt, SelectionKey.OP_READ);
			this.getGworker().ans.put(sc.hashCode(), this);
			return true;
		} catch (Exception e)
		{
			Log.error(Log.trace(e));
			return false;
		}
	}

	/** 注册网络响应消息(鉴权通过前). */
	protected final boolean regEvn(ProtocolMessageEnum cmd, Class<?> begin, Class<?> end, Class<?> uni, Class<?> cbClass, boolean gusr)
	{
		if (this.evns.get(cmd.getNumber()) != null)
		{
			Log.error("duplicate cmd: %s", cmd);
			return false;
		}
		Method m = null;
		if (cbClass != null)
		{
			m = Misc.findMethodByName(cbClass, cmd.toString().toLowerCase());
			if (m == null)
			{
				Log.error("can not found call back method for cmd: %s, cls: %s", cmd, cbClass.getSimpleName());
				return false;
			}
		}
		Log.info("reg H2N-MSG, cmd: %s(%04X), cls: %s, begin: %s, end: %s, uni: %s", cmd, cmd.getNumber(), cbClass == null ? "NULL" : cbClass.getSimpleName(),//
				begin == null ? "NULL" : begin.getSimpleName(), //
				end == null ? "NULL" : end.getSimpleName(), //
				uni == null ? "NULL" : uni.getSimpleName());
		this.evns.put(cmd.getNumber(), new Cb(cmd, m, begin, end, uni, gusr));
		return true;
	}

	/** 获得一个新的事务ID. */
	private final int nextTid()
	{
		return this.seq.getAndIncrement();
	}

	/** ---------------------------------------------------------------- */
	/**                                                                  */
	/**  */
	/**                                                                  */
	/** ---------------------------------------------------------------- */
	/** 做两件事, 1, 检查超时的事务. 2, 决定是否要发心跳. */
	public final void check(long now)
	{
		this.checkGts(now);
		this.checkHeartBeat(now);
	}

	/** 检查超时的事务. */
	private final void checkGts(long now)
	{
		Iterator<Gh2ntrans> it = this.gts.values().iterator();
		while (it.hasNext())
		{
			Gh2ntrans gt = it.next();
			if (now < gt.gts)
				continue;
			it.remove();
			gt.tm = true;
			gt.tmCb.accept(null);
			if (Gsc.ec != null)
				Gsc.ec.gh2nTrans(gt);
		}
	}

	/** 检查是否要发送心跳. */
	private final void checkHeartBeat(long now)
	{
		if (!this.est) /* 连接还未建立. */
			return;
		if (!this.ready)
			return;
		if (now - this.mts > Cfg.libgsc_peer_heartbeat) /* 如果链路上超过LIBGSC_PEER_HEARTBEAT时间无任何消息, 才发送心跳. */
		{
			HbStub hb = this.heartbeatreq();
			this.future(hb.cmd, hb.msg, rsp -> { /* 收到心跳响应. */
				this.mts = System.currentTimeMillis();
				this.heartbeatrsp(rsp.ret, rsp.end, rsp.extend, false);
			}, tm -> { /* 超时未收到心跳. */
				this.heartbeatrsp((short) Ret.RET_FAILURE.ordinal(), null, null, false);
				this.closeSlient();
				this.evnDis();
			}, null);
		}
	}

	public String toString()
	{
		return Misc.printf2Str("name: %s, peer: %s", this.name, Net.getRemoteAddr(this.sc));
	}

	/** 心跳. */
	public class HbStub
	{
		/** 心跳指令. */
		public ProtocolMessageEnum cmd;
		/** 消息. */
		public Message msg;

		public HbStub(ProtocolMessageEnum cmd, Message msg)
		{
			this.cmd = cmd;
			this.msg = msg;
		}
	}
}
