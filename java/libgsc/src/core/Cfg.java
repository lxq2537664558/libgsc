package core;

import misc.Log;
import misc.Misc;

/**
 * 
 * libgsc配置加载.
 * 
 * @author xuzewen
 * @create on 2008-10-01
 * 
 */
public class Cfg
{
	/** ---------------------------------------------------------------- */
	/**                                                                  */
	/** 支持的配置项. */
	/**                                                                  */
	/** ---------------------------------------------------------------- */
	/** 服务地址. */
	public static final String LIBGSC_SERVER_HOST = "LIBGSC_SERVER_HOST";
	/** 服务端口. */
	public static final String LIBGSC_SERVER_PORT = "LIBGSC_SERVER_PORT";
	/** 消息总线线程数. */
	public static final String LIBGSC_WORKER = "LIBGSC_WORKER";
	/** 上行报文尺寸限制. */
	public static final String LIBGSC_PEER_MTU = "LIBGSC_PEER_MTU";
	/** 连接上的接收缓冲区尺寸. */
	public static final String LIBGSC_PEER_RCVBUF = "LIBGSC_PEER_RCVBUF";
	/** 连接上的发送缓冲区尺寸. */
	public static final String LIBGSC_PEER_SNDBUF = "LIBGSC_PEER_SNDBUF";
	/** 连接上的心跳间隔(毫秒), 同时用于H2N的心跳发送和N2H的心跳检测(LIBGSC_PEER_HEARTBEAT * 2). */
	public static final String LIBGSC_PEER_HEARTBEAT = "LIBGSC_PEER_HEARTBEAT";
	/** 当一个N2H连接上来后, 等待其第一个消息的时间(毫秒), 超过此时间连接将被强制断开. */
	public static final String LIBGSC_N2H_ZOMBIE = "LIBGSC_N2H_ZOMBIE";
	/** libgsc上所有连接都是linger(-1), 因此引入此选项, 用于N2H套接字延迟关闭(毫秒). */
	public static final String LIBGSC_N2H_LAZY_CLOSE = "LIBGSC_N2H_LAZY_CLOSE";
	/** H2N重连等待(毫秒). */
	public static final String LIBGSC_H2N_RECONN = "LIBGSC_H2N_RECONN";
	/** H2N网络事务超时时间(毫秒), 即发送请求后等待响应的时间. */
	public static final String LIBGSC_H2N_TRANS_TIMEOUT = "LIBGSC_H2N_TRANS_TIMEOUT";
	/** 定时器精度(毫秒). */
	public static final String LIBGSC_QUARTZ = "LIBGSC_QUARTZ";
	/** 日志服务器地址. */
	public static final String LIBGSC_LOG_HOST = "LIBGSC_LOG_HOST";
	/** 日志服务器端口. */
	public static final String LIBGSC_LOG_PORT = "LIBGSC_LOG_PORT";
	/** 日志级别. */
	public static final String LIBGSC_LOG_LEVEL = "LIBGSC_LOG_LEVEL";
	/** ---------------------------------------------------------------- */
	/**                                                                  */
	/** . */
	/**                                                                  */
	/** ---------------------------------------------------------------- */
	public static String libgsc_server_host = "0.0.0.0";
	public static int libgsc_server_port = 1225;
	public static int libgsc_worker = 4;
	public static int libgsc_peer_mtu = 8192;
	public static int libgsc_peer_rcvbuf = 8192;
	public static int libgsc_peer_sndbuf = 8192;
	public static long libgsc_peer_heartbeat = 15 * 1000;
	public static long libgsc_n2h_zombie = 3 * 1000;
	public static long libgsc_n2h_lazy_close = 3 * 1000;
	public static long libgsc_h2n_reconn = 3 * 1000;
	public static long libgsc_h2n_trans_timeout = 5 * 1000;
	public static long libgsc_quartz = 500;
	//
	public static String libgsc_log_host = "127.0.0.1";
	public static int libgsc_log_port = 1224;
	public static String libgsc_log_level = "INFO";

	private Cfg()
	{

	}

	public static final void init()
	{
		Cfg.libgsc_server_host = Misc.getSetEnvStr(Cfg.LIBGSC_SERVER_HOST, Cfg.libgsc_server_host);
		Cfg.libgsc_server_port = Misc.getSetEnvInt(Cfg.LIBGSC_SERVER_PORT, Cfg.libgsc_server_port);
		Cfg.libgsc_worker = Misc.getSetEnvInt(Cfg.LIBGSC_WORKER, Cfg.libgsc_worker);
		Cfg.libgsc_peer_mtu = Misc.getSetEnvInt(Cfg.LIBGSC_PEER_MTU, Cfg.libgsc_peer_mtu);
		Cfg.libgsc_peer_rcvbuf = Misc.getSetEnvInt(Cfg.LIBGSC_PEER_RCVBUF, Cfg.libgsc_peer_rcvbuf);
		Cfg.libgsc_peer_sndbuf = Misc.getSetEnvInt(Cfg.LIBGSC_PEER_SNDBUF, Cfg.libgsc_peer_sndbuf);
		Cfg.libgsc_peer_heartbeat = Misc.getSetEnvInt(Cfg.LIBGSC_PEER_HEARTBEAT, (int) Cfg.libgsc_peer_heartbeat);
		Cfg.libgsc_n2h_zombie = Misc.getSetEnvInt(Cfg.LIBGSC_N2H_ZOMBIE, (int) Cfg.libgsc_n2h_zombie);
		Cfg.libgsc_n2h_lazy_close = Misc.getSetEnvInt(Cfg.LIBGSC_N2H_LAZY_CLOSE, (int) Cfg.libgsc_n2h_lazy_close);
		Cfg.libgsc_h2n_reconn = Misc.getSetEnvInt(Cfg.LIBGSC_H2N_RECONN, (int) Cfg.libgsc_h2n_reconn);
		Cfg.libgsc_h2n_trans_timeout = Misc.getSetEnvInt(Cfg.LIBGSC_H2N_TRANS_TIMEOUT, (int) Cfg.libgsc_h2n_trans_timeout);
		Cfg.libgsc_quartz = Misc.getSetEnvInt(Cfg.LIBGSC_QUARTZ, (int) Cfg.libgsc_quartz);
		//
		Cfg.libgsc_log_host = Misc.getSetEnvStr(Cfg.LIBGSC_LOG_HOST, Cfg.libgsc_log_host);
		Cfg.libgsc_log_port = Misc.getSetEnvInt(Cfg.LIBGSC_LOG_PORT, Cfg.libgsc_log_port);
		Cfg.libgsc_log_level = Misc.getSetEnvStr(Cfg.LIBGSC_LOG_LEVEL, Cfg.libgsc_log_level);
		Log.info("\n----------------------------------------------------------------");
		Misc.getEnvs().forEach(o -> {
			if (o.getKey().toString().indexOf("LIBGSC_") == 0)
				Log.info("%s=%s", o.getKey(), o.getValue());
		});
		Log.info("\n----------------------------------------------------------------");
	}
}
