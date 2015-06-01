package actor;

import java.nio.ByteBuffer;
import java.nio.channels.SocketChannel;

import misc.Log;
import misc.Misc;
import misc.Net;
import net.Pro;
import stmp.Stmp;
import stmp.StmpDec;
import stmp.StmpNode;

import com.google.protobuf.Message;
import com.google.protobuf.ProtocolMessageEnum;

import core.Cb;
import core.Gn2htrans;
import core.Gsc;

/**
 * 
 * 描述了一个网络向libgsc发起的TCP连接.
 * 
 * 只允许上行GPDU_REQ和下行GPDU_RSP/GPDU_NOT.
 * 
 * @created on: 2014年11月20日 下午8:45:51
 * @author: xuzewen
 */
public class N2H extends ActorNet
{
	/** 用户层数据. */
	public Gusr gusr = null;
	/** 是否要延迟关闭. */
	public boolean lz = false;

	public N2H(SocketChannel sc, ByteBuffer bb)
	{
		super(ActorType.N2H, sc, bb);
		this.est = true;
		this.gts = System.currentTimeMillis();
		this.wk = Gsc.getWorkerIndex(); /* 取当前工作线程索引. */
		this.bb = bb;
	}

	/** 网络消息送达事件. */
	public boolean evnMsg(StmpNode root)
	{
		/**
		 * N2H支持下行(发出)END/UNI, 上行(收到)BEGIN/UNI.
		 */
		switch (root.self.t)
		{
		case Stmp.STMP_TAG_TRANS_BEGIN:
			return this.evnBegin(root);
		case Stmp.STMP_TAG_TRANS_UNI:
			return this.evnUni(root);
		default:
			if (Log.isDebug())
				Log.debug("unsupported STMP transaction: %02X", root.self.t);
			return false;
		}
	}

	/** BEGIN. */
	private final boolean evnBegin(StmpNode root)
	{
		Long uid = StmpDec.getLong(root, Stmp.STMP_TAG_UID);
		if (uid == null) /* 无UID, 被认为是长连接. */
			return this.gusr == null ? this.evnBeginNoGusr(root) : this.evnBeginOnGusr(root);
		Long sid = StmpDec.getLong(root, Stmp.STMP_TAG_SID); /* 否则就是短连接, 有UID时, 一定要有SID. */
		if (sid == null)
		{
			if (Log.isDebug())
				Log.debug("missing required field: STMP_TAG_SID.");
			return false;
		}
		return this.evnBeginTmpConn(root, uid, sid);
	}

	/** BEGIN-长连接(鉴权通过前). */
	private final boolean evnBeginNoGusr(StmpNode root)
	{
		try
		{
			Gn2htrans gt = Pro.unpkgBegin(root);
			if (gt == null)
				return false;
			gt.n2h = this;
			Cb cb = Gsc.evns.get(gt.cmd.getNumber());
			if (Log.isTrace())
			{
				Log.trace("\n  <-- BEGIN(%s): {%s}, END(%s), CMD: %s, TID: %08X\n", //
						cb.beginName, gt.begin == null ? "NULL" : Misc.pb2str(gt.begin),//
						cb.endName, cb.cmd, gt.tid);
			}
			cb.cb.invoke(null, this, gt, gt.begin);
			return true;
		} catch (Exception e)
		{
			if (Log.isDebug())
				Log.debug(Log.trace(e));
		}
		return true;
	}

	/** BEGIN-长连接(鉴权通过后). */
	private final boolean evnBeginOnGusr(StmpNode root)
	{
		Integer tid = StmpDec.getInt(root, Stmp.STMP_TAG_STID);
		if (tid == null)
		{
			if (Log.isDebug())
				Log.debug("missing required field: STMP_TAG_STID");
			return false;
		}
		byte[] sec = StmpDec.getBin(root, Stmp.STMP_TAG_DAT);
		if (sec == null)
		{
			if (Log.isDebug())
				Log.debug("missing required field: STMP_TAG_DAT");
			return false;
		}
		Gn2htrans gt = new Gn2htrans(this, null, tid, null, null); /* 未初始化. */
		this.gusr.future(v -> this.gusr.procBegin0(sec, gt)); /* 前转到Gusr中处理. */
		return true;
	}

	/** BEGIN-短连接. */
	private final boolean evnBeginTmpConn(StmpNode root, long uid, long sid)
	{
		Log.warn("unsupported.");
		return false;
	}

	/** UNI. */
	private final boolean evnUni(StmpNode root)
	{
		return this.gusr == null ? this.evnUniNoGusr(root) : this.evnUniOnGusr(root);
	}

	/** UNI(鉴权通过前). */
	private final boolean evnUniNoGusr(StmpNode root)
	{
		Log.warn("unsupported.");
		return false;
	}

	/** UNI(鉴权通过后). */
	private final boolean evnUniOnGusr(StmpNode root)
	{
		Log.warn("unsupported.");
		return false;
	}

	/** ---------------------------------------------------------------- */
	/**                                                                  */
	/**  */
	/**                                                                  */
	/** ---------------------------------------------------------------- */
	/** 仅当网络主动断开时调用. */
	public final void evnDis()
	{
		this.est = false;
		this.sc = null;
		if (Log.isDebug())
			Log.debug("have a connection lost, peer: %s", Net.getRemoteAddr(this.sc));
		if (this.gusr != null)
		{
			Gusr g = this.gusr;
			this.gusr.future(v -> {
				this.gusr = null;
				g.disc();
				g.n2h = null;
			});
		}
	}

	/** 当N2H与Gusr无关联时的STMP-END消息. */
	public final void sendEnd(Gn2htrans gt)
	{
		if (Log.isTrace())
		{
			Cb cb = Gsc.evns.get(gt.cmd.getNumber());
			if (cb != null)
			{
				Log.trace("\n  --> END(%s): {%s}, BEGIN(%s): {%s}, RET: %04X, CMD: %s, TID: %08X, PEER: %s\n", //
						cb.endName, gt.end.end == null ? "NULL" : Misc.pb2str(gt.end.end),//
						cb.beginName, gt.begin == null ? "NULL" : Misc.pb2str(gt.begin),//
						gt.end.ret, gt.cmd, gt.tid, Net.getRemoteAddr(this.sc));
			} else
				Log.fault("it`s a bug, cmd: %s", gt.cmd);
		}
		this.send(Pro.pkgEnd(gt.end.ret, gt.tid, gt.end.end, gt.end.extend));
		gt.finish();
	}

	/** 当N2H与Gusr无关联时的STMP-UNI消息. */
	public final void sendUni(ProtocolMessageEnum cmd, Message uni, byte[] ext)
	{
		Log.warn("unsupported.");
		if (Gsc.ec != null)
			Gsc.ec.gn2hSendUni(this, cmd, uni, ext);
	}

	/** 套接字延迟关闭. */
	public final void lazyClose()
	{
		if (!this.est)
			return;
		this.lz = true;
		this.getGworker().addLazyCloseN2h(this);
	}

	public String toString()
	{
		return Misc.printf2Str("gusr: %s, peer: %s", this.gusr, Net.getRemoteAddr(this.sc));
	}
}
