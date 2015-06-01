/*
 * DateMisc.h
 *
 *  Created on: 2010-12-1
 *      Author: xuzewen
 */

#ifndef DATEMISC_H_
#define DATEMISC_H_

#if !defined (__LIBMISC_H__) && !defined (LIBMISC)
#error only libmisc.h can be included directly.
#endif

#include "../macro.h"

class DateMisc
{
private:
	DateMisc();
	virtual ~DateMisc();
public:
	static time_t sec; /** 秒. */
	static time_t minute; /** 分. */
	static time_t hour; /** 小时. */
	static time_t day; /** 天. */
public:
	static void gettimeofday(struct timeval* tv); /** gettimeofday. */
	static ullong getTs(); /** 返回当前时间戳, 微秒. */
	static ullong getMsec(); /** 返回当前时间戳, 毫秒. */
	static ullong getMsec(struct timeval* ts); /** 返回指定时间的毫秒数, 微秒被抛弃. */
	static int getElap(struct timeval* now, struct timeval* old); /** 返回两个时间点相差的微秒数, now应该大于old. */
	static ullong getDiDa(); /** 返回系统时钟滴答声, 精确到微秒. */
	static ullong getDiDaMsec(); /** 返回系统时钟滴答声, 精确到毫秒. */
	static char* yyyymm(); /** 当前时间: yyyymm. */
	static char* yyyymmdd(); /** 当前时间: yyyy-mm-dd. */
	static string yyyymmdd(time_t ts); /** 指定时间: yyyy-mm-dd. */
	static char* yyyymmddhh(); /** 当前时间: yyyy-mm-dd hh. */
	static char* yyyymmddhhmiss(); /** 当前时间: yyyy-mm-dd hh:mi:ss. */
	static char* yyyymmddhhmissShort(); /** 当前时间: yyyymmddhhmiss. */
	static void yyyymmddhhmiss(time_t ts, char* str); /** 指定时间: yyyy-mm-dd hh:mi:ss.*/
	static string yyyymmddhhmiss(time_t ts); /** 指定时间: yyyy-mm-dd hh:mi:ss.*/
	static void yyyymmddhhmissShort(time_t ts, char* str); /** 指定时间: yyyymmddhhmiss.*/
	static string yyyymmddhhmissShort(time_t ts); /** 指定时间: yyyymmddhhmiss.*/
	static char* hhmiss(); /** 当前时间: hh:mi:ss. */
	static char* hhmissms(); /** 当前时间: hh:mi:ss.ms. */
	static char* yyyymmLast(); /** 上个月. */
	static bool isSameDay(time_t f, time_t s); /** 比较两个时间是不是在同一天. */
	static time_t parseYYYY_MM_DD_HHMISS(char* str); /** 将yyyy-mm-dd hh:mi:ss(如2013-11-29 23:59:59)转换成绝对时间秒. */
	static int getYear(time_t ts); /** 返回指定绝对时间的年部分. */
	static int getMonth(time_t ts); /** 返回指定绝对时间的月部分. */
	static int getDay(time_t ts); /** 返回指定绝对时间的月部分. */
	static int getHour(time_t ts); /** 返回指定绝对时间的小时部分. */
	static int getMinu(time_t ts); /** 返回指定绝对时间的分钟部分. */
	static int getSec(time_t ts); /** 返回指定绝对时间的秒部分. */
	static time_t makeTime(int year, int month, int day, int hour, int minu, int sec); /** 构造一个指定的时间. */
	/**
	 *
	 * 以ts为界, 将ts调至指定的小时.
	 * 如果ts此时为2013-10-17 10:13:12.
	 * 当hour为12时, 将返回2013-10-17 12:00:00
	 * 当hour为9时, 将返回2013-10-18 09:00:00, 也就是跳转到下一天的09点.
	 * hour = hour % 24, 即hour最大取值23.
	 *
	 * */
	static time_t roll2NextHh(time_t ts, uchar hour);
	static time_t roll2NextHhmi(time_t ts, uchar hour, uchar minute); /** 参考roll2NextHh. */
	static time_t roll2NextHhmiss(time_t ts, uchar hour, uchar minute, uchar sec); /** 参考roll2NextHh. */
};

#endif /* DATEMISC_H_ */

