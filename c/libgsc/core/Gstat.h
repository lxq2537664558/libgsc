/*
 * Gstat.h
 *
 *  Created on: Mar 11, 2015 6:34:19 PM
 *      Author: xuzewen
 */

#ifndef GSTAT_H_
#define GSTAT_H_

#if !defined (__LIBGSCH__) && !defined (LIBGSC)
#error only libgsc.h can be included directly.
#endif

#include <libmisc.h>

enum
{
	LIBGSC_RCV_BYTES = 0x00, /** 服务器收到的字节数. */
	LIBGSC_RCV_MSGS, /** 服务器收到的消息数. */
	LIBGSC_SND_BYTES, /** 服务器发出的字节数. */
	LIBGSC_SND_MSGS, /** 服务器发出的消息数. */
	//
	LIBGSC_N2H_TOTAL, /** 总区生成的N2H数量. */
	LIBGSC_N2H_DELETE, /** 调用了delete n2h的数量. */
	STAT_END
};

/**
 *
 * 监测与统计.
 *
 */
class Gstat
{
private:
	Gstat();
	virtual ~Gstat();
public:
	static void inc(uint item); /** 自增1. */
	static void incv(uint item, ullong v); /** 自增v. */
	static ullong get(uint item); /** 获得某项的值. */
};

#endif /* GSTAT_H_ */
