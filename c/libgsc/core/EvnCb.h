/*
 * EvnCb.h
 *
 *  Created on: Jan 31, 2015
 *      Author: root
 */

#ifndef EVNCB_H_
#define EVNCB_H_

#if !defined (__LIBGSCH__) && !defined (LIBGSC)
#error only libgsc.h can be included directly.
#endif

#include "Gn2h.h"
#include "../actor/N2H.h"
#include "../actor/H2N.h"

/**
 *
 *  消息日志接口.
 *
 */
class EvnCb
{
public:
	EvnCb();
	virtual ~EvnCb();
public:
	virtual void gn2hTrans(Gn2htrans* gt) = 0; /** N2H事务结束. */
	virtual void gn2h(Gn2h* gt) = 0; /** N2H短连接事务结束. */
	virtual void gn2hSendUni(N2H* n2h, ushort cmd, Message* uni, string* ext) = 0; /** N2H上的STMP-UNI消息(由libgsc发出). */
	virtual void gn2hSendUniOnGusr(Gusr* gusr, ushort cmd, Message* uni, string* ext) = 0; /** Gusr上的STMP-UNI消息(由libgsc发出). */
	virtual void gn2hRecvUni(N2H* n2h, ushort cmd, Message* uni, string* ext) = 0; /** N2H上的STMP-UNI消息(由libgsc收到). */
	virtual void gn2hRecvUniOnGusr(Gusr* gusr, ushort cmd, Message* uni, string* ext) = 0; /** N2H上的STMP-UNI消息(由libgsc收到). */
	//
	virtual void gh2nTrans(Gh2ntrans* gt) = 0; /** H2N事务结束. */
	virtual void gh2nRecvUni(H2N* h2n, ushort cmd, Message* uni, string* ext) = 0; /** H2N上的STMP-UNI消息(由libgsc接收). */
	virtual void gh2nSendUni(H2N* h2n, ushort cmd, Message* uni, string* ext) = 0; /** H2N上的STMP-UNI消息(由libgsc发出). */
	//
	virtual void quartz(ullong now) = 0; /** 定时器振荡. */
};

#endif /* EVNCB_H_ */
