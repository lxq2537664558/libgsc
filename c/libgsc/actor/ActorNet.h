/*
 * ActorNet.h
 *
 *  Created on: Jan 31, 2015
 *      Author: root
 */

#ifndef ACTORNET_H_
#define ACTORNET_H_

#if !defined (__LIBGSCH__) && !defined (LIBGSC)
#error only libgsc.h can be included directly.
#endif

#include "Actor.h"

enum NetProtocol
{
	NP_NONE = 0x00 /** 未知. */, NP_STMP /** STMP. */, NP_WEBSOCKET /**  WebSocket. */, NP_HTTP /** HTTP, 只支持H2N上的HTTP协议. */
};

class ActorNet: public Actor
{
private:
	void sendStmp(uchar* dat, uint len); /** STMP报文出栈. */
	void sendWebSocket(uchar* dat, uint len); /** websocket报文出栈. */
public:
	NetProtocol np; /** 连接上的协议类型. */
	bool wshs; /** websocket-handshake是否完成, 仅在np == NP_WEBSOCKET时有效. */
	bool est; /** 连接是否处在establish状态. */
	int cfd; /** 客户端描述字. */
	int dlen; /** 下面缓冲区中数据的长度. */
	uchar* rbuf; /** 接收缓冲区, 它保留了一个GSC_PDU大小. */
	ullong lts; /** 最后收到消息的时间戳(毫秒), 同时用于lazyclose判断. */
	struct sockaddr_in peer; /** 远端地址描述. */
public:
	ActorNet(ActorType type, const char* name, uint wk);
	void send(uchar* dat, uint len); /** 消息出栈. */
	void close(); /** 强制关闭连接. */
	virtual void evnDis() = 0;
	virtual bool evnMsg(stmp_node* root) = 0;
	virtual ~ActorNet();
};

#endif /* ACTORNET_H_ */
