/*
 * Gworker.h
 *
 *  Created on: Jan 31, 2015
 *      Author: root
 */

#ifndef GWORKER_H_
#define GWORKER_H_

#if !defined (__LIBGSCH__) && !defined (LIBGSC)
#error only libgsc.h can be included directly.
#endif

#include "../actor/N2H.h"
#include "../actor/H2N.h"
#include "../actor/HttpH2n.h"

class Gworker: public Actor
{
private:
	unordered_map<uint, HttpH2n*> httph2ns; /** 线程上所有的HttpH2n, 用于检查其请求是否超时. */
	list<H2N*> h2ns; /** 线程上所有的H2N, 用于H2N自身检查事务超时和心跳发送 */
	unordered_map<uint, N2H*> lazyCloseN2hs; /** 缓存了要延时关闭的N2H, 从最后收到消息的时间戳开始计时. */
	ullong h2nsCheckInterval; /** 向H2N发送通知的间隔. */
	ullong h2nsLastNot; /** 上次向H2N发送通知的时间. */
	ullong lazyCloseN2hsCheckInterval; /** 检查lazyCloseN2hs超时的间隔时间. */
	ullong lazyCloseN2hsLastCheck; /** 上次检查lazyCloseN2hs的时间. */
	ullong ansCheckInterval; /** 检查ans间隔时间. */
	ullong ansLastCheck; /** 上次检查ans的时间. */
public:
	volatile bool busy; /** 工作线程是否处在忙状态. */
	int efd; /** 工作线程持用的epoll描述字. */
	int rfd; /** 管道读描述字. */
	int wfd; /** 管道写描述字. */
	pthread_t t; /** 线程标识. */
	pthread_mutex_t mutex; /** afs上的锁.*/
	queue<actor_future*> afs; /** 缓存所有待处理的lambda. */
	unordered_map<uint /** socket-fd. */, ActorNet*> ans; /** 工作线程上的所有网络连接. */
private:
	void evnItc(); /** 管道消息. */
	bool evnNet4Length(ActorNet* an); /** 网络消息(length + dat的报文格式)(已废弃). */
	bool evnNet4Stmp(ActorNet* an); /** 网络消息(STMP协议报文格式). */
	bool evnNet4WebSocket(ActorNet* an); /** 网络消息(WebSocket协议报文格式). */
	bool evnNet4Http(ActorNet* an); /** 网络消息(HTTP协议报文格式). */
	void checkN2h(ullong now); /** 检查ans中的N2H连接. */
	void checkH2n(ullong now); /** H2N定时器振荡. */
	void checkLazyCloseN2hs(ullong now); /** 检查要延迟关闭的N2H连接. */
	void checkHttpH2n(ullong now); /** 检查HttpH2n是否超时. */
public:
	Gworker();
	virtual ~Gworker();
public:
	void addActorNet(ActorNet* an); /** 添加一个N2H连接. */
	void evnSend(struct epoll_event* evn); /** 写事件. */
	void evnRecv(struct epoll_event* evn); /** 读事件. */
	void evnErro(struct epoll_event* evn); /** 异常事件. */
	void doFuture(); /** 处理可能到达的future事件. */
	void check(ullong now); /** 定时任务. */
	//
	void push(actor_future* f); /** 前转一个future到当前工作线程. */
	void addCfd4Send(int cfd); /** 将描述字添加到epoll(注册写事件). */
	void modCfd2Recv(int cfd); /** 将描述字修改为注册epoll的读事件. */
	void addCfd4Recv(int cfd); /** 将描述字添加到epoll(注册读事件). */
	void delCfd(int cfd); /** 将描述字从epoll卸下. */
	void removeActorNet(ActorNet* an); /** 从epoll处卸下一个ActorNet. */
	void addLazyCloseN2h(N2H* n2h); /** 添加一个要延迟关闭的N2H. */
	string toString();
};

#endif /* GWORKER_H_ */
