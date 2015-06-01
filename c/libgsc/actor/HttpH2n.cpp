/*
 * HttpH2n.cpp
 *
 *  Created on: Apr 2, 2015 4:54:21 PM
 *      Author: xuzewen
 */

#include "HttpH2n.h"
#include "../core/Cfg.h"
#include "../core/Gsc.h"
#include "../net/Lx.h"

HttpH2n::HttpH2n(const char* host, int port, int sec /** 从连接到响应的时间限制(秒). */) :
		ActorNet(ACTOR_H2N_HTTP, "HttpH2n", Gsc::rrWk() % Cfg::libgsc_worker /** 轮询工作线程. */)
{
	this->np = NP_HTTP;
	this->host.assign(host);
	this->tts = DateMisc::getMsec() + (sec * 1000);
	this->send = false;
	Net::setAddr(host, port, &this->peer);
}

/** 发起一个HTTP请求. */
void HttpH2n::future(const char* req /** 包括头和正文. */, function<void(string* rsp)> rsp, function<void()> fail, function<void()> tm)
{
	this->rspCb = rsp;
	this->failCb = fail;
	this->tmCb = tm;
	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock < 1 || sock >= Cfg::libgsc_peer_limit)
	{
		LOG_FAULT("can not create socket any more, sock: %d", sock)
		if (sock > 0)
			Net::close(sock);
		Actor::future([this]
		{
			this->failCb();
			delete this;
		});
		return;
	}
	this->cfd = sock;
	this->req.assign(req);
	Actor::future([this]
	{
		Lx::setFdAtt(this->cfd);
		if (::connect(this->cfd, (struct sockaddr*) &this->peer, sizeof(struct sockaddr)) == 0) /** 连接成功. */
		{
			if (this->sendHttpRequest()) /** 发送请求. */
			{
				this->est = true;
				Lx::getGwk()->addActorNet(this); /** 等待读事件. */
				Lx::getGwk()->addCfd4Recv(this->cfd);
			} else
			{
				this->failCb();
				delete this;
			}
			return;
		}
		if (errno != EINPROGRESS) /** 连接失败. */
		{
			Gworker* gwk =Lx::getGwk();
			gwk->ans.erase(this->cfd);
			gwk->delCfd(this->cfd);
			Net::close(this->cfd);
			this->failCb();
			delete this;
			return;
		}
		Lx::getGwk()->addActorNet(this); /** 等待读事件. */
		Lx::getGwk()->addCfd4Send(this->cfd);
	});
}

/** 发送一个HTTP请求. */
bool HttpH2n::sendHttpRequest()
{
	this->send = true;
	if (::send(this->cfd, this->req.data(), this->req.length(), MSG_DONTWAIT) == (int) this->req.length())
		return true;
	if (errno == EAGAIN || errno == EWOULDBLOCK)
	{
		LOG_DEBUG("TCP buffer was full, can not send anymore, we will close this peer: %s, size: %08X", Net::sockaddr2str(&this->peer).c_str(), (uint)this->req.length())
	} else
	{
		LOG_DEBUG("client socket exception, peer: %s, cfd: %d, size: %08X, errno: %d", Net::sockaddr2str(&this->peer).c_str(), this->cfd, (uint)this->req.length(), errno)
	}
	return false;
}

/** 写事件. */
void HttpH2n::evnSend()
{
	int err;
	socklen_t len = sizeof(int);
	if (getsockopt(this->cfd, SOL_SOCKET, SO_ERROR, (char* /** for windows. */) &err, &len) < 0 || err)
	{
		loop: //
		Gworker* gwk = Lx::getGwk();
		gwk->ans.erase(this->cfd);
		gwk->delCfd(this->cfd);
		Net::close(this->cfd);
		this->del(); /** 由于Gworker引用了一次, 所在这里不直接delete this. */
		this->failCb();
		return;
	}
	this->est = true;
	if (!this->sendHttpRequest()) /** 发送请求失败. */
		goto loop;
	Lx::getGwk()->modCfd2Recv(this->cfd);
}

/** 连接已失去. */
void HttpH2n::evnDis()
{
	if (!this->send)
		this->failCb();
	this->del();
}

/** HTTP响应. */
bool HttpH2n::evnMsg4Http(uchar* dat, uint len)
{
	string rsp((char*) dat, len);
	this->rspCb(&rsp); /** 交由上层自己处理. */
	return false; /** 总是关闭(这里简单处理, 假设响应总是在一个报文中). */
}

string HttpH2n::toString()
{
	string str;
	SPRINTF_STRING(&str, "HTTP-H2N(%s), rc: %d, rf: %s, wk: %d, cfd: %d, dlen: %d, host: %s, req: %s", //
			this->name.c_str(), this->rc, this->rf ? "true" : "false", this->wk, //
			this->cfd, this->dlen, this->host.c_str(), this->req.c_str())
	return str;
}

bool HttpH2n::evnMsg4Stmp(stmp_node* root)
{
	return false;
}

HttpH2n::~HttpH2n()
{

}

