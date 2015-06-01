/*
 * Gworker.cpp
 *
 *  Created on: Jan 31, 2015
 *      Author: root
 */

#include "Gworker.h"
#include "Lx.h"
#include "WebSocket.h"
#include "../core/Gstat.h"
#include "../core/Cfg.h"
#include "../core/Gsc.h"
#include "../actor/N2H.h"
#include "../actor/HttpH2n.h"

Gworker::Gworker() :
		Actor(type, "Gworker", 0 /** 由Lx赋值. */)
{
	this->wk = 0;
	this->efd = epoll_create(Cfg::libgsc_peer_limit / Cfg::libgsc_worker);
	this->rfd = 0;
	this->wfd = 0;
	this->busy = false;
	this->t = 0;
	pthread_mutex_init(&this->mutex, NULL);
	//
	this->h2nsCheckInterval = 1 * 1000;
	this->h2nsLastNot = 0;
	//
	this->lazyCloseN2hsCheckInterval = 1 * 1000;
	this->lazyCloseN2hsLastCheck = 0;
	//
	this->ansCheckInterval = 1 * 1000;
	this->ansLastCheck = 0;
}

/** 写事件. */
void Gworker::evnSend(struct epoll_event* evn)
{
	auto it = this->ans.find(evn->data.fd);
	if (it == this->ans.end()) /** 不应该找不到. */
	{
		LOG_FAULT("it`s a bug.")
		return;
	}
	ActorNet* an = it->second;
	if (an->np != NP_HTTP)
	{
		LOG_FAULT("it`s a bug, should be http-client connection: %s", an->toString().c_str())
		return;
	}
	an->ref(); /** 如果消息处理失败, an可能已经被删除, 为防止这种情况, 这里引用一次, 并在下面释放. */
	((HttpH2n*) an)->evnSend();
	an->unRef();
}

/** 读事件. */
void Gworker::evnRecv(struct epoll_event* evn)
{
	if (evn->data.fd == this->rfd)
	{
		this->evnItc();
		return;
	}
	auto it = this->ans.find(evn->data.fd);
	if (it == this->ans.end())
	{
		LOG_FAULT("it`s a bug.")
		return;
	}
	ActorNet* an = it->second;
	if (an->type == ACTOR_N2H)
	{
		if (((N2H*) an)->lz)
			return;
	}
	an->ref(); /** 如果消息处理失败, an可能已经被删除, 为防止这种情况, 这里引用一次, 并在下面释放. */
	int count;
	bool flag = true;
	while (flag)
	{
		count = ::recv(evn->data.fd, an->rbuf + an->dlen, Cfg::libgsc_peer_mtu - an->dlen, MSG_DONTWAIT);
		if (count < 1)
		{
			if ((count == -1 && errno != EAGAIN) || count == 0)
				flag = false;
			break;
		}
		an->dlen += count;
		Gstat::incv(LIBGSC_RCV_BYTES, count);
		switch (an->np)
		{
		case NP_NONE:
			if (an->rbuf[0] >= STMP_TAG_TRANS_BEGIN && an->rbuf[0] <= STMP_TAG_TRANS_UNI) /** STMP-protocol. */
			{
				an->np = NP_STMP;
				flag = this->evnNet4Stmp(an);
			} else if (an->rbuf[0] == 'G' /** GET. */)
			{
				an->np = NP_WEBSOCKET;
				flag = this->evnNet4WebSocket(an);
			}
			break;
		case NP_STMP:
			flag = this->evnNet4Stmp(an);
			break;
		case NP_WEBSOCKET:
			flag = this->evnNet4WebSocket(an);
			break;
		case NP_HTTP:
			flag = this->evnNet4Http(an);
			break;
		default:
			LOG_FAULT("it`s a bug.")
			break;
		}
	}
	if (!flag)
	{
		LOG_DEBUG("have a client disconnected: %s, errno: %d", an->toString().c_str(), errno)
		if (an->est)
		{
			this->removeActorNet(an);
			an->evnDis();
		} else
			; /** 如果连接已经不在, 则一定是已经调用过evnDis和上面的removeActorNet. */
	}
	an->unRef();
}

/** 网络消息(STMP协议报文格式). */
bool Gworker::evnNet4Stmp(ActorNet* an)
{
	uchar* dat = an->rbuf;
	int len = an->dlen;
	for (;;)
	{
		if (len < 3) /** 至少要有一个tlv. */
			break;
		if (dat[1] == 0xFF)
		{
			LOG_DEBUG("unsupported over 64K PDU.")
			return false;
		}
		int l = (dat[1] == 0xFE ? 3 : 1); /** 获得len字段的长度. */
		if (len < 1 + l) /** 不够一个tag + len, 如65 30, 或65 FE 01 FF. */
			break;
		int size;
		if (dat[1] < 0xFE)
			size = 1/** tag*/+ 1 /** len字段本身 */+ dat[1] /** val */;
		else /** 有两字节表示长度. */
		{
			ushort s;
			memcpy(&s, dat + 2, 2);
			s = ntohs(s); /** 取出长度. */
			size = 1 + l + s;
		}
		if (size > Cfg::libgsc_peer_mtu)
		{
			LOG_DEBUG("packet format error, we will close this connection, peer: %s, size: %08X", Net::sockaddr2str(&an->peer).c_str(), size)
			return false;
		}
		if (len < size) /** 还未到齐. */
			break;
		//
		stmp_node root;
		if (stmpdec_unpack(dat, size, &root) != 0)
		{
			LOG_DEBUG("STMP protocol error.")
			return false;
		}
		LOG_RECORD("\n  <-- PEER: %s CFD: %d\n%s", Net::sockaddr2str(&an->peer).c_str(), an->cfd, stmpdec_printnode2str(&root).c_str())
		Gstat::inc(LIBGSC_RCV_MSGS);
		an->lts = DateMisc::getMsec(); /** 更新最后收到消息的时间戳. */
		if (!an->evnMsg(&root))
		{
			stmpdec_free(&root);
			return false;
		}
		stmpdec_free(&root);
		//
		dat += size;
		len -= size;
	}
	//
	if (len != an->dlen)
	{
		for (int i = 0; i < len; ++i)
			an->rbuf[i] = dat[i];
		an->dlen = len;
	}
	return true;
}

/** 网络消息(WebSocket协议报文格式). */
bool Gworker::evnNet4WebSocket(ActorNet* an)
{
	if (!an->wshs) /** 还未完成握手. */
	{
		if (!WebSocket::checkHandShake(an)) /** 检查握手报文是否完整. */
			return true;
		an->wshs = true;
		if (an->dlen == 0) /** 刚好一个握手报文. */
			return true;
	}
	return WebSocket::unpack(an);
}

/** 网络消息(HTTP协议报文格式). */
bool Gworker::evnNet4Http(ActorNet* an)
{
	return ((HttpH2n*) an)->evnMsg4Http(an->rbuf, an->dlen);
}

/** 管道消息. */
void Gworker::evnItc()
{
	static uchar buf[0x2000];
	while (::read(this->rfd, buf, 0x2000) > 0)
		;/** 一次读尽. */
}

/** 异常事件. */
void Gworker::evnErro(struct epoll_event* evn)
{
	auto it = this->ans.find(evn->data.fd);
	if (it == this->ans.end())
	{
		LOG_WARN("can not found ActorNet for cfd: %d", evn->data.fd)
		return;
	}
	ActorNet* an = it->second;
	LOG_TRACE("have a client disconnected: %s, errno: %d", an->toString().c_str(), errno)
	this->removeActorNet(an);
}

/** 处理可能到达的future事件. */
void Gworker::doFuture()
{
	while (1)
	{
		actor_future* f = NULL;
		this->busy = false;
		pthread_mutex_lock(&this->mutex);
		if (this->afs.empty())
		{
			pthread_mutex_unlock(&this->mutex);
			return;
		}
		f = this->afs.front();
		this->afs.pop();
		pthread_mutex_unlock(&this->mutex);
		this->busy = true;
		f->cb();
		delete f;
	}
}

/** 前转一个future到当前工作线程. */
void Gworker::push(actor_future* f)
{
	pthread_mutex_lock(&this->mutex);
	this->afs.push(f);
	pthread_mutex_unlock(&this->mutex);
	//
	static uchar b[1];
	if (!this->busy)
		::write(this->wfd, b, 1);
}

/** 将描述字添加到epoll(注册写事件). */
void Gworker::addCfd4Send(int cfd)
{
	struct epoll_event ce = { 0 };
	ce.data.fd = cfd;
	ce.events = EPOLLOUT | EPOLLERR | EPOLLRDHUP | EPOLLET;
	if (epoll_ctl(this->efd, EPOLL_CTL_ADD, cfd, &ce) == -1)
		LOG_FAULT("add FD to epoll failed, cfd: %d, errno: %d", cfd, errno)
}

/** 将描述字修改为注册epoll的读事件. */
void Gworker::modCfd2Recv(int cfd)
{
	struct epoll_event ce = { 0 };
	ce.data.fd = cfd;
	ce.events = EPOLLIN | EPOLLERR | EPOLLRDHUP | EPOLLET;
	if (epoll_ctl(this->efd, EPOLL_CTL_MOD, cfd, &ce) == -1)
		LOG_FAULT("mod FD to epoll failed, cfd: %d, errno: %d", cfd, errno)
}

/** 将描述字添加到epoll(注册读事件). */
void Gworker::addCfd4Recv(int cfd)
{
	struct epoll_event ce = { 0 };
	ce.data.fd = cfd;
	ce.events = EPOLLIN | EPOLLERR | EPOLLRDHUP | EPOLLET;
	if (epoll_ctl(this->efd, EPOLL_CTL_ADD, cfd, &ce) == -1)
		LOG_FAULT("add FD to epoll failed, cfd: %d, errno: %d", cfd, errno)
}

/** 将描述字从epoll卸下. */
void Gworker::delCfd(int cfd)
{
	struct epoll_event ce = { 0 };
	ce.data.fd = cfd;
	ce.events = EPOLLIN | EPOLLERR | EPOLLRDHUP | EPOLLET;
	if (epoll_ctl(this->efd, EPOLL_CTL_DEL, cfd, &ce) == -1)
		LOG_FAULT("remove FD from epoll failed, cfd: %d, errno: %d", cfd, errno)
}

/** 从epoll处卸下一个ActorNet. */
void Gworker::removeActorNet(ActorNet* an)
{
	if (an->type == ACTOR_N2H)
		this->lazyCloseN2hs.erase(an->cfd);
	else if (an->type == ACTOR_H2N_HTTP)
		this->httph2ns.erase(an->cfd);
	//
	this->ans.erase(an->cfd);
	this->delCfd(an->cfd);
	Net::close(an->cfd);
	an->est = false;
}

/** 添加一个ActorNet连接. */
void Gworker::addActorNet(ActorNet* an)
{
	auto it = this->ans.find(an->cfd);
	if (it != this->ans.end()) /** 描述字对应的ActorNet应该早已被删除. */
	{
		LOG_FAULT("it`s a bug, an: %s", an->toString().c_str())
		return;
	}
	if (an->type == ACTOR_H2N)
		this->h2ns.push_back((H2N*) an);
	else if (an->type == ACTOR_H2N_HTTP)
		this->httph2ns[an->cfd] = ((HttpH2n*) an);
	this->ans[an->cfd] = an;
}

/** 添加一个要延迟关闭的N2H. */
void Gworker::addLazyCloseN2h(N2H* n2h)
{
	this->lazyCloseN2hs[n2h->cfd] = n2h;
}

/** 定时任务. */
void Gworker::check(ullong now)
{
	this->checkN2h(now);
	this->checkH2n(now);
	this->checkLazyCloseN2hs(now);
	this->checkHttpH2n(now);
}

/** 检查ans中的N2H连接. */
void Gworker::checkN2h(ullong now)
{
	if (now - this->ansLastCheck < this->ansCheckInterval)
		return;
	this->ansLastCheck = now;
	for (auto it = this->ans.begin(); it != this->ans.end();)
	{
		if (it->second->type == ACTOR_H2N || it->second->type == ACTOR_H2N_HTTP) /** 不关心对外的连接. */
		{
			++it;
			continue;
		}
		N2H* n2h = (N2H*) it->second;
		if (n2h->lz) /** 不关心要延迟关闭的连接. */
		{
			++it;
			continue;
		}
		/** -------------------------------- */
		/**                                  */
		/** 僵尸连接 */
		/**                                  */
		/** -------------------------------- */
		if (n2h->lts == 0) /** 新连接. */
		{
			if (now - n2h->gts >= Cfg::libgsc_n2h_zombie)
			{
				LOG_DEBUG("got a zombie n2h connection: %s", n2h->toString().c_str())
				this->ans.erase(it++);
				this->lazyCloseN2hs.erase(n2h->cfd);
				this->delCfd(n2h->cfd);
				Net::close(n2h->cfd);
				n2h->evnDis();
			} else
				++it;
			continue;
		}
		/** -------------------------------- */
		/**                                  */
		/** 超出心跳时间 */
		/**                                  */
		/** -------------------------------- */
		if (now - n2h->lts >= Cfg::libgsc_peer_heartbeat)
		{
			LOG_DEBUG("have a n2h connection lost heart-beat: %s", n2h->toString().c_str())
			this->ans.erase(it++);
			this->lazyCloseN2hs.erase(n2h->cfd); /** 可能存在, 可能不存在. */
			this->delCfd(n2h->cfd);
			Net::close(n2h->cfd);
			n2h->evnDis();
			continue;
		}
		++it;
	}
}

/** H2N定时器振荡. */
void Gworker::checkH2n(ullong now)
{
	if (now - this->h2nsLastNot < this->h2nsCheckInterval)
		return;
	this->h2nsLastNot = now;
	for (auto& it : this->h2ns)
		it->check(now);
}

/** 检查要延迟关闭的N2H连接. */
void Gworker::checkLazyCloseN2hs(ullong now)
{
	if (now - this->lazyCloseN2hsLastCheck < this->lazyCloseN2hsCheckInterval)
		return;
	this->lazyCloseN2hsLastCheck = now;
	for (auto it = this->lazyCloseN2hs.begin(); it != this->lazyCloseN2hs.end();)
	{
		if (now >= it->second->lts)
		{
			LOG_DEBUG("N2H lazy-close, elap: %llumsec, actor: %s", now - it->second->lts, it->second->toString().c_str())
			this->ans.erase(it->second->cfd);
			Net::close(it->second->cfd);
			it->second->evnDis();
			this->lazyCloseN2hs.erase(it++);
		} else
			++it;
	}
}

/** 检查HttpH2n是否超时. */
void Gworker::checkHttpH2n(ullong now)
{
	for (auto it = this->httph2ns.begin(); it != this->httph2ns.end();)
	{
		HttpH2n* http = it->second;
		if (now >= http->tts)
		{
			LOG_DEBUG("have a http-n2h connection timeout: %s", http->toString().c_str())
			this->httph2ns.erase(it++);
			http->tmCb();
			this->delCfd(http->cfd);
			Net::close(http->cfd);
			http->est = false;
			http->del();
		} else
			++it;
	}
}

string Gworker::toString()
{
	string str;
	SPRINTF_STRING(&str, "ITC(%s), wk:%d, busy:%s, efd:%d, rfd:%d, wfd:%d, afs: %lu, ans:%lu, h2ns:%lu, lazyCloseN2hs:%lu", //
			this->name.c_str(), this->wk, this->busy ? "true" : "false", this->efd, this->rfd, this->wfd, //
			this->afs.size(), this->ans.size(), this->h2ns.size(), this->lazyCloseN2hs.size())
	return str;
}

/** 网络消息(length + dat的报文格式)(已废弃). */
bool Gworker::evnNet4Length(ActorNet* an)
{
	uchar* dat = an->rbuf;
	int len = an->dlen;
	for (;;)
	{
		if (len < 4)
			break;
		int size = 0x00;
		memcpy(&size, dat, 4);
		size = ntohl(size) & 0x0FFFFFFF;
		if (size < 5 || size > Cfg::libgsc_peer_mtu)
		{
			LOG_DEBUG("packet format error, we will close this connection, peer: %s, size: %08X", Net::sockaddr2str(&an->peer).c_str(), size)
			return false;
		}
		if (len < size)
			break;
		//
		LOG_RECORD("\n  <-- peer: %s    cfd: %d\n%s", Net::sockaddr2str(&an->peer).c_str(), an->cfd, Misc::printhex2str(dat, size).c_str())
		Gstat::inc(LIBGSC_RCV_MSGS);
		an->lts = DateMisc::getMsec(); /** 更新最后收到消息的时间戳. */
		//if (!an->evnMsg(dat, size))
		//		return false;
		//
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

Gworker::~Gworker()
{

}

