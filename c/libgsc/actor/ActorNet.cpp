/*
 * ActorNet.cpp
 *
 *  Created on: Jan 31, 2015
 *      Author: root
 */

#include "ActorNet.h"
#include "../net/Lx.h"
#include "../core/Cfg.h"
#include "../core/Gstat.h"

ActorNet::ActorNet(ActorType type, const char* name, uint wk) :
		Actor(type, name, wk)
{
	this->np = NP_NONE;
	this->wshs = false;
	this->est = false;
	this->cfd = 0;
	this->dlen = 0;
	this->rbuf = (uchar*) malloc(Cfg::libgsc_peer_mtu);
	this->lts = 0ULL;
}

/** 向套接字发送一段数据. */
void ActorNet::send(uchar* dat, uint len)
{
	Gstat::incv(LIBGSC_SND_BYTES, len);
	Gstat::inc(LIBGSC_SND_MSGS);
	LOG_RECORD("\n  --> PEER: %s CFD: %d\n%s", Net::sockaddr2str(&this->peer).c_str(), this->cfd, (this->np == NP_WEBSOCKET && !this->wshs /** 是websocket连接, 但还未完成握手流程. */) ? Misc::printhex2str(dat, len).c_str() : stmpdec_print2str(dat, len).c_str())
	if (!this->est)
		return;
	if (this->np == NP_WEBSOCKET && this->wshs) /** websocket握手已完成. */
		this->sendWebSocket(dat, len);
	else
		this->sendStmp(dat, len);
}

/** STMP报文出栈. */
void ActorNet::sendStmp(uchar* dat, uint len)
{
	if (::send(this->cfd, dat, len, MSG_DONTWAIT) == len)
		return;
	if (errno == EAGAIN || errno == EWOULDBLOCK)
	{
		LOG_DEBUG("TCP buffer was full, can not send anymore, we will close this peer: %s, size: %08X", Net::sockaddr2str(&this->peer).c_str(), len)
	} else
	{
		LOG_DEBUG("client socket exception, peer: %s, cfd: %d, size: %08X, errno: %d", Net::sockaddr2str(&this->peer).c_str(), this->cfd, len, errno)
	}
	Lx::getGwk()->removeActorNet(this);
	this->evnDis();
}

/** websocket报文出栈. */
void ActorNet::sendWebSocket(uchar* dat, uint len)
{
	uchar* buf = NULL;
	int size;
	if (len < 0x7E) /** 小帧. */
	{
		size = len + 2;
		buf = (uchar*) malloc(size);
		buf[0] = 0x82;
		buf[1] = len;
		memcpy(buf + 2, dat, len);
	} else if (len < 0x10000) /** 中帧. */
	{
		size = len + 4;
		buf = (uchar*) malloc(size);
		buf[0] = 0x82;
		buf[1] = 0x7E;
		ushort s = htons((ushort) len);
		memcpy(buf + 2, &s, 2);
		memcpy(buf + 4, dat, len);
	} else /** 大帧. */
	{
		size = len + 10;
		buf = (uchar*) malloc(size);
		buf[0] = 0x82;
		buf[1] = 0x7F;
		ullong s = Net::htonll(len);
		memcpy(buf + 2, &s, 8);
		memcpy(buf + 10, dat, len);
	}
	if (::send(this->cfd, buf, size, MSG_DONTWAIT) == size)
	{
		free(buf);
		return;
	}
	free(buf);
	if (errno == EAGAIN || errno == EWOULDBLOCK)
	{
		LOG_DEBUG("TCP buffer was full, can not send anymore, we will close this peer: %s, size: %08X", Net::sockaddr2str(&this->peer).c_str(), size)
	} else
	{
		LOG_DEBUG("client socket exception, peer: %s, cfd: %d, size: %08X, errno: %d", Net::sockaddr2str(&this->peer).c_str(), this->cfd, size, errno)
	}
	Lx::getGwk()->removeActorNet(this);
	this->evnDis();
}

/** 强制关闭连接. */
void ActorNet::close()
{
	if (!this->est)
		return;
	Lx::getGwk()->removeActorNet(this);
	this->evnDis();
}

ActorNet::~ActorNet()
{
	free(this->rbuf);
}

