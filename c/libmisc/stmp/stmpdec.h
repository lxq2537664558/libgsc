/*
 * stmpdec.h
 *
 *  Created on: 2013-8-26
 *      Author: Administrator
 */

#ifndef STMPDEC_H_
#define STMPDEC_H_

#if !defined (__LIBMISC_H__) && !defined (LIBMISC)
#error only libmisc.h can be included directly.
#endif

#include "stmp.h"

typedef struct stmp_node
{
	tlv self;
	struct stmp_node* s;
	struct stmp_node* c;
} stmp_node;

/** 从dat中解析出一个stmp_node结构, 成功时返回0. */
int stmpdec_unpack(uchar* dat, uint size, stmp_node* node);

/** 在指定的节点上返回一串二进制值, 如果找不到节点, return -1, 否则返回v的长度(可能为0), 超过max部分将被截断. */
int stmpdec_get_bin(stmp_node* node, ushort t, uchar* v, uint max);

/** 在指定的节点上返回一串二进制值, 如果找不到节点, return NULL, 否则其首地址, 并设len为其长度. */
uchar* stmpdec_peek_bin(stmp_node* node, ushort t, uint* len);

/** 在指定的节点上返回一个uchar值. */
int stmpdec_get_char(stmp_node* node, ushort t, uchar* v);

/** 在指定的节点上返回一个字符串值, 返回的strlen(str) 一定 <= max, 超长部分将被截断. */
int stmpdec_get_str(stmp_node* node, ushort t, char* str, uint max);

/** 在指定的节点上返回一个ushort值. */
int stmpdec_get_short(stmp_node* node, ushort t, ushort* v);

/** 在指定的节点上返回一个uint值. */
int stmpdec_get_int(stmp_node* node, ushort t, uint* v);

/** 在指定的节点上返回一个ullong值. */
int stmpdec_get_long(stmp_node* node, ushort t, ullong* v);

/** 在node上查找一个子node. */
int stmpdec_get_node(stmp_node* node, ushort t, stmp_node** c);

/** 释放一个stmp_node, 通常是释放root, 也就是根结点. */
void stmpdec_free(stmp_node* root);

/** 打印一个节点到标准输出.*/
void stmpdec_printnode(stmp_node* node);

/** 打印一个节点到str.*/
string stmpdec_printnode2str(stmp_node* node);

/** 解包, 并打印到标准输出. */
void stmpdec_printf(uchar* dat, uint size);

/** 解包, 并打印到标准输出. */
void stmpdec_printpdu(stmp_pdu* pdu);

/** 解包, 并打印到str. */
string stmpdec_print2str(uchar* data, uint size);

/** 解包, 并打印到str. */
string stmpdec_printpdu2str(stmp_pdu* pdu);

#endif /* STMPDEC_H_ */
