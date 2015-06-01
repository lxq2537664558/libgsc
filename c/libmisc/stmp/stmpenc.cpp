/*
 * stmpenc.c
 *
 *  Created on: 2013-8-26
 *      Author: xuzewen
 */

#include "stmpenc.h"
#include "../misc/Net.h"

static const uchar __STMP_LEN_0xFE__ = 0xFE;
static const uchar __STMP_LEN_0xFF__ = 0xFF;
static void stmpenc_add_tag__(stmp_pdu* sp, ushort t, uint len);

/** 向sp添加一个uchar值, t一定是一个简单体. */
void stmpenc_add_char(stmp_pdu* sp, ushort t, uchar v)
{
	stmpenc_add_bin(sp, t, &v, 1);
}

/** 向sp添加一个字符串值, t一定是一个简单体. */
void stmpenc_add_str(stmp_pdu* sp, ushort t, char* v)
{
	stmpenc_add_bin(sp, t, (uchar*) v, strlen(v));
}

/** 向sp添加一个ushort值, t一定是一个简单体. */
void stmpenc_add_short(stmp_pdu* sp, ushort t, ushort v)
{
	ushort x = htons(v);
	stmpenc_add_bin(sp, t, (uchar*) &x, sizeof(ushort));
}

/** 向sp添加一个uint值, t一定是一个简单体. */
void stmpenc_add_int(stmp_pdu* sp, ushort t, uint v)
{
	uint x = htonl(v);
	stmpenc_add_bin(sp, t, (uchar*) &x, sizeof(uint));
}

/** 向sp添加一个ullong值, t一定是一个简单体. */
void stmpenc_add_long(stmp_pdu* sp, ushort t, ullong v)
{
	ullong n = Net::htonll(v);
	stmpenc_add_bin(sp, t, (uchar*) &n, 8);
}

/** 向sp添加一串二制值. */
void stmpenc_add_bin(stmp_pdu* sp, ushort t, uchar* v, uint l)
{
	uchar ll = stmp_tlv_len(l);
	uchar tl;
	if (t & 0xFF00)
	{
		tl = 2;
		ushort tag = htons(t);
		sp->rm -= (ll + tl + l);
		memcpy(sp->buff + sp->rm, &tag, 2);
	} else
	{
		tl = 1;
		uchar tag = (uchar) t;
		sp->rm -= (ll + tl + l);
		memcpy(sp->buff + sp->rm, &tag, 1);
	}
	if (ll == 1)
	{
		uchar len = (uchar) l;
		memcpy(sp->buff + sp->rm + tl, &len, 1);
	} else if (ll == 3)
	{
		ushort len = htons(l);
		memcpy(sp->buff + sp->rm + tl, &__STMP_LEN_0xFE__, 1);
		memcpy(sp->buff + sp->rm + tl + 1, &len, 2);
	} else if (ll == 5)
	{
		uint len = htonl(l);
		memcpy(sp->buff + sp->rm + tl, &__STMP_LEN_0xFF__, 1);
		memcpy(sp->buff + sp->rm + tl + 1, &len, 4);
	}
	memcpy(sp->buff + sp->rm + tl + ll, v, l);
}

/** 向sp添加一个构造体tag. */
void stmpenc_add_tag(stmp_pdu* sp, ushort t)
{
	stmpenc_add_tag__(sp, t, sp->len - sp->rm);
}

/** 返回构造好的stmp报文, 返回值在栈上, 指向了sp内部的缓冲区, 因此不需要释放. */
uchar* stmpenc_take(stmp_pdu* sp, uint* len)
{
	*len = sp->len - sp->rm;
	return sp->buff + sp->rm;
}

/** 记住sp的当前位置, 将从这里开始添加一个子构造体的信元(这些信元必需都是简单体). */
void stmpenc_set_point(stmp_pdu* sp)
{
	sp->p = sp->rm;
}

/** 向sp添加一个构造体tag, stmpenc_set_point总是应该在此函数之前调用. */
void stmpenc_add_tag4point(stmp_pdu* sp, ushort t)
{
	stmpenc_add_tag__(sp, t, sp->p - sp->rm);
}

static void stmpenc_add_tag__(stmp_pdu* sp, ushort t, uint len)
{
	uchar ll = stmp_tlv_len(len);
	uchar tl;
	if (t & 0xFF00)
	{
		tl = 2;
		ushort tag = htons(t);
		sp->rm -= (ll + tl);
		memcpy(sp->buff + sp->rm, &tag, 2);
	} else
	{
		tl = 1;
		uchar tag = (uchar) t;
		sp->rm -= (ll + tl);
		memcpy(sp->buff + sp->rm, &tag, 1);
	}
	if (ll == 1)
	{
		uchar x = (uchar) len;
		memcpy(sp->buff + sp->rm + tl, &x, 1);
	} else if (ll == 3)
	{
		ushort x = htons(len);
		memcpy(sp->buff + sp->rm + tl, &__STMP_LEN_0xFE__, 1);
		memcpy(sp->buff + sp->rm + tl + 1, &x, 2);
	} else if (ll == 5)
	{
		uint x = htonl(len);
		memcpy(sp->buff + sp->rm + tl, &__STMP_LEN_0xFF__, 1);
		memcpy(sp->buff + sp->rm + tl + 1, &x, 4);
	}
}

/** 重置stmp_pdu, 以便重新填充内容. */
void stmpenc_reset(stmp_pdu* sp)
{
	sp->rm = sp->len;
}
