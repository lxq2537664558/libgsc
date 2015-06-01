/*
 * Gstat.cpp
 *
 *  Created on: Mar 11, 2015 6:34:19 PM
 *      Author: xuzewen
 */

#include "Gstat.h"

static ullong items[STAT_END] = { 0 };

Gstat::Gstat()
{

}

void Gstat::inc(uint item)
{
	__sync_fetch_and_add(items + (item % STAT_END), 1);
}

/** 自增v. */
void Gstat::incv(uint item, ullong v)
{
	__sync_fetch_and_add(items + (item % STAT_END), v);
}

/** 获得某项的值. */
ullong Gstat::get(uint item)
{
	return items[item % STAT_END];
}

Gstat::~Gstat()
{

}

