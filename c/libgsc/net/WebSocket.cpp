/*
 * WebSocket.cpp
 *
 *  Created on: Mar 27, 2015 1:55:12 PM
 *      Author: xuzewen
 */

#include "WebSocket.h"
#include "../core/Gstat.h"
#include "../core/Cfg.h"

WebSocket::WebSocket()
{

}

/** 解析报文帧. */
bool WebSocket::unpack(ActorNet* an)
{
	//		 81 85 C7 5B 2E F3 AF 3E 42 9F A8
	//		 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	//		 +-+-+-+-+-------+-+-------------+-------------------------------+
	//		 |F|R|R|R| opcode|M| Payload len | Extended payload length |
	//		 |I|S|S|S| (4) |A| (7) | (16/64) |
	//		 |N|V|V|V| |S| | (if payload len==126/127) |
	//		 | |1|2|3| |K| | |
	//		 +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - -
	uchar* dat = an->rbuf;
	int len = an->dlen;
	for (;;)
	{
		if (len < 2) /** 不完整的报文, */
			break;
		if (!(dat[0] & 0x80)) /** FIN, 是否为最后一帧. */
		{
			LOG_DEBUG("unsupported multi-frame.")
			return false;
		}
		char opcode = (dat[0] & 0x0F); /** 操作码. */
		if (!(dat[1] & 0x80)) /** MASK, 是否使用了掩码. */
		{
			LOG_DEBUG("client MUST use mask.")
			return false;
		}
		switch (opcode)
		{
		case WS_FRAME_CLOSE: /** 直接关闭. */
			LOG_DEBUG("got a web socket close request, we will close this socket immediate: %s", an->toString().c_str())
			return false;
		case WS_FRAME_CONTINUATION: /** 不处理. */
		case WS_FRAME_PING:
		case WS_FRAME_PONG:
			LOG_DEBUG("unsupported opcode: %02X", opcode)
			return true;
		case WS_FRAME_TEXT:
		case WS_FRAME_BINARY:
			break; /** 去下面. */
		default:
			LOG_DEBUG("unsupported opcode: %02X", opcode)
			return false;
		}
		if (dat[1] == 0x80)
		{
			LOG_DEBUG("need 1 byte at least.")
			return false;
		}
		if (dat[1] == 0xFF) /** 不支持超过两个字节长度的报文. */
		{
			LOG_DEBUG("unsupported over 64K PDU.")
			return false;
		}
		int size;
		if (dat[1] < 0xFE)
			size = 1/** 首字节 */+ 1 /** len字段本身 */+ 4 /** 4字节掩码. */+ (dat[1] & 0x7F) /** 内容长度. */;
		else /** 有两字节表示长度. */
		{
			ushort s;
			memcpy(&s, dat + 2, 2);
			s = ntohs(s); /** 取出长度. */
			size = 1 /** 首字节 */+ 3 /** len字段本身 */+ 4 /** 4字节掩码. */+ s /** 内容长度. */;
		}
		if (size > Cfg::libgsc_peer_mtu)
		{
			LOG_DEBUG("packet format error, we will close this connection, peer: %s, size: %08X", Net::sockaddr2str(&an->peer).c_str(), size)
			return false;
		}
		if (len < size) /** 还未到齐. */
			break;
		//
		if (an->type == ACTOR_N2H && ((N2H*) an)->lz) /** 这里有一个问题: 一次性收到多个消息时, 如果单个消息处理完, 需要延迟关闭套接字, 后面的将还会继续处理, 所有这里作一次判断. */
		{
			dat += size; /** 直接越过, 不处理, 等待客户端的websocket-close报文, 或者延迟关闭执行. */
			len -= size;
			continue;
		}
		//
		uchar mask[4];
		memcpy(&mask, (dat[1] < 0xFE) ? dat + 2 : dat + 4, 4);
		int ofst = (dat[1] < 0xFE) ? 6 : 8;
		for (int i = ofst; i < size; i++) /** 解码. */
			dat[i] = (dat[i] ^ mask[(i - ofst) % 4]);
		//
		an->lts = DateMisc::getMsec(); /** 更新最后收到消息的时间戳. */
		if (!WebSocket::ws_frame_binary(dat + ofst, size - ofst, an))
			return false;
		dat += size;
		len -= size;
	}
	if (len != an->dlen)
	{
		for (int i = 0; i < len; ++i)
			an->rbuf[i] = dat[i];
		an->dlen = len;
	}
	return true;
}

/** 一进制帧. */
bool WebSocket::ws_frame_binary(uchar* dat, int len, ActorNet* an)
{
	stmp_node root;
	if (stmpdec_unpack(dat, len, &root) != 0)
	{
		LOG_DEBUG("STMP protocol error.")
		return false;
	}
	LOG_RECORD("\n  <-- peer: %s    cfd: %d\n%s", Net::sockaddr2str(&an->peer).c_str(), an->cfd, stmpdec_printnode2str(&root).c_str())
	Gstat::inc(LIBGSC_RCV_MSGS);
	if (!an->evnMsg(&root))
	{
		stmpdec_free(&root);
		return false;
	}
	stmpdec_free(&root);
	return true;
}

/** 检查握手协议. */
bool WebSocket::checkHandShake(ActorNet* an)
{
	uchar* dat = an->rbuf;
	int len = an->dlen;
	bool r = false;
	int ofst = 0;
	for (int i = len - 1; i >= 4; i -= 4)
	{
		if (dat[i] == 0x0A && dat[i - 1] == 0x0D && dat[i - 2] == 0x0A && dat[i - 3] == 0x0D)
		{
			dat[i - 1] = 0; /** 最后一个0D 0A 作为字符串截止符. */
			ofst = i;
			r = true;
			break;
		}
	}
	if (!r) /** 不是个完整协议报文. */
		return false;
	string swk;
	if (!WebSocket::parseSwk((char*) dat, &swk))
		return false;
	WebSocket::sendHandShake(an, &swk);
	for (int i = ofst + 1; i < len; ++i) /** 如果后面还有报文, 将其移动到缓冲区起始位置. */
		an->rbuf[0] = dat[i];
	an->dlen = len - ofst - 1;
	return true;
}

/** 从握手协议中取出Sec-WebSocket-Key. */
bool WebSocket::parseSwk(const char* dat, string* swk)
{
	static const char* key = "Sec-WebSocket-Key:";
	static const int keylen = strlen(key);
	//
	vector<string> arr;
	Misc::split((char*) dat, "\r\n", &arr);
	for (auto& it : arr)
	{
		const char* p = strstr(it.c_str(), key);
		if (p == NULL)
			continue;
		int plen = (int) strlen(p);
		if (keylen >= plen) /** 只有一个key长? */
			return false;
		for (int i = 0; i < plen; ++i)
		{
			if (p[keylen + i] == ' ') /** 空格. */
				continue;
			swk->assign(p + keylen + i, plen - keylen - i);
			return true;
		}
	}
	return false;
}

/** 发送握手响应. */
void WebSocket::sendHandShake(ActorNet* an, string* swk)
{
	static const string k = string("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
	string str = *swk + k;
	uchar sha[0x14];
	Misc::sha1((uchar*) str.data(), str.length(), sha);
	char* dat = Base64::encode(sha, 0x14);
	str.clear();
	SPRINTF_STRING(&str, "HTTP/1.1 101 Switching Protocols\r\nConnection: Upgrade\r\nUpgrade: WebSocket\r\nSec-WebSocket-Accept: %s\r\n\r\n", dat)
	free(dat);
	an->send((uchar*) str.data(), str.length()); /** 发送响应. */
}

WebSocket::~WebSocket()
{

}

