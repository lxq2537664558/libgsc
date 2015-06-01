package core;

import stmp.Stmp.Ret;
import actor.N2H;

import com.google.protobuf.GeneratedMessage.Builder;
import com.google.protobuf.Message;
import com.google.protobuf.ProtocolMessageEnum;

/**
 * 
 * N2H网络事务抽象.
 *
 * @Created on: 2015年1月8日 下午7:06:11
 * @Author: xuzewen
 * 
 */
public class Gn2htrans
{
	/** 连接延迟关闭(毫秒). */
	public int lzmsec;
	/** 事务发起者. */
	public N2H n2h;
	/** 消息命令字. */
	public ProtocolMessageEnum cmd;
	/** 事务id. */
	public int tid;
	/** 事务上BEGIN消息. */
	public Message begin;
	/** BEGIN消息上的扩展数据. */
	public byte[] extbegin;
	/** END. */
	public Gend end = new Gend();

	public Gn2htrans(N2H n2h, ProtocolMessageEnum cmd, int tid, Message begin, byte[] ext)
	{
		this.n2h = n2h;
		this.cmd = cmd;
		this.tid = tid;
		this.begin = begin;
		this.extbegin = ext;
		this.lzmsec = 0;
	}

	private final void end(short ret, Builder<?> end, byte[] extend)
	{
		this.end.ret = ret;
		this.end.end = end == null ? null : end.build();
		this.end.extend = extend;
		if (this.end.ret == (short) Ret.RET_FAILURE.ordinal()) /* 失败, 关闭连接. */
		{
			this.n2h.future(v -> {
				this.n2h.close();
				this.finish();
			});
			return;
		}
		this.n2h.future(v -> {
			if (this.lzmsec > 0)
			{
				this.n2h.lts += this.lzmsec;
				this.n2h.lazyClose(); /* 延迟关闭. */
			}
			if (this.n2h.gusr != null) /* 有应用层关联. */
			{
				this.n2h.gusr.future(vx -> {
					this.n2h.gusr.sendEnd0(this);
				});
			} else
			{
				this.n2h.sendEnd(this);
			}
		});
	}

	public final void end(ProtocolMessageEnum ret, Builder<?> end, byte[] extend)
	{
		this.end((short) ret.getNumber(), end, extend);
	}

	public final void end(short ret)
	{
		this.end(ret, null, null);
	}

	public final void end(ProtocolMessageEnum ret)
	{
		this.end(ret, null, null);
	}

	public final void end(Builder<?> end)
	{
		this.end(Ret.RET_SUCCESS, end, null);
	}

	public final void end(ProtocolMessageEnum ret, Builder<?> end)
	{
		this.end(ret, end, null);
	}

	public final void success()
	{
		this.end(Ret.RET_SUCCESS, null, null);
	}

	public final void failure()
	{
		this.end(Ret.RET_FAILURE, null, null);
	}

	public final void successLazyClose(int sec)
	{
		this.lzmsec = (int) (sec == 0 ? Cfg.libgsc_n2h_lazy_close/* 默认值. */: sec * 1000);
		this.end(Ret.RET_SUCCESS, null, null);
	}

	public final void successLazyClose(Builder<?> end, int sec)
	{
		this.lzmsec = (int) (sec == 0 ? Cfg.libgsc_n2h_lazy_close/* 默认值. */: sec * 1000);
		this.end(Ret.RET_SUCCESS, end, null);
	}

	public final void successLazyClose(ProtocolMessageEnum ret, int sec)
	{
		this.lzmsec = (int) (sec == 0 ? Cfg.libgsc_n2h_lazy_close/* 默认值. */: sec * 1000);
		this.end(ret, null, null);
	}

	public final void finish()
	{
		if (Gsc.ec != null)
			Gsc.ec.gn2hTrans(this);
	}
}
