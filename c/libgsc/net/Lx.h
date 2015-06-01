/*
 * Lx.h
 *
 *  Created on: 2012-7-30
 *      Author: xuzewen
 */

#ifndef LX_H_
#define LX_H_

#if !defined (__LIBGSCH__) && !defined (LIBGSC)
#error only libgsc.h can be included directly.
#endif

#include "../actor/ActorNet.h"
#include "Gworker.h"

/**
 *
 * 基于epoll的并发服务器实现.
 *
 */
class Lx
{
public:
	static int sfd; /** 服务器套接字句柄. */
	static uint seq; /** 连接标识发生器.  */
	static pthread_key_t key; /** 工作线程本地存储. */
	static Gworker* wks; /** 工作线程. */
private:
	Lx();
	virtual ~Lx();
	static void* svc(void* arg); /** 工作线程入口. */
	static bool initPipe(Gworker* gwk); /** 初始化工作线程自己的通信句柄. */
public:
	static bool init();/** 初始化libgsc消息总线. */
	static bool publish(const char* host, int port); /** 开启libgsc服务端口. */
	static void stop(); /** 关闭libgsc. */
	static void evnConn(Gworker* gwk); /** 连接到来事件. */
	static int getWk(); /** 返回当前工作线程的索引. */
	static Gworker* getGwk(); /** 返回当前工作线程. */
	static void setFdAtt(int cfd); /** 设置套接字属性. */
};

#endif /* LX_H_ */
