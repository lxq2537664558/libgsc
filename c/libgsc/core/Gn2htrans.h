/*
 * Gn2htrans.h
 *
 *  Created on: Jan 31, 2015
 *      Author: root
 */

#ifndef GN2HTRANS_H_
#define GN2HTRANS_H_

#if !defined (__LIBGSCH__) && !defined (LIBGSC)
#error only libgsc.h can be included directly.
#endif

#include "../actor/N2H.h"

/**
 *
 * N2H事务(网络发起), 事务总是在N2H线程上发起, 在N2H线程上结束.
 *
 */
class Gn2htrans
{
public:
	int lzmsec; /** 连接延迟关闭(毫秒). */
	ushort cmd; /** 消息命令字. */
	uint tid; /** 事务id. */
	Message* begin; /** 事务上BEGIN消息. */
	string* extbegin; /** BEGIN消息上的扩展数据. */
	ushort ret; /** 处理结果. */
	Message* endx; /** 事务上END息. */
	string* extend; /** END消息上的扩展数据. */
	N2H* n2h; /** 事务发起者. */
public:
	Gn2htrans(N2H* n2h, ushort cmd, uint tid, Message* begin, string* extbegin);
	virtual ~Gn2htrans();
	void finish(); /** 事务结束. */
public:
	void end(ushort ret, Message* end, string* extend = NULL); /** 事务结束(事务总是在N2H关联的线程上结束). */
	void success(string* extend = NULL); /** 事务结束. */
	void end(Message* end, string* extend = NULL); /** 事务结束. */
	void end(ushort ret, string* extend = NULL); /** 事务结束. */
	void successLazyClose(int sec = 0); /** 事务结束. */
	void endLazyClose(Message* end, int sec = 0); /** 事务结束. */
	void endLazyClose(ushort ret, int sec = 0); /** 事务结束. */
	void failure(); /** 事务结束. */
};

#endif /* GN2HTRANS_H_ */
