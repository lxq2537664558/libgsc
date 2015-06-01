/*
 * Cb.h
 *
 *  Created on: Jan 31, 2015
 *      Author: root
 */

#ifndef CB_H_
#define CB_H_

#if !defined (__LIBGSCH__) && !defined (LIBGSC)
#error only libgsc.h can be included directly.
#endif

#include "../ginc.h"

class Cb
{
public:
	bool gusr; /** 鉴权前/后. */
	ushort cmd;
	void* cb;
	Descriptor* begin;
	Descriptor* end;
	Descriptor* uni;
public:
	Cb(ushort cmd, void* cb, const Descriptor* begin, const Descriptor* end, const Descriptor* uni, bool gusr);
	virtual ~Cb();
};

#endif /* CB_H_ */
