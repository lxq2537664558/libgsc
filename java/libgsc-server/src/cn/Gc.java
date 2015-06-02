package cn;

import gcpb.NetGcWithGas.GcAuthWithGasReq;
import gcpb.NetGcWithGas.GcAuthWithGasRsp;

import java.net.InetSocketAddress;

import misc.Log;
import misc.Misc;
import misc.Net;
import actor.H2N;
import cn.Gcb.CmdGas;

import com.google.protobuf.ByteString;
import com.google.protobuf.Message;

import core.Cfg;

/**
 * 
 * libgsc主动向外发起的连接.
 *
 * @Created on: 2015年6月2日 上午11:01:21
 * @Author: xuzewen
 *
 */
public class Gc extends H2N
{
	private static Gc inst = null;

	private byte[] comkey = null;

	private Gc(InetSocketAddress addr)
	{
		super(addr);
	}

	public static void init()
	{
		Gc.inst = new Gc(Net.getAddr(Cfg.libgsc_server_host, Cfg.libgsc_server_port));
	}

	public static Gc instance()
	{
		return Gc.inst;
	}

	/** 连接建立后的回调. */
	public void estb()
	{
		GcAuthWithGasReq.Builder b = GcAuthWithGasReq.newBuilder();
		b.setSalt(ByteString.copyFromUtf8("hello."));
		b.setPlat("PC");
		GcAuthWithGasReq req = b.build();
		this.future(CmdGas.GAS_REQ_GC_AUTH, req, rsp -> { /* 发送鉴权请求. */
			Log.info("got rsp: %s", Misc.pb2str(rsp.end));
		}, timeout -> {
			Log.error("timeout, req: %s", Misc.pb2str(req));
		}, null);
	}

	public void disc()
	{
		Log.warn("connection lost with remote-server: %s", Net.getAddr(this.addr));
	}

	/** 注册消息回调. */
	public boolean regEvns()
	{
		return this.regEvn(CmdGas.GAS_REQ_GC_AUTH, GcAuthWithGasReq.class, GcAuthWithGasRsp.class, null, null, false);
	}

	public HbStub heartbeatreq()
	{
		return null;
	}

	public void heartbeatrsp(short ret, Message rsp, byte[] ext, boolean tm)
	{

	}

	/** 消息出栈时的加密函数. */
	public byte[] encryp(byte[] by)
	{
		return Misc.rc4enc(this.comkey, by);
	}

	/** 消息入栈时的解密函数. */
	public byte[] decryp(byte[] by)
	{
		return this.encryp(by);
	}
}
