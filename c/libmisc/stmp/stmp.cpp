/*
 * stmp.c
 *
 *  Created on: 2013-8-26
 *      Author: xuzewen
 */

#include "stmp.h"

typedef struct
{
	char d[0x20];
} stmp_ieis; /** 信元描述. */

static stmp_ieis __t_ieis__[] = { //
		/**0x60*/{ "TRANS_BEGIN" },
		/**0x61*/{ "TRANS_END" },
		/**0x62*/{ "TRANS_CONTINUE" },
		/**0x63*/{ "TRANS_SWITCH" },
		/**0x64*/{ "TRANS_ABORT" },
		/**0x65*/{ "TRANS_CANCEL" },
		/**0x66*/{ "TRANS_UNI" },
		/**0x67*/{ "SEC" },
		/**0x68*/{ "EXT" } };

static stmp_ieis __gs_ieis__[] = { //
		/**0x00*/{ "STID" },
		/**0x01*/{ "DTID" },
		/**0x02*/{ "UID" },
		/**0x03*/{ "SID" },
		/**0x04*/{ "CMD" },
		/**0x05*/{ "RET" },
		/**0x06*/{ "DAT" },
		/**0x07*/{ "ATT" }, };

static const char* __STMP_IU__ = "IEI_UNKNOWN";

/** 返回tag的字符串表现形式, 用于日志. */
char* stmp_tag_desc(ushort t)
{
	if (t >= STMP_TAG_TRANS_BEGIN && t < STMP_TAG_EXT)
		return __t_ieis__[t - STMP_TAG_TRANS_BEGIN].d;
	if (t <= STMP_TAG_EXT)
		return __gs_ieis__[t].d;
	return (char*) __STMP_IU__;
}

/** 获得len影响的l字段的长度, 可能的返回值是1, 表示l只需一个字表示, 3, 表示需两个字节表示(ushort), 5, 表示需4个字节表示(uint). */
uchar stmp_tlv_len(uint len)
{
	return len < 0x000000FE ? 1 : (len <= 0x0000FFFF ? 3 : 5);
}
