/*
 * ActorGusrDb.h
 *
 *  Created on: Mar 4, 2015 4:02:48 PM
 *      Author: xuzewen
 */

#ifndef ACTORGUSRDB_H_
#define ACTORGUSRDB_H_

#include "ActorBlocking.h"
#include "Gusr.h"

/**
 *
 * Gusr的数据库查询接口, 适用于短连接.
 *
 */
class ActorGusrDb: public ActorBlocking
{
public:
	ActorGusrDb(const char* name, int pool);
	virtual Gusr* refGusr(ullong uid, ullong* sid /** 返回值, Gusr上的会话id. */) = 0; /** 引用Gusr. */
	virtual void unRefGusr(Gusr* gusr) = 0; /** 释放Gusr. */
	virtual ~ActorGusrDb();
};

#endif /* ACTORGUSRDB_H_ */
