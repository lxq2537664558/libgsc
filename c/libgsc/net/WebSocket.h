/*
 * WebSocket.h
 *
 *  Created on: Mar 27, 2015 1:55:12 PM
 *      Author: xuzewen
 */

#ifndef WEBSOCKET_H_
#define WEBSOCKET_H_

#include "../actor/ActorNet.h"

#define WS_FRAME_CONTINUATION								0x00
#define WS_FRAME_TEXT										0x01
#define WS_FRAME_BINARY										0x02
#define WS_FRAME_CLOSE										0x08
#define WS_FRAME_PING										0x09
#define WS_FRAME_PONG										0x0A

class WebSocket
{
private:
	WebSocket();
	virtual ~WebSocket();
	static bool parseSwk(const char* dat, string* swk); /** 从握手协议中取出Sec-WebSocket-Key. */
	static void sendHandShake(ActorNet* an, string* swk); /** 发送握手响应. */
	static bool ws_frame_binary(uchar* dat, int len, ActorNet* an); /** 根据不同的操作码解析报文. */
public:
	static bool checkHandShake(ActorNet* an); /** 检查握手协议. */
	static bool unpack(ActorNet* an); /** 解析报文帧. */
};

#endif /* WEBSOCKET_H_ */
