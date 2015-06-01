/*
 * Gh2ntrans.cpp
 *
 *  Created on: Feb 2, 2015 3:15:54 PM
 *      Author: xuzewen
 */

#include "Gh2ntrans.h"
#include "Gsc.h"

Gh2ntrans::Gh2ntrans(H2N* h2n, ushort cmd, Message* begin, function<void(ushort ret, Message* end, string* ext)>& end, function<void()>& tm, string* extbegin)
{
	this->h2n = h2n;
	this->cmd = cmd;
	this->ret = RET_SUCCESS;
	this->tid = 0;
	this->endCb = end;
	this->tmCb = tm;
	this->tm = false;
	this->begin = begin;
	this->extbegin = extbegin == NULL ? NULL : new string(*extbegin);
	this->end = NULL;
	this->extend = NULL;
	this->gts = DateMisc::getMsec();
}

Gh2ntrans::~Gh2ntrans()
{
	this->h2n->unRef();
	if (this->begin != NULL)
		delete this->begin;
	if (this->extbegin != NULL)
		delete this->extbegin;
	if (this->end != NULL)
		delete this->end;
	if (this->extend != NULL)
		delete this->extend;
}

