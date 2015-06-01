package core;

import java.util.function.Consumer;

import actor.H2N;

import com.google.protobuf.Message;
import com.google.protobuf.ProtocolMessageEnum;

/**
 * 
 * H2N网络事务抽象.
 *
 * @Created on: 2015年1月8日 下午7:06:11
 * @Author: xuzewen
 * 
 */
public class Gh2ntrans
{
	/** 是否超时. */
	public boolean tm;
	/** 命令字. */
	public ProtocolMessageEnum cmd;
	/** 事务id. */
	public int tid;
	/** 事务关联的H2N连接. */
	public H2N h2n;
	/** 事务上BEGIN消息. */
	public Message begin;
	/** BEGIN消息上的扩展数据. */
	public byte[] extbegin;
	/** END. */
	public Gend end = new Gend();
	/** 事务产生时间. */
	public long gts;
	/** END回调. */
	public Consumer<Gend> endCb;
	/** 超时回调. */
	public Consumer<Void> tmCb;

	public Gh2ntrans(ProtocolMessageEnum cmd, int tid, H2N h2n, Message begin, Consumer<Gend> endCb, /* END回调. */Consumer<Void> tmCb, /* 超时回调. */byte[] extbegin)
	{
		this.tm = false;
		this.cmd = cmd;
		this.tid = tid;
		this.h2n = h2n;
		this.begin = begin;
		this.endCb = endCb;
		this.tmCb = tmCb;
		this.extbegin = extbegin;
		this.gts = System.currentTimeMillis();
	}
}
