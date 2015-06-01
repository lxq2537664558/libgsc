/*
 * stmp.h
 *
 *  Created on: 2013-8-26
 *      Author: xuzewen
 */

#ifndef STMP_H_
#define STMP_H_

#if !defined (__LIBMISC_H__) && !defined (LIBMISC)
#error only libmisc.h can be included directly.
#endif

#include "../misc/Misc.h"

typedef struct
{
	ushort t; /** tag. */
	uint l; /** length. */
	uchar* v; /** value. */
} tlv;

typedef struct
{
	int len; /** buff的总长度. */
	int rm; /** remain, 剩余. */
	int p; /** position, 位置. */
	uchar* buff;
} stmp_pdu;

/** ---------------------------------------------------------------- */
/**                                                                  */
/** 事务部分. */
/**                                                                  */
/** ---------------------------------------------------------------- */
#define STMP_TAG_TRANS_BEGIN								0x60		/** 事务开始. */
#define STMP_TAG_TRANS_END									0x61		/** 事务结束. */
#define STMP_TAG_TRANS_CONTINUE								0x62		/** 事务继续. */
#define STMP_TAG_TRANS_SWITCH								0x63		/** 消息前转. */
#define STMP_TAG_TRANS_ABORT								0x64		/** 事务中断. */
#define STMP_TAG_TRANS_CANCEL								0x65		/** 事务取消. */
#define STMP_TAG_TRANS_UNI									0x66		/** 单向消息. */
/** ---------------------------------------------------------------- */
/**                                                                  */
/** 一般信元. */
/**                                                                  */
/** ---------------------------------------------------------------- */
#define STMP_TAG_SEC										0x67		/** 加密块. */
#define STMP_TAG_EXT										0x68		/** 扩展. */
//
#define STMP_TAG_STID										0x00		/** 源事务ID. */
#define STMP_TAG_DTID										0x01		/** 目的事务ID. */
#define STMP_TAG_UID										0x02		/** 用户标识. */
#define STMP_TAG_SID										0x03		/** 会话标识. */
#define STMP_TAG_CMD										0x04		/** 命令字. */
#define STMP_TAG_RET										0x05		/** 返回值. */
#define STMP_TAG_DAT										0x06		/** 数据. */
#define STMP_TAG_ATT										0x07		/** 附件(attachment). */
/** ---------------------------------------------------------------- */
/**                                                                  */
/** 返回值. */
/**                                                                  */
/** ---------------------------------------------------------------- */
#define RET_SUCCESS											0x0000		/** 成功. */
#define RET_FAILURE											0x0001		/** 失败. */
#define RET_INVALID											0x0002		/** 无效. */
#define RET_PRESENT											0x0003		/** 有. */
#define RET_NOT_PRESENT										0x0004		/** 没有. */
#define RET_EXCEPTION										0x0005		/** 异常. */
#define RET_NOT_FOUND										0x0006		/** 找不到. */
#define RET_TIME_OUT										0x0007		/** 超时. */
#define RET_SID_ERROR										0x0008		/** 会话id错误, 适用于短连接. */
#define RET_USR_DEF											0x0010		/** 用户自定义返回值起始. */

/** 返回tag的字符串描述形式. */
char* stmp_tag_desc(ushort t);

/** 获得len导致的length字段的长度. */
uchar stmp_tlv_len(uint len);

#include "stmpenc.h"
#include "stmpdec.h"

#endif /* STMP_H_ */
