/*
 * Cb.cpp
 *
 *  Created on: Jan 31, 2015
 *      Author: root
 */

#include "Cb.h"

Cb::Cb(ushort cmd, void* cb, const Descriptor* begin, const Descriptor* end, const Descriptor* uni, bool gusr)
{
	this->gusr = gusr;
	this->cmd = cmd;
	this->cb = cb;
	this->begin = (Descriptor*) begin;
	this->end = (Descriptor*) end;
	this->uni = (Descriptor*) uni;
}

Cb::~Cb()
{

}

