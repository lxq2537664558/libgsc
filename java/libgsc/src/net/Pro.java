package net;

import java.lang.reflect.InvocationTargetException;

import misc.Log;
import stmp.Stmp;
import stmp.StmpDec;
import stmp.StmpEnc;
import stmp.StmpNode;
import stmp.StmpPdu;

import com.google.protobuf.InvalidProtocolBufferException;
import com.google.protobuf.Message;
import com.google.protobuf.ProtocolMessageEnum;

import core.Cb;
import core.Gn2htrans;
import core.Gsc;

/**
 * 
 * libgsc的通信协议定义.
 *
 * @Created on: 2015年1月9日 下午1:04:31
 * @Author: xuzewen
 * 
 */
public class Pro
{
	/** 打包一个STMP-BEGIN. */
	public static final byte[] pkgBegin(ProtocolMessageEnum cmd, int tid, byte pb[], byte[] ext)
	{
		int size = (pb == null ? 0 : pb.length) + (ext == null ? 0 : ext.length);
		StmpPdu begin = new StmpPdu(size);
		if (ext != null)
			StmpEnc.addBin(begin, Stmp.STMP_TAG_ATT, ext);
		if (pb != null)
			StmpEnc.addBin(begin, Stmp.STMP_TAG_DAT, pb);
		StmpEnc.addShort(begin, Stmp.STMP_TAG_CMD, (short) cmd.getNumber());
		StmpEnc.addInt(begin, Stmp.STMP_TAG_STID, tid);
		StmpEnc.addTag(begin, Stmp.STMP_TAG_TRANS_BEGIN);
		return begin.bytes();
	}

	/** 打包一个STMP-BEGIN的加密部分. */
	public static final byte[] pkgStmpBeginSecDat(ProtocolMessageEnum cmd, byte pb[], byte[] ext)
	{
		int size = (pb == null ? 0 : pb.length) + (ext == null ? 0 : ext.length);
		StmpPdu sec = new StmpPdu(size);
		if (ext != null)
			StmpEnc.addBin(sec, Stmp.STMP_TAG_ATT, ext);
		if (pb != null)
			StmpEnc.addBin(sec, Stmp.STMP_TAG_DAT, pb);
		StmpEnc.addShort(sec, Stmp.STMP_TAG_CMD, (short) cmd.getNumber());
		StmpEnc.addTag(sec, Stmp.STMP_TAG_SEC);
		return sec.bytes();
	}

	/** 解包一个BEGIN(长连接, 未加密). */
	public static final Gn2htrans unpkgBegin(StmpNode root) throws InvalidProtocolBufferException, IllegalAccessException, IllegalArgumentException, InvocationTargetException
	{
		Integer tid = StmpDec.getInt(root, Stmp.STMP_TAG_STID);
		if (tid == null)
		{
			if (Log.isDebug())
				Log.debug("missing required field: STMP_TAG_STID");
			return null;
		}
		Short cmd = StmpDec.getShort(root, Stmp.STMP_TAG_CMD);
		if (cmd == null)
		{
			if (Log.isDebug())
				Log.debug("missing required field: STMP_TAG_CMD");
			return null;
		}
		Cb cb = Gsc.evns.get(cmd.intValue());
		if (cb == null) /* 不支持的命令字 . */
		{
			if (Log.isDebug())
				Log.debug("can not found call back for this cmd: %04X", cmd);
			return null;
		}
		if (cb.beginName == null)
		{
			if (Log.isDebug())
				Log.debug("it`s an unexpected cmd: %04X", cmd);
			return null;
		}
		byte[] dat = StmpDec.getBin(root, Stmp.STMP_TAG_DAT);
		if (dat == null)
		{
			if (Log.isDebug())
				Log.debug("missing required field: STMP_TAG_DAT");
			return null;
		}
		return new Gn2htrans(null, cb.cmd, tid, cb.newBeginMsg(dat), StmpDec.getBin(root, Stmp.STMP_TAG_ATT));
	}

	/** 打包一个STMP-END. */
	public static final byte[] pkgEnd(short ret, int tid, Message end, byte[] ext)
	{
		byte[] pb = (end == null ? null : end.toByteArray());
		int size = (pb == null ? 0 : pb.length) + (ext == null ? 0 : ext.length);
		StmpPdu pdu = new StmpPdu(size);
		if (ext != null)
			StmpEnc.addBin(pdu, Stmp.STMP_TAG_ATT, ext);
		if (pb != null && pb.length > 0)
			StmpEnc.addBin(pdu, Stmp.STMP_TAG_DAT, pb);
		StmpEnc.addShort(pdu, Stmp.STMP_TAG_RET, ret);
		StmpEnc.addInt(pdu, Stmp.STMP_TAG_DTID, tid);
		StmpEnc.addTag(pdu, Stmp.STMP_TAG_TRANS_END);
		return pdu.bytes();
	}

	/** 打包一个STMP-END中的加密部分. */
	public static final byte[] pkgEndSecDat(short ret, Message end, byte[] ext)
	{
		byte[] pb = (end == null ? null : end.toByteArray());
		int size = (pb == null ? 0 : pb.length) + (ext == null ? 0 : ext.length);
		StmpPdu pdu = new StmpPdu(size);
		if (ext != null)
			StmpEnc.addBin(pdu, Stmp.STMP_TAG_ATT, ext);
		if (pb != null && pb.length > 0)
			StmpEnc.addBin(pdu, Stmp.STMP_TAG_DAT, pb);
		StmpEnc.addShort(pdu, Stmp.STMP_TAG_RET, ret);
		StmpEnc.addTag(pdu, Stmp.STMP_TAG_SEC);
		return pdu.bytes();
	}

	/** 打包一个STMP-UNI中的加密部分. */
	public static final byte[] pkgUniSecDat(ProtocolMessageEnum cmd, Message uni, byte[] ext)
	{
		byte[] pb = (uni == null ? null : uni.toByteArray());
		int size = (pb == null ? 0 : pb.length) + (ext == null ? 0 : ext.length);
		StmpPdu pdu = new StmpPdu(size);
		if (ext != null)
			StmpEnc.addBin(pdu, Stmp.STMP_TAG_ATT, ext);
		if (pb != null && pb.length > 0)
			StmpEnc.addBin(pdu, Stmp.STMP_TAG_DAT, pb);
		StmpEnc.addShort(pdu, Stmp.STMP_TAG_CMD, (short) cmd.getNumber());
		StmpEnc.addTag(pdu, Stmp.STMP_TAG_SEC);
		return pdu.bytes();
	}
}
