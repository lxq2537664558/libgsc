/*
 * Gn2htrans.cpp
 *
 *  Created on: Jan 31, 2015
 *      Author: root
 */

#include "Gn2htrans.h"
#include "Gsc.h"
#include "Cfg.h"

Gn2htrans::Gn2htrans(N2H* n2h, ushort cmd, uint tid, Message* begin, string* extbegin)
{
	this->lzmsec = 0;
	this->cmd = cmd;
	this->tid = tid;
	this->begin = begin;
	this->extbegin = (extbegin == NULL ? NULL : new string(*extbegin));
	this->ret = RET_SUCCESS;
	this->endx = NULL;
	this->extend = NULL;
	n2h->ref();
	this->n2h = n2h;
}

void Gn2htrans::end(ushort ret, Message* end, string* extend)
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
	this->n2h->future([this]
	{
		if(this->lzmsec > 0)
		{
			this->n2h->lts += this->lzmsec;
			this->n2h->lazyClose(); /** 延迟关闭. */
		}
		if (this->n2h->gusr != NULL) /** 有应用层关联. */
		{
			this->n2h->gusr->future([this]
					{
						this->n2h->gusr->sendEnd0(this);
					});
		} else
		{
			this->n2h->sendEnd(this);
		}
	});
}

void Gn2htrans::end(Message* end, string* extend)
{
	this->end(RET_SUCCESS, end, extend);
}

void Gn2htrans::end(ushort ret, string* extend)
{
	this->end(ret, NULL, extend);
}

void Gn2htrans::success(string* extend)
{
	this->end(RET_SUCCESS, NULL, extend);
}

void Gn2htrans::failure()
{
	this->end(RET_FAILURE, NULL, NULL);
}

void Gn2htrans::finish()
{
	if (Gsc::ec)
		Gsc::ec->gn2hTrans(this);
	delete this;
}

void Gn2htrans::successLazyClose(int sec)
{
	this->lzmsec = (sec == 0 ? Cfg::libgsc_n2h_lazy_close /** 默认值. */: sec * 1000);
	this->end(RET_SUCCESS, NULL, extend);
}

void Gn2htrans::endLazyClose(Message* end, int sec)
{
	this->lzmsec = (sec == 0 ? Cfg::libgsc_n2h_lazy_close /** 默认值. */: sec * 1000);
	this->end(RET_SUCCESS, end, NULL);
}

void Gn2htrans::endLazyClose(ushort ret, int sec)
{
	this->lzmsec = (sec == 0 ? Cfg::libgsc_n2h_lazy_close /** 默认值. */: sec * 1000);
	this->end(ret, NULL, NULL);
}

Gn2htrans::~Gn2htrans()
{
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

