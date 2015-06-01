/*
 * Gn2h.h
 *
 *  Created on: Mar 4, 2015 2:47:14 PM
 *      Author: xuzewen
 */

#ifndef GN2H_H_
#define GN2H_H_

#if !defined (__LIBGSCH__) && !defined (LIBGSC)
#error only libgsc.h can be included directly.
#endif

#include "../actor/Actor.h"
class Gusr;
class N2H;

/**
 *
 * N2H短连接事务(网络发起), 事务总是在N2H线程上发起, 在N2H线程上结束.
 *
 */
class Gn2h
{
public:
	Gusr* gusr; /** 应用层. */
	ullong sid; /** 会话id. */
	ushort cmd; /** 消息命令字. */
	uint tid; /** 事务id. */
	Message* begin; /** 事务上BEGIN消息. */
	string * extbegin; /** BEGIN消息上的扩展数据. */
	ushort ret; /** 处理结果. */
	Message* endx; /** 事务上END消息. */
	string* extend; /** END消息上的扩展数据. */
	N2H* n2h; /** 事务发起者. */
public:
	Gn2h(Gusr* gusr, N2H* n2h, ullong sid, ushort cmd, uint tid, Message* begin, string* extbegin);
	virtual ~Gn2h();
public:
	void end(ushort ret, Message* end, string* extend = NULL); /** 事务结束(事务总是在N2H关联的线程上结束). */
	void success(string* extend = NULL); /** 事务结束. */
	void end(Message* end, string* extend = NULL); /** 事务结束. */
	void end(ushort ret, string* extend = NULL); /** 事务结束. */
	void failure(); /** 事务结束. */
	void finish(); /** 事务结束. */
};

#endif /* GN2H_H_ */
