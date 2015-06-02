package cn;

import gcpb.NetGcWithGas.GcAuthWithGasReq;
import gcpb.NetGcWithGas.GcAuthWithGasRsp;
import actor.Gusr;
import actor.H2N;
import actor.N2H;

import com.google.protobuf.Descriptors.EnumDescriptor;
import com.google.protobuf.Descriptors.EnumValueDescriptor;
import com.google.protobuf.Message;
import com.google.protobuf.ProtocolMessageEnum;

import core.EvnCb;
import core.Gh2ntrans;
import core.Gn2htrans;
import core.Gsc;

/**
 * 
 * 事务输出.
 *
 * @Created on: 2015年1月9日 上午11:09:54
 * @Author: xuzewen
 * 
 */
public class Gcb implements EvnCb
{
	private static final Gcb gcb = new Gcb();

	/** libgsc为服务端, 接收到所有可能的命令字定义. */
	public enum CmdGas implements ProtocolMessageEnum
	{
		/** 鉴权. */
		GAS_REQ_GC_AUTH;

		public EnumDescriptor getDescriptorForType()
		{
			return null;
		}

		public int getNumber()
		{
			return this.ordinal();
		}

		public EnumValueDescriptor getValueDescriptor()
		{
			return null;
		}
	}

	public static final Gcb instance()
	{
		return gcb;
	}

	/** 注册消息回调. */
	public final void reg()
	{
		Gsc.regN2HEvn(CmdGas.GAS_REQ_GC_AUTH, GcAuthWithGasReq.class, GcAuthWithGasRsp.class, null, GcMsgAuth.class, false);
	}

	/** ---------------------------------------------------------------- */
	/**                                                                  */
	/**                                                                  */
	/**                                                                  */
	/** ---------------------------------------------------------------- */
	/** N2H事务结束. */
	public final void gn2hTrans(Gn2htrans gt)
	{

	}

	/** N2H上的STMP-UNI消息(由libgsc发出). */
	public final void gn2hSendUni(N2H n2h, ProtocolMessageEnum cmd, Message uni, byte[] ext)
	{

	}

	/** Gusr上的STMP-UNI消息(由libgsc发出). */
	public final void gn2hSendUniOnGusr(Gusr gusr, ProtocolMessageEnum cmd, Message uni, byte[] ext)
	{

	}

	/** N2H上的STMP-UNI消息(由libgsc收到). */
	public final void gn2hRecvUni(N2H n2h, ProtocolMessageEnum cmd, Message uni, byte[] ext)
	{

	}

	/** N2H上的STMP-UNI消息(由libgsc收到). */
	public final void gn2hRecvUniOnGusr(Gusr gusr, ProtocolMessageEnum cmd, Message uni, byte[] ext)
	{

	}

	/** H2N事务结束. */
	public final void gh2nTrans(Gh2ntrans gt)
	{

	}

	/** H2N上的STMP-UNI消息(由libgsc接收). */
	public final void gh2nRecvUni(H2N h2n, ProtocolMessageEnum cmd, Message uni, byte[] ext)
	{

	}

	/** H2N上的STMP-UNI消息(由libgsc发出). */
	public final void gh2nSendUni(H2N h2n, ProtocolMessageEnum cmd, Message uni, byte[] ext)
	{

	}

	/** 定时器振荡. */
	public final void quartz(long now)
	{

	}
}
