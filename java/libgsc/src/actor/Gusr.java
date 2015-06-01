package actor;

import misc.Log;
import misc.Misc;
import misc.Net;
import net.Pro;
import stmp.Stmp;
import stmp.StmpDec;
import stmp.StmpEnc;
import stmp.StmpNode;
import stmp.StmpPdu;

import com.google.protobuf.GeneratedMessage.Builder;
import com.google.protobuf.Message;
import com.google.protobuf.ProtocolMessageEnum;

import core.Cb;
import core.Cfg;
import core.Gn2htrans;
import core.Gsc;

/**
 *
 * libgsc N2H连接上的用户数据抽象.
 * 
 * @Created on: 2015年1月10日 下午12:27:18
 * @Author: xuzewen
 * 
 */
public abstract class Gusr extends Actor
{
	/** 关联的N2H网络连接. */
	public N2H n2h = null;
	/** Gusr唯一标识. */
	public long uid;

	public Gusr(long uid)
	{
		super(ActorType.ITC);
		this.wk = Gsc.hashWk(uid);
		this.uid = uid;
	}

	/** ---------------------------------------------------------------- */
	/**                                                                  */
	/**  */
	/**                                                                  */
	/** ---------------------------------------------------------------- */
	/** 连接断开事件. */
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

	/** 处理长连接上的STMP-BEGIN. */
	public final void procBegin0(byte[] sec, Gn2htrans gt)
	{
		StmpNode root = StmpDec.unpack(this.decryp(sec));
		if (root == null)
		{
			this.close();
			return;
		}
		Short cmd = StmpDec.getShort(root, Stmp.STMP_TAG_CMD);
		if (cmd == null)
		{
			if (Log.isDebug())
				Log.debug("missing required field: STMP_TAG_CMD");
			this.close();
			return;
		}
		Cb cb = Gsc.evns.get(cmd.intValue());
		if (cb == null)
		{
			if (Log.isDebug())
				Log.debug("can not found call back for this cmd: %04X", cmd);
			this.close();
			return;
		}
		if (cb.gusr != true || cb.beginName == null) /* 不应该在这里收到. */
		{
			if (Log.isDebug())
				Log.debug("it`s an unexpected cmd: %s", cb.cmd);
			this.close();
			return;
		}
		byte[] pb = StmpDec.getBin(root, Stmp.STMP_TAG_DAT);
		if (pb == null) /* 为简单起见(避免在业务处理中频繁判断BEGIN是否为空), BEGIN中的pb对象为必选. */
		{
			if (Log.isDebug())
				Log.debug("missing required field: STMP_TAG_DAT");
			this.close();
			return;
		}
		try
		{
			gt.begin = cb.newBeginMsg(pb);
			gt.extbegin = StmpDec.getBin(root, Stmp.STMP_TAG_ATT);
			gt.cmd = cb.cmd;
			if (Log.isTrace())
			{
				Log.trace("\n  <-- BEGIN(%s): {%s}, END(%s), UID: %d, CMD: %s, TID: %08X, PEER: %s\n", //
						cb.beginName, Misc.pb2str(gt.begin), cb.endName, this.uid, gt.cmd, gt.tid, Net.getRemoteAddr(this.n2h.sc));
			}
			cb.cb.invoke(null, this, gt, gt.begin);
		} catch (Exception e)
		{
			if (Log.isDebug())
				Log.debug(Log.trace(e));
			this.close();
			return;
		}
	}

	/** 处理短连接上的STMP-BEGIN. */
	public final void procBegin1()
	{
		Log.warn("unsupported.");
	}

	/** END出栈. */
	public final void sendEnd0(Gn2htrans gt)
	{
		if (Log.isTrace())
		{
			Cb cb = Gsc.evns.get(gt.cmd.getNumber());
			if (cb != null)
			{
				Log.trace("\n  --> END(%s): {%s}, BEGIN(%s): {%s}, UID: %d, RET: %04X, CMD: %s, TID: %08X, PEER: %s\n", //
						cb.endName, gt.end.end == null ? "NULL" : Misc.pb2str(gt.end.end), //
						cb.beginName, gt.begin == null ? "NULL" : Misc.pb2str(gt.begin),//
						this.uid, gt.end.ret, gt.cmd, gt.tid, this.n2h == null ? "NULL" : Net.getRemoteAddr(this.n2h.sc));
			} else
				Log.fault("it`s a bug, cmd: %s", gt.cmd);
		}
		byte sec[] = this.encryp(Pro.pkgEndSecDat(gt.end.ret, gt.end.end, gt.end.extend)); /* 打包加密块. */
		StmpPdu end = new StmpPdu(sec.length);
		StmpEnc.addBin(end, Stmp.STMP_TAG_DAT, sec);
		StmpEnc.addInt(end, Stmp.STMP_TAG_DTID, gt.tid);
		StmpEnc.addTag(end, Stmp.STMP_TAG_TRANS_END);
		this.n2h.future(v -> {
			this.n2h.send(end.bytes()); /* 前转到N2H的线程出栈. */
			gt.finish();
		});
	}

	/** END出栈(短连接). */
	public final void sendEnd1()
	{
		Log.warn("unsupported.");
		return;
	}

	/** UNI出栈. */
	public final void senUni(ProtocolMessageEnum cmd, Message uni, byte[] ext)
	{
		if (Log.isTrace())
		{
			Cb cb = Gsc.evns.get(cmd.getNumber());
			if (cb != null)
			{
				Log.trace("\n  --> UNI(%s): {%s}, UID: %d, CMD: %s, PEER: %s\n", //
						cb.uniName, uni == null ? "NULL" : Misc.pb2str(uni),//
						this.uid, cmd, this.n2h == null ? "NULL" : Net.getRemoteAddr(this.n2h.sc));
			} else
				Log.fault("it`s a bug, cmd: %s", cmd);
		}
		byte[] by = this.encryp(Pro.pkgUniSecDat(cmd, uni, ext)); /* 加密. */
		StmpPdu pdu = new StmpPdu(by.length);
		StmpEnc.addBin(pdu, Stmp.STMP_TAG_DAT, by);
		StmpEnc.addTag(pdu, Stmp.STMP_TAG_TRANS_UNI);
		this.n2h.future(v -> this.n2h.send(pdu.bytes())); /* 前转到N2H的线程出栈. */
		if (Gsc.ec != null)
			Gsc.ec.gn2hSendUniOnGusr(this, cmd, uni, ext);
	}

	/** 是否在线. */
	public final boolean isOnline()
	{
		return this.n2h != null;
	}

	/** 强制断开连接(延迟操作). */
	public final void lazyKick()
	{
		N2H n2h = this.n2h;
		n2h.future(v -> n2h.lazyClose());
		this.n2h = null;
	}

	/** 踢下线(将导致连接被延迟关闭). */
	public final void kick(ProtocolMessageEnum cmd, Builder<?> b)
	{
		Message msg = b == null ? null : b.build();
		byte[] by = this.encryp(Pro.pkgUniSecDat(cmd, msg, null)); /* 加密. */
		StmpPdu pdu = new StmpPdu(by.length);
		StmpEnc.addBin(pdu, Stmp.STMP_TAG_DAT, by);
		StmpEnc.addTag(pdu, Stmp.STMP_TAG_TRANS_UNI);
		N2H n2h = this.n2h;
		n2h.future(v -> {
			n2h.send(pdu.bytes()); /* 消息出栈. */
			n2h.lts = System.currentTimeMillis() + Cfg.libgsc_n2h_lazy_close;
			n2h.lazyClose(); /* 延迟关闭. */
			n2h.gusr = null;
		});
		this.n2h = null;
		if (Gsc.ec != null)
			Gsc.ec.gn2hSendUniOnGusr(this, cmd, msg, null);
	}

	/** 踢下线, 无消息提示(连接被立即关闭). */
	public final void kickNoMsg()
	{
		N2H n2h = this.n2h;
		n2h.future(v -> {
			n2h.getGworker().removeActorNet(n2h); /* 关闭连接. */
			/**
			 *
			 * 这里为了达到静默关闭目的, 没调用n2h->close();
			 *
			 * */
		});
		this.n2h = null;
	}

	/** 关闭连接, 断开与N2H之间的关联. */
	private final void close()
	{
		N2H n2h = this.n2h;
		n2h.future(v -> {
			n2h.getGworker().removeActorNet(n2h);
			n2h.evnDis();
		});
	}
}
