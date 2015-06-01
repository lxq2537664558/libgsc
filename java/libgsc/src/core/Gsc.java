package core;

import java.lang.reflect.Method;
import java.nio.channels.Selector;
import java.nio.channels.ServerSocketChannel;
import java.util.HashMap;

import misc.Log;
import misc.Misc;
import misc.Net;
import net.Gworker;

import com.google.protobuf.ProtocolMessageEnum;

/**
 * 
 * libgsc服务器.
 * 
 * @author xuzewen
 * @time 2015年1月12日 上午11:03:53
 *
 */
public class Gsc
{
	/** libgsc工作线程. */
	public static Gworker wks[];
	/** libgsc服务器句柄. */
	public static ServerSocketChannel srv = null;
	/** 事务结束回调. */
	public static EvnCb ec = null;
	/** N2H上行业务消息回调. */
	public static final HashMap<Integer, Cb> evns = new HashMap<>();
	/** 当前线程的Gworker对象. */
	public static final ThreadLocal<Gworker> currgwk = new ThreadLocal<>();

	private Gsc()
	{

	}

	/** 初始化libgsc消息总线. */
	public static final boolean init()
	{
		Cfg.init();
		try
		{
			Gsc.wks = new Gworker[Cfg.libgsc_worker];
			for (int i = 0; i < Gsc.wks.length; ++i)
				Gsc.wks[i] = new Gworker(i, Selector.open());
			Misc.sleep(100);
			return true;
		} catch (Exception e)
		{
			Log.error(Log.trace(e));
			return false;
		}
	}

	/** libgsc向外提供服务(开启监听端口). */
	public static final boolean publish()
	{
		try
		{
			Gsc.srv = ServerSocketChannel.open();
			Gsc.srv.socket().bind(Net.getAddr(Cfg.libgsc_server_host, Cfg.libgsc_server_port));
			Gsc.srv.socket().setReuseAddress(true);
			Gsc.srv.configureBlocking(false);
			Log.info("libgsc already listen on %s:%d.", Cfg.libgsc_server_host, Cfg.libgsc_server_port);
			for (int i = 0; i < Gsc.wks.length; ++i)
			{
				Gworker wk = Gsc.wks[i];
				if (i == 0) /* 只有一个Gworker线程处理accept. */
				{
					wk.future(v -> {
						if (!wk.regServerSocketChannel(Gsc.srv))
							System.exit(1);
					});
				}
			}
			return true;
		} catch (Exception e)
		{
			Log.error(Log.trace(e));
			return false;
		}
	}

	/** 定时器振荡. */
	public static final void hold()
	{
		while (true)
		{
			Misc.sleep(Cfg.libgsc_quartz);
			long now = System.currentTimeMillis();
			if (Gsc.ec != null) /* 应用还未初始化. */
				Gsc.ec.quartz(now);
			for (int i = 0; i < Gsc.wks.length; ++i)
			{
				Gworker wk = Gsc.wks[i];
				wk.future(v -> wk.check(now));
			}
		}
	}

	/** 注册N2H上的消息回调. */
	public static final boolean regN2HEvn(ProtocolMessageEnum cmd, Class<?> begin, Class<?> end, Class<?> uni, Class<?> cbClass, boolean gusr)
	{
		if (Gsc.evns.get(cmd.getNumber()) != null)
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
		Log.info("reg N2H-MSG, cmd: %s(%04X), cls: %s, begin: %s, end: %s, uni: %s", cmd, cmd.getNumber(), cbClass == null ? "NULL" : cbClass.getSimpleName(),//
				begin == null ? "NULL" : begin.getSimpleName(), //
				end == null ? "NULL" : end.getSimpleName(), //
				uni == null ? "NULL" : uni.getSimpleName());
		Gsc.evns.put(cmd.getNumber(), new Cb(cmd, m, begin, end, uni, gusr));
		return false;
	}

	/** 注册libgsc业务日志出口. */
	public static final void regEvnCb(EvnCb ec)
	{
		Gsc.ec = ec;
	}

	/** 返回当前工作线程. */
	public static final Gworker getWorker()
	{
		return Gsc.currgwk.get();
	}

	/** 为Actor选择一个工作线程(round-robin). */
	public static final int rrWk()
	{
		return (Gworker.rb.incrementAndGet() & 0x7FFFFFFF) % Cfg.libgsc_worker;
	}

	/** 为Actor选择一个工作线程(散列). */
	public static final int hashWk(long id)
	{
		return (int) ((id & 0x7FFFFFFFFFFFFFFFL) % (long) Cfg.libgsc_worker);
	}

	/** 返回当前工作线程的索引, 没有时返回-1. */
	public static final int getWorkerIndex()
	{
		Gworker wk = Gsc.currgwk.get();
		return wk == null ? -1 : wk.wk;
	}
}
