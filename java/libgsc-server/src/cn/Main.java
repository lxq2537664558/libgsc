package cn;

import misc.Log;
import misc.Misc;
import core.Cfg;
import core.Gsc;

public class Main
{
	public static void main(String[] args)
	{
		Log.init("none", null, null, 0, Log.OUTPUT_STDOUT);
		Log.setRecord();
		// server();
		client();
	}

	/** libgsc作client. */
	public static void client()
	{
		Misc.setEnv(Cfg.LIBGSC_PEER_HEARTBEAT, 2000);
		Gsc.init();
		Gc.init();
		Gsc.hold();
	}

	/** libgsc作server. */
	public static void server()
	{
		Misc.setEnv(Cfg.LIBGSC_N2H_ZOMBIE, 1000000L);
		Gsc.init(); /* libgsc初始化, 包括配置读取, 消息总线的准备. */
		Gsc.ec = Gcb.instance(); /* 事务输出设置. */
		Gcb.instance().reg(); /* 注册网络消息回调. */
		Gsc.publish(); /* libgsc开启监听端口. 也就是从这里开始向外提供服务. 因此在调用此函数前, 网络消息回调等业务运行环境都应该已经就緒. */
		Gsc.hold(); /* 这是一个main-loop, 提供一定精度的定时器. */
	}
}
