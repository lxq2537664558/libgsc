/*
 * Gn2h.cpp
 *
 *  Created on: Mar 4, 2015 2:47:14 PM
 *      Author: xuzewen
 */

#include "Gn2h.h"
#include "../actor/N2H.h"
#include "../actor/Gusr.h"
#include "../core/Gsc.h"

Gn2h::Gn2h(Gusr* gusr, N2H* n2h, ullong sid, ushort cmd, uint tid, Message* begin, string* extbegin)
{
	this->gusr = gusr;
	this->gusr->ref();
	this->sid = sid;
	this->cmd = cmd;
	this->tid = tid;
	this->begin = begin;
	this->extbegin = (extbegin == NULL ? NULL : new string(*extbegin));
	this->ret = RET_SUCCESS;
	this->endx = NULL;
	this->extend = NULL;
	this->n2h = n2h;
}

void Gn2h::end(ushort ret, Message* end, string* extend)
{
	this->ret = ret;
	this->endx = end;
	this->extend = extend == NULL ? NULL : new string(*extend);
	if (ret == RET_FAILURE) /** 失败, 关闭连接. */
	{
		this->n2h->future([this]
		{
			this->n2h->close();
			this->finish();
		});
		return;
	}
	this->gusr->future([this]
	{
		this->gusr->sendEnd1(this);
	});
}

void Gn2h::success(string* extend)
{
	this->end(RET_SUCCESS, NULL, extend);
}

void Gn2h::end(Message* end, string* extend)
{
	this->end(RET_SUCCESS, end, extend);
}

void Gn2h::end(ushort ret, string* extend)
{
	this->end(ret, NULL, extend);
}

void Gn2h::failure()
{
	this->end(RET_FAILURE, NULL, NULL);
}

/** 事务结束. */
void Gn2h::finish()
{
	if (Gsc::ec)
		Gsc::ec->gn2h(this);
	delete this;
}

Gn2h::~Gn2h()
{
	this->gusr->unRef(); /** Gusr在事务构造时被引用1次, 在事务结束时释放引用. */
	this->n2h->unRef(); /** N2H在事务构造时被引用1次, 在事务结束时释放引用. */
	if (this->begin != NULL)
		delete this->begin;
	if (this->extbegin != NULL)
		delete this->extbegin;
	if (this->endx != NULL)
		delete this->endx;
	if (this->extend != NULL)
		delete this->extend;
}

