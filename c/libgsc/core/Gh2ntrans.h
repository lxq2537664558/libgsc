/*
 * Gh2ntrans.h
 *
 *  Created on: Feb 2, 2015 3:15:54 PM
 *      Author: xuzewen
 */

#ifndef GH2NTRANS_H_
#define GH2NTRANS_H_

#if !defined (__LIBGSCH__) && !defined (LIBGSC)
#error only libgsc.h can be included directly.
#endif

#include "../actor/H2N.h"

class Gh2ntrans
{
public:
	bool tm; /** 是否超时. */
	ushort cmd; /** 命令字. */
	ushort ret; /** 响应结果. */
	uint tid; /** 事务id. */
	H2N* h2n; /** 事务关联的H2N连接. */
	Message* begin; /** 事务上BEGIN消息. */
	string* extbegin; /** BEGIN消息上的扩展数据. */
	Message* end; /** 事务上END消息. */
	string* extend; /** END消息上的扩展数据. */
	//
	ullong gts; /** 事务产生时间. */
	function<void(ushort ret, Message* end, string* ext)> endCb;
	function<void()> tmCb;
public:
	Gh2ntrans(H2N* h2n, ushort cmd, Message* begin, function<void(ushort ret, Message* end, string* ext)>& end, function<void()>& tm, string* extbegin = NULL);
	virtual ~Gh2ntrans();
};

#endif /* GH2NTRANS_H_ */
