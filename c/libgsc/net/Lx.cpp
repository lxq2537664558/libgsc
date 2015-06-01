/*
 * Lx.cpp
 *
 *  Created on: 2012-7-30
 *      Author: xuzewen
 */

#include "Lx.h"
#include "../core/Cfg.h"
#include "../core/Gsc.h"
#include "../core/Gstat.h"

int Lx::sfd; /** 服务器套接字句柄. */
uint Lx::seq; /** 连接标识发生器.  */
pthread_key_t Lx::key; /** 工作线程本地存储. */
Gworker* Lx::wks; /** 工作线程. */

Lx::Lx()
{

}

/** 初始化libgsc消息总线. */
bool Lx::init()
{
	::signal(SIGPIPE, SIG_IGN);
	Lx::seq = Misc::randomInt();
	pthread_key_create(&Lx::key, NULL);
	Lx::wks = new Gworker[Cfg::libgsc_worker];
	for (int i = 0; i < Cfg::libgsc_worker; ++i)
	{
		Lx::wks[i].wk = i;
		if (pthread_create(&Lx::wks[i].t, NULL, Lx::svc, (void*) (&Lx::wks[i])) != 0)
		{
			LOG_FAULT("no more thread can be create, libgsc server will be exist...")
			return false;
		}
	}
	Misc::sleep(100);
	return true;
}

/** 开启libgsc服务端口. */
bool Lx::publish(const char* host, int port)
{
	Lx::sfd = Net::tcpListen(host, port);
	if (Lx::sfd < 0)
	{
		LOG_FAULT("libgsc can not listen on: %s:%d, errno: %d", host, port, errno)
		return false;
	}
	Net::setNoBlocking(Lx::sfd);
	/**
	 *
	 * libgsc被设计成只有一个线程负责处理accept,
	 * 并不是说多个线程处理accept有什么不妥, 只是一个就够了.
	 *
	 * */
	struct epoll_event ee = { 0 };
	ee.data.fd = Lx::sfd;
	ee.events = (EPOLLIN | EPOLLERR | EPOLLRDHUP | EPOLLET);
	if (epoll_ctl(Lx::wks[0].efd, EPOLL_CTL_ADD, Lx::sfd, &ee) == -1) /** 添加到第0个工作线程. */
	{
		LOG_FAULT("call epoll_ctl (EPOLL_CTL_ADD) failed, worker-indx: 0, efd: %d, errno: %d", Lx::wks[0].efd, errno)
		return false;
	}
	LOG_INFO("libgsc already listen on %s:%d, sfd: %d, and waiting for connection...", host, port, Lx::sfd)
	return true;
}

/** 工作线程入口. */
void* Lx::svc(void* arg)
{
	int size = Cfg::libgsc_peer_limit / Cfg::libgsc_worker;
	Gworker* gwk = (Gworker*) arg;
	struct epoll_event* evns = (struct epoll_event*) calloc(1, sizeof(struct epoll_event) * size);
	if (!Lx::initPipe(gwk))
	{
		exit(1);
		return NULL;
	}
	LOG_INFO("worker-thread start successfully, index: %02X, epoll-fd: %08X, pipe-read: %08X, pipe-write: %08X", gwk->wk, gwk->efd, gwk->rfd, gwk->wfd)
	int i = 0;
	int count = 0;
	while (1)
	{
		gwk->busy = false;
		count = epoll_wait(gwk->efd, evns, size, -1);
		gwk->busy = true;
		for (i = 0; i < count; ++i)
		{
			if (evns[i].events & EPOLLOUT) /** 写事件. */
			{
				gwk->evnSend(evns + i);
				continue;
			}
			if (!(evns[i].events & EPOLLIN))
			{
				gwk->evnErro(evns + i); /** 错误事件. */
				continue;
			}
			if (evns[i].data.fd == Lx::sfd)
			{
				Lx::evnConn(gwk); /** 连接到来事件. */
				continue;
			}
			gwk->evnRecv(evns + i); /** 消息事件. */
		}
		gwk->doFuture(); /** 处理可能到达的future事件. */
	}
	return NULL;
}

/** 连接到来事件. */
void Lx::evnConn(Gworker* gwk)
{
	struct sockaddr_in peer;
	socklen_t socklen = sizeof(struct sockaddr_in);
	int cfd;
	while (1) /** got the all in-coming connection. */
	{
		cfd = ::accept(Lx::sfd, (struct sockaddr*) &peer, &socklen);
		if (cfd == -1) /** ( errno == EAGAIN || errno == EWOULDBLOCK) || thundering herd */
			break;
		if (cfd >= Cfg::libgsc_peer_limit)
		{
			LOG_FAULT("over the LIBGSC_PEER_LIMIT: %u, we will close this connection, cfd: %u", Cfg::libgsc_peer_limit, cfd)
			Net::close(cfd);
			continue;
		}
		/**
		 *  由于::accept被设计成工作在单线程下, 所以需要对上来的连接进行分配.
		 *  这里采取的策略是, 将cfd对LIBGSC_WORKER进行取模, 以期待尽量的负载均衡.
		 *  因此导致的结果是, 某个描述字总是分配给某个固定的线程, 这样在操作Gworker::ans散列表时更为方便(不会出现同一个描述字(值)出现在不同的ans散列表中).
		 */
		Gworker* g = &Lx::wks[Gsc::hashWk(cfd)];
		N2H* n2h = new N2H(cfd, g->wk, &peer);
		Gstat::inc(LIBGSC_N2H_TOTAL);
		n2h->future([cfd, n2h, g]
		{
			LOG_TRACE("got a connection from: %s, cfd: %d", Net::sockaddr2str(&n2h->peer).c_str(), cfd)
			Lx::setFdAtt(cfd);
			g->addActorNet(n2h);
			g->addCfd4Recv(cfd);
		});
	}
}

/** 初始化工作线程自己的通信句柄. */
bool Lx::initPipe(Gworker* gwk)
{
	sockaddr_in addr;
	memset(&addr, 0, sizeof(sockaddr_in));
	int fd[2] = { 0 };
	if (pipe(fd) != 0)
	{
		LOG_FAULT("create pipe failed, errno: %d", errno)
		return false;
	}
	//
	fcntl(fd[0], F_SETFL, O_NONBLOCK);
	fcntl(fd[1], F_SETFL, O_NONBLOCK);
	//
	gwk->rfd = fd[0]; /** 读端. */
	gwk->wfd = fd[1]; /** 写端. */
	gwk->addCfd4Recv(gwk->rfd);
	pthread_setspecific(Lx::key, gwk);
	return true;
}

/** 返回当前工作线程的索引. */
int Lx::getWk()
{
	Gworker* wk = (Gworker*) pthread_getspecific(Lx::key);
	return wk == NULL ? -1 : wk->wk;
}

/** 返回当前工作线程. */
Gworker* Lx::getGwk()
{
	return (Gworker*) pthread_getspecific(Lx::key);
}

/** 关闭libgsc. */
void Lx::stop()
{
	LOG_OPER("libgsc server shutdown.")
	Net::close(Lx::sfd);
}

/** 设置套接字属性. */
void Lx::setFdAtt(int cfd)
{
	Net::setNoBlocking(cfd);
	Net::setLinger(cfd);
	Net::setSNDBUF(cfd, Cfg::libgsc_peer_sndbuf / 2);
	Net::setRCVBUF(cfd, Cfg::libgsc_peer_rcvbuf / 2);
	Net::setNODELAY(cfd);
}

Lx::~Lx()
{

}

