/*
 * Actor.h
 *
 *  Created on: Jul 31, 2014 9:05:43 AM
 *      Author: xuzewen
 */
#ifndef ACTOR_H_
#define ACTOR_H_

#if !defined (__LIBGSCH__) && !defined (LIBGSC)
#error only libgsc.h can be included directly.
#endif

#include "../ginc.h"

typedef struct
{
	function<void()> cb;
} actor_future;

enum ActorType
{
	/** 进程内Actor, 附着在某个Gworker上. */
	ACTOR_ITC,
	/** 拥有自己独立线程或线程池的actor, 主要用于IO阻塞操作, 如数据库查询. */
	ACTOR_BLOCKING,
	/** 即network to host, 网络到主机的连接. */
	ACTOR_N2H,
	/** 即host to network, 主机到网络的连接. */
	ACTOR_H2N,
	/** 主机到网络的http连接. */
	ACTOR_H2N_HTTP
};

class Actor
{
protected:
	int rc; /** 引用计数. */
	bool rf; /** 删除标记. */
public:
	int wk = -1; /** 工作线程索引, -1时无效. */
	ActorType type; /** actor类型. */
	string name; /** actor名称. */
public:
	Actor(ActorType type, const char* name, int wk = INVALID);
	void future(function<void()> cb); /** future函数可以在任意线程中调用, 但lambda一定是在Actor所在的线程中被执行. */
	void ref(); /** 增加一个引用计数. */
	void unRef(); /** 减少一个引用计数. */
	void del(); /** delete this, 任何actor都应该在自己所在的线程被delete. */
	bool isDel(); /** 是否被设置了删除标记. */
	virtual string toString() = 0;
	virtual ~Actor() = 0;
};

#endif /* ACTOR_H_ */
