/*
 * stmpdec.c
 *
 *  Created on: 2013-8-26
 *      Author: xuzewen
 */

#include "stmpdec.h"
#include "../misc/Net.h"
#include "../misc/Logger.h"

static const uchar __STMP_LEN_0xFE__ = 0xFE;
static const uchar __STMP_LEN_0xFF__ = 0xFF;
static int stmpdec_unpack__(uchar* dat, uint size, stmp_node* node);
static int stmpdec_gen_tl(uchar* dat, uint size, ushort* t, uchar* tl, uchar* ll, uint* l);
static int stmpdec_search_s(stmp_node* node, ushort tag, stmp_node** r);
static int stmpdec_search(stmp_node* node, ushort tag, stmp_node** r);
static void stmpdec_print__(stmp_node* node, string* buff, int space);

/** 从dat中解析出一个stmp_node结构, 成功时返回0. */
int stmpdec_unpack(uchar* dat, uint size, stmp_node* node)
{
	memset(node, 0, sizeof(stmp_node));
	if (stmpdec_unpack__(dat, size, node) == 0)
		return 0;
	stmpdec_free(node);
	return 1;
}

/** 在指定的节点上返回一串二进制值, 如果找不到节点, return -1, 否则返回v的长度(可能为0), 超过max部分将被截断. */
int stmpdec_get_bin(stmp_node* node, ushort t, uchar* v, uint max)
{
	stmp_node* x = NULL;
	if (stmpdec_search(node, t, &x) != 0)
		return -1;
	if (x->self.l == 0)
		return 0;
	memcpy(v, x->self.v, x->self.l > max ? max : x->self.l);
	return x->self.l > max ? max : x->self.l;
}

/** 在指定的节点上返回一串二进制值, 如果找不到节点, return NULL, 否则其首地址, 并设len为其长度. */
uchar* stmpdec_peek_bin(stmp_node* node, ushort t, uint* len)
{
	stmp_node* x = NULL;
	if (stmpdec_search(node, t, &x) != 0)
		return NULL;
	if (x->self.l == 0)
		return NULL;
	*len = x->self.l;
	return x->self.v;
}

/** 在指定的节点上返回一个uchar值. */
int stmpdec_get_char(stmp_node* node, ushort t, uchar* v)
{
	stmp_node* x = NULL;
	if (stmpdec_search(node, t, &x) != 0)
		return 1;
	if (x->self.l != 1)
		return 1;
	memcpy(v, x->self.v, 1);
	return 0;
}

/** 在指定的节点上返回一个字符串值, 返回的strlen(str) 一定 <= max, 超长部分将被截断. */
int stmpdec_get_str(stmp_node* node, ushort t, char* str, uint max)
{
	stmp_node* x = NULL;
	if (stmpdec_search(node, t, &x) != 0)
		return 1;
	if (x->self.l < 1)
		return 1;
	memcpy(str, x->self.v, x->self.l > max ? max : x->self.l);
	return 0;
}

/** 在指定的节点上返回一个ushort值. */
int stmpdec_get_short(stmp_node* node, ushort t, ushort* v)
{
	stmp_node* x = NULL;
	if (stmpdec_search(node, t, &x) != 0)
		return 1;
	if (x->self.l != 2)
		return 1;
	memcpy(v, x->self.v, 2);
	*v = ntohs(*v);
	return 0;
}

/** 在指定的节点上返回一个uint值. */
int stmpdec_get_int(stmp_node* node, ushort t, uint* v)
{
	stmp_node* x = NULL;
	if (stmpdec_search(node, t, &x) != 0)
		return 1;
	if (x->self.l != 4)
		return 1;
	memcpy(v, x->self.v, 4);
	*v = ntohl(*v);
	return 0;
}

/** 在指定的节点上返回一个ullong值. */
int stmpdec_get_long(stmp_node* node, ushort t, ullong* v)
{
	stmp_node* x = NULL;
	if (stmpdec_search(node, t, &x) != 0)
		return 1;
	if (x->self.l != 8)
		return 1;
	*v = Net::ntohll(*((ullong*) x->self.v));
	return 0;
}

/** 在node上查找一个子node. */
int stmpdec_get_node(stmp_node* node, ushort t, stmp_node** c)
{
	return stmpdec_search(node, t, c);
}

/** 仅仅搜索node和node的sibling, 而不往更深处搜索. 找到返回0, 并设置r, 否则返回1. 表示未找到. */
int stmpdec_search_s(stmp_node* node, ushort tag, stmp_node** r)
{
	if (node->self.t == tag)
	{
		*r = node;
		return 0;
	}
	return node->s == NULL ? 1 : stmpdec_search_s(node->s, tag, r);
}

/** 仅仅搜索node和node的child, 而不往更深处搜索. 找到返回0, 并设置r, 否则返回1. 表示未找到. */
int stmpdec_search(stmp_node* node, ushort tag, stmp_node** r)
{
	if (node->self.t == tag)
	{
		*r = node;
		return 0;
	}
	return node->c == NULL ? 1 : stmpdec_search_s(node->c, tag, r);
}

void stmpdec_free(stmp_node* node)
{
	if (node->self.v != NULL)
	{
		//free(node->self.v);
		/** v与dat共享内存. */
		node->self.v = 0;
		node->self.l = 0;
	}
	if (node->c != NULL)
	{
		stmpdec_free(node->c);
		free(node->c);
		node->c = NULL;
	}
	if (node->s != NULL)
	{
		stmpdec_free(node->s);
		free(node->s);
		node->s = 0;
	}
}

int stmpdec_unpack__(uchar* dat, uint size, stmp_node* node)
{
	int ret = 1;
	uint l = 0;
	ushort t = 0;
	uchar tl = 0;
	uchar ll = 0;
	if (stmpdec_gen_tl(dat, size, &t, &tl, &ll, &l) != 0)
		return ret;
	node->self.t = t;
	node->self.l = l;
	if ((t & 0xFF00) ? (t & 0x4000) : (t & 0x40)) /** 构造体. */
	{
		node->c = (stmp_node*) calloc(1, sizeof(stmp_node));
		if (stmpdec_unpack__(dat + tl + ll, node->self.l, node->c) != 0)
			return ret;
	} else /** 简单体. */
	{
		//node->self.v = (uchar*) malloc(node->self.l);
		//memcpy(node->self.v, dat + tl + ll, node->self.l);
		node->self.v = dat + tl + ll; /** v与dat共享内存. */
	}
	uint cur = (tl + ll + node->self.l);
	uint rem = size - cur; /** 剩余字节. */
	if (rem != 0)
	{
		node->s = (stmp_node*) calloc(1, sizeof(stmp_node));
		return stmpdec_unpack__(dat + cur, rem, node->s);
	}
	return 0;
}

/** 获得tag(t), tag字段长度(tl), length字段长度(ll), value长度(l). */
int stmpdec_gen_tl(uchar* dat, uint size, ushort* t, uchar* tl, uchar* ll, uint* l)
{
	int ret = 1;
	if (size < 2)
	{
		LOG_TRACE("without enough size for unpack tag-field and length-field, size: %08X", size)
		return ret;
	}
	if (dat[0] & 0x80)
	{
		if (size < 3)
		{
			LOG_TRACE("without enough size for unpack length-field, size: %08X", size)
			return ret;
		}
		memcpy(t, dat, 2);
		*t = ntohs(*t);
		*tl = 2;
	} else
	{
		*t = dat[0];
		*tl = 1;
	}
	*ll = (dat[*tl] == __STMP_LEN_0xFE__ ? 3 : (dat[*tl] == __STMP_LEN_0xFF__ ? 5 : 1));
	if (size < (uint) (*tl + *ll))
	{
		LOG_TRACE("without enough size for unpack length-field, size: %08X, tl: %02X, ll: %02X", size, *tl, *ll)
		return ret;
	}
	if (*ll == 1) /** 一个字节表示长度. */
	{
		*l = dat[*tl];
		if (size < ((uint) (*tl + *ll)) + *l)
		{
			LOG_TRACE("without enough size for unpack length-field, size: %08X, tl: %02X, ll: %02X, l: %02X", size, *tl, *ll, *l)
			return ret;
		}
		return 0;
	}
	if (*ll == 3) /** 两个字节表示长度. */
	{
		memcpy(((uchar*) l), dat + (*tl + 1), 2);
		*l = ntohs(*((ushort*) l));
		if (size < ((uint) (*tl + *ll)) + *l)
		{
			LOG_TRACE("without enough size for unpack value-field, size: %08X, tl: %02X, ll: %02X, l: %04X", size, *tl, *ll, *l)
			return ret;
		}
		return 0;
	}
	if (*ll == 5) /** 四个字节表示长度. */
	{
		memcpy(l, dat + (*tl + 1), 4);
		*l = ntohl(*l);
		if (size < ((uint) (*tl + *ll)) + *l)
		{
			LOG_TRACE("without enough size for unpack value-field, size: %08X, tl: %02X, ll: %02X, l: %08X", size, *tl, *ll, *l)
			return ret;
		}
		return 0;
	}
	return ret;
}

/** 打印一个节点到str.*/
string stmpdec_printnode2str(stmp_node* node)
{
	string str;
	stmpdec_print__(node, &str, 0);
	return str;
}

void stmpdec_print__(stmp_node* node, string* buff, int space)
{
	int i = 0;
	for (; i < space; ++i)
		SPRINTF_STRING(buff, "%s", i == space - 1 ? " |" : " ")
	char* tstr = stmp_tag_desc(node->self.t);
	uchar ll = stmp_tlv_len(node->self.l);
	if ((node->self.t & 0xFF00) ? (node->self.t & 0x4000) : (node->self.t & 0x40)) /** 构造体. */
	{
		space += 4;
		if (ll == 1)
		{
			SPRINTF_STRING(buff, (node->self.t & 0xFF00) ? ("--%04X(%s)[%02X]\n") : ("----%02X(%s)[%02X]\n"), node->self.t, tstr, node->self.l)
		} else if (ll == 3)
		{
			SPRINTF_STRING(buff, (node->self.t & 0xFF00) ? ("--%04X(%s)[%04X]\n") : ("----%02X(%s)[%04X]\n"), node->self.t, tstr, node->self.l)
		} else
		{
			SPRINTF_STRING(buff, (node->self.t & 0xFF00) ? ("--%04X(%s)[%08X]\n") : ("----%02X(%s)[%08X]\n"), node->self.t, tstr, node->self.l)
		}
		i = 0;
		for (; i < space; ++i)
		{
			SPRINTF_STRING(buff, "%s", i == space - 1 ? " |\n" : " ")
		}
	} else
	{
		if (ll == 1)
			SPRINTF_STRING(buff, (node->self.t & 0xFF00) ? ("--%04X(%s)[%02X]={") : ("----%02X(%s)[%02X]={"), node->self.t, tstr, node->self.l)
		else if (ll == 3)
		{
			SPRINTF_STRING(buff, (node->self.t & 0xFF00) ? ("--%04X(%s)[%04X]={") : ("----%02X(%s)[%04X]={"), node->self.t, tstr, node->self.l)
		} else
		{
			SPRINTF_STRING(buff, (node->self.t & 0xFF00) ? ("--%04X(%s)[%08X]={") : ("----%02X(%s)[%08X]={"), node->self.t, tstr, node->self.l)
		}
		i = 0;
		for (; (uint) i < node->self.l; ++i)
		{
			if ((uint) i == node->self.l - 1)
			{
				SPRINTF_STRING(buff, "%02X", (uchar) (((uchar*) node->self.v)[i]))
			} else
			{
				SPRINTF_STRING(buff, "%02X ", (uchar) (((uchar*) node->self.v)[i]))
			}
		}
		SPRINTF_STRING(buff, "%s", "}\n");
		space += 4;
	}
	if (node->c != 0)
		stmpdec_print__(node->c, buff, space);
	if (node->s != 0)
		stmpdec_print__(node->s, buff, space - 4);
}

void stmpdec_printnode(stmp_node* node)
{
	printf("%s\n", stmpdec_printnode2str(node).c_str());
}

/** 解包, 并打印到标准输出. */
void stmpdec_printf(uchar* dat, uint size)
{
	stmp_node root;
	Misc::printhex(dat, size);
	if (stmpdec_unpack(dat, size, &root) == 0)
	{
		printf("%s\n", stmpdec_printnode2str(&root).c_str());
		stmpdec_free(&root);
		return;
	}
	LOG_TRACE("can not unpack this STMP PDU.")
}

/** 解包, 并打印到标准输出. */
void stmpdec_printpdu(stmp_pdu* pdu)
{
	stmpdec_printf(pdu->buff + pdu->rm, pdu->len - pdu->rm);
}

/** 解包, 并打印到str. */
string stmpdec_print2str(uchar* dat, uint size)
{
	stmp_node root;
	if (stmpdec_unpack(dat, size, &root) == 0)
	{
		string str = stmpdec_printnode2str(&root);
		stmpdec_free(&root);
		return str;
	}
	LOG_TRACE("can not unpack this STMP PDU.")
	return "";
}

/** 解包, 并打印到str. */
string stmpdec_printpdu2str(stmp_pdu* pdu)
{
	return stmpdec_print2str(pdu->buff + pdu->rm, pdu->len - pdu->rm);
}
