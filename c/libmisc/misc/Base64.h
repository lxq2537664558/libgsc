/*
 * Base64.h
 *
 *  Created on: 2012-7-8
 *      Author: xuzewen
 */

#ifndef BASE64_H_
#define BASE64_H_

#if !defined (__LIBMISC_H__) && !defined (LIBMISC)
#error only libmisc.h can be included directly.
#endif

#include "../macro.h"

/**
 *
 * BASE64编解码, 移植于glib2.
 *
 * */

class Base64
{
private:
	Base64();
	virtual ~Base64();
	static int encode_step(const uchar* in, int len, int break_lines, char* out, int* state, int* save);
	static int encode_close(int break_lines, char *out, int *state, int *save);
	static int decode_step(const char *in, int len, unsigned char *out, int *state, unsigned int *save);
public:
	static char* encode(const uchar* dat, int len); /** 编码, 返回值需要调用者自行释放. */
	static uchar* decode(const char* dat, int* out); /** 解码, 返回值需要调用者自行释放. */
};

#endif /* BASE64_H_ */
