package core;

import actor.Gusr;
import actor.H2N;
import actor.N2H;

import com.google.protobuf.Message;
import com.google.protobuf.ProtocolMessageEnum;

/**
 * 
 * libgsc系统事件回调.
 *
 * @Created on: 2015年1月16日 下午1:33:48
 * @Author: xuzewen
 * 
 */
public interface EvnCb
{
	/** N2H事务结束. */
	void gn2hTrans(Gn2htrans gt);

	/** N2H上的STMP-UNI消息(由libgsc发出). */
	void gn2hSendUni(N2H n2h, ProtocolMessageEnum cmd, Message uni, byte[] ext);

	/** Gusr上的STMP-UNI消息(由libgsc发出). */
	void gn2hSendUniOnGusr(Gusr gusr, ProtocolMessageEnum cmd, Message uni, byte[] ext);

	/** N2H上的STMP-UNI消息(由libgsc收到). */
	void gn2hRecvUni(N2H n2h, ProtocolMessageEnum cmd, Message uni, byte[] ext);

	/** N2H上的STMP-UNI消息(由libgsc收到). */
	void gn2hRecvUniOnGusr(Gusr gusr, ProtocolMessageEnum cmd, Message uni, byte[] ext);

	/** H2N事务结束. */
	void gh2nTrans(Gh2ntrans gt);

	/** H2N上的STMP-UNI消息(由libgsc接收). */
	void gh2nRecvUni(H2N h2n, ProtocolMessageEnum cmd, Message uni, byte[] ext);

	/** H2N上的STMP-UNI消息(由libgsc发出). */
	void gh2nSendUni(H2N h2n, ProtocolMessageEnum cmd, Message uni, byte[] ext);

	/** 定时器振荡. */
	void quartz(long now);
}
