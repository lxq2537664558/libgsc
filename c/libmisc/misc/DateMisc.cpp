/*
 * DateMisc.cpp
 *
 *  Created on: 2010-12-1
 *      Author: xuzewen
 */

#include "DateMisc.h"
#include "Logger.h"

time_t DateMisc::sec = 1; /** 秒. */
time_t DateMisc::minute = DateMisc::sec * 60; /** 分. */
time_t DateMisc::hour = DateMisc::minute * 60; /** 小时. */
time_t DateMisc::day = DateMisc::hour * 24; /** 天. */

#if defined(WINDOWS)
void localtime_r(time_t* t, struct tm* tm)
{
	::localtime_s(tm, t);
}
#endif

DateMisc::DateMisc()
{

}

/** gettimeofday. */
void DateMisc::gettimeofday(struct timeval* tv)
{
#if defined(WINDOWS) && !defined(__MINGW_H)
	FILETIME ft;
	u_int64_t tim;
	GetSystemTimeAsFileTime(&ft);
	tim = filetime_to_unix_epoch(&ft);
	tv->tv_sec = (long) (tim / 1000000ULL);
	tv->tv_usec = (long) (tim % 1000000ULL);
#else
	::gettimeofday(tv, NULL);
#endif
}

/** 返回当前时间戳, 微秒. */
ullong DateMisc::getTs()
{
	struct timeval tv;
	DateMisc::gettimeofday(&tv);
	ullong ts = tv.tv_sec;
	ts *= 1000000ULL;
	ts += tv.tv_usec;
	return ts;
}

/** 返回当前时间戳, 毫秒. */
ullong DateMisc::getMsec()
{
	return DateMisc::getTs() / 1000ULL;
}

/** 返回两个时间点相差的微秒数, now应该大于old. */
int DateMisc::getElap(struct timeval* now, struct timeval* old)
{
	long int sec = now->tv_sec - old->tv_sec;
	long int use = now->tv_usec - old->tv_usec;
	sec = use < 0 ? (sec > 0 ? sec - 1 : sec) : sec; /** 获得秒.*/
	use = use < 0 ? (1000000 - old->tv_usec + now->tv_usec) : (now->tv_usec - old->tv_usec);
	return (int) (sec * 1000000 + use);
}

/** 返回指定时间的毫秒数, 微秒被抛弃. */
ullong DateMisc::getMsec(struct timeval* ts)
{
	if (ts != NULL)
	{
		ullong x = ts->tv_sec;
		x = x * (ullong) 1000;
		x += (ts->tv_usec / 1000);
		return x;
	}
	struct timeval now;
	DateMisc::gettimeofday(&now);
	return DateMisc::getMsec(&now);
}

/** 返回系统时钟滴答声, 精确到微秒. */
ullong DateMisc::getDiDa()
{
#ifdef WIN32
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	double f = (double) freq.QuadPart;
	QueryPerformanceCounter(&freq);
	double s = freq.QuadPart;
	return (s / f) * 1000 * 1000;
#else
#ifdef LINUX
	struct timespec dida;
	clock_gettime(CLOCK_MONOTONIC, &dida);
	return (dida.tv_sec * 1000000) + (dida.tv_nsec / 1000);
#else
	return 0ULL; /** unsupported OS. */
#endif
#endif
}

/** 返回系统时钟滴答声, 精确到毫秒. */
ullong DateMisc::getDiDaMsec()
{
	return DateMisc::getDiDa() / 1000ULL;
}

/** 当前时间: yyyymm. */
char* DateMisc::yyyymm()
{
	static char buff[0x07] = { 0 };
	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	sprintf(buff, "%04d%02d", t->tm_year + 1900, t->tm_mon + 1);
	return buff;
}
/** 当前时间: yyyy-mm-dd. */
char* DateMisc::yyyymmdd()
{
	static char buff[0x0B] = { 0 };
	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	sprintf(buff, "%04d-%02d-%02d", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);
	return buff;
}

/** 指定时间: yyyy-mm-dd. */
string DateMisc::yyyymmdd(time_t ts)
{
	string str;
	struct tm ft;
	localtime_r(&ts, &ft);
	SPRINTF_STRING(&str, "%04d-%02d-%02d", ft.tm_year + 1900, ft.tm_mon + 1, ft.tm_mday);
	return str;
}

/** 当前时间: yyyy-mm-dd hh. */
char* DateMisc::yyyymmddhh()
{
	static char buff[0x0F] = { 0 };
	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	sprintf(buff, "%04d-%02d-%02d-%02d", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour);
	return buff;
}

/** 当前时间: yyyy-mm-dd hh:mi:ss. */
char* DateMisc::yyyymmddhhmiss()
{
	static char buff[0x14] = { 0 };
	DateMisc::yyyymmddhhmiss(time(NULL), buff);
	return buff;
}

/** 当前时间: yyyymmddhhmiss. */
char* DateMisc::yyyymmddhhmissShort()
{
	static char buff[0x0F] = { 0 };
	DateMisc::yyyymmddhhmissShort(time(NULL), buff);
	return buff;
}

/** 指定时间: yyyy-mm-dd hh:mi:ss.*/
void DateMisc::yyyymmddhhmiss(time_t ts, char* str)
{
	struct tm ft;
	localtime_r(&ts, &ft);
	sprintf(str, "%04d-%02d-%02d %02d:%02d:%02d", ft.tm_year + 1900, ft.tm_mon + 1, ft.tm_mday, ft.tm_hour, ft.tm_min, ft.tm_sec);
}

/** 指定时间: yyyy-mm-dd hh:mi:ss.*/
string DateMisc::yyyymmddhhmiss(time_t ts)
{
	string str;
	struct tm ft;
	localtime_r(&ts, &ft);
	SPRINTF_STRING(&str, "%04d-%02d-%02d %02d:%02d:%02d", ft.tm_year + 1900, ft.tm_mon + 1, ft.tm_mday, ft.tm_hour, ft.tm_min, ft.tm_sec)
	return str;
}

/** 指定时间: yyyymmddhhmiss.*/
void DateMisc::yyyymmddhhmissShort(time_t ts, char* str)
{
	struct tm ft;
	localtime_r(&ts, &ft);
	sprintf(str, "%04d%02d%02d%02d%02d%02d", ft.tm_year + 1900, ft.tm_mon + 1, ft.tm_mday, ft.tm_hour, ft.tm_min, ft.tm_sec);
}

/** 指定时间: yyyymmddhhmiss.*/
string DateMisc::yyyymmddhhmissShort(time_t ts)
{
	string str;
	struct tm ft;
	localtime_r(&ts, &ft);
	SPRINTF_STRING(&str, "%04d%02d%02d%02d%02d%02d", ft.tm_year + 1900, ft.tm_mon + 1, ft.tm_mday, ft.tm_hour, ft.tm_min, ft.tm_sec)
	return str;
}

/** 当前时间: hh:mi:ss. */
char* DateMisc::hhmiss()
{
	static char buff[0x09] = { 0 };
	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	strftime(buff, 0x09, "%X", t);
	return buff;
}

/** 当前时间: hh:mi:ss.ms. */
char* DateMisc::hhmissms()
{
	static char buff[0x0D] = { 0 };
#if defined(WINDOWS)
	SYSTEMTIME t;
	GetLocalTime(&t);
	sprintf(buff, "%02d:%02d:%02d.%03d", t.wHour, t.wMinute, t.wSecond, t.wMilliseconds);
#else
	struct timeval tv;
	DateMisc::gettimeofday(&tv);
	struct tm *t = localtime((time_t*) &(tv.tv_sec));
	sprintf(buff, "%02d:%02d:%02d.%03d", t->tm_hour, t->tm_min, t->tm_sec, (int) (tv.tv_usec / 1000));
#endif
	return buff;
}

/** 上个月. */
char* DateMisc::yyyymmLast()
{
	static char buff[0x07] = { 0 };
	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	int y = t->tm_year + 1900;
	int m = t->tm_mon + 1;
	if (m > 1)
		m -= 1;
	else
	{
		y -= 1;
		m = 12;
	}
	sprintf(buff, "%04d%02d", y, m);
	return buff;
}

/** 比较两个时间是不是在同一天. */
bool DateMisc::isSameDay(time_t f, time_t s)
{
	struct tm ft;
	struct tm st;
	::localtime_r(&f, &ft);
	::localtime_r(&s, &st);
	return (ft.tm_mday == st.tm_mday) && (::abs(f - s) < DateMisc::day);
}

/** 将yyyy-mm-dd hh:mi:ss(如2013-11-29 23:59:59)转换成绝对时间秒. */
time_t DateMisc::parseYYYY_MM_DD_HHMISS(char* str)
{
	struct tm tm;
#ifdef LINUX
	::strptime(str, "%Y-%m-%d %H:%M:%S", &tm);
#else
	LOG_FAULT("unsupported OS.")
#endif

	return ::mktime(&tm);
}

/** 返回指定绝对时间的年部分. */
int DateMisc::getYear(time_t ts)
{
	struct tm ft;
	::localtime_r(&ts, &ft);
	return ft.tm_year + 1900;
}

/** 返回指定绝对时间的月部分. */
int DateMisc::getMonth(time_t ts)
{
	struct tm ft;
	::localtime_r(&ts, &ft);
	return ft.tm_mon + 1;
}

/** 返回指定绝对时间的月部分. */
int DateMisc::getDay(time_t ts)
{
	struct tm ft;
	::localtime_r(&ts, &ft);
	return ft.tm_mday;
}

/** 返回指定绝对时间的小时部分. */
int DateMisc::getHour(time_t ts)
{
	struct tm ft;
	::localtime_r(&ts, &ft);
	return ft.tm_hour;
}

/** 返回指定绝对时间的分钟部分. */
int DateMisc::getMinu(time_t ts)
{
	struct tm ft;
	::localtime_r(&ts, &ft);
	return ft.tm_min;
}

/** 返回指定绝对时间的秒部分. */
int DateMisc::getSec(time_t ts)
{
	struct tm ft;
	::localtime_r(&ts, &ft);
	return ft.tm_sec;
}

/** 构造一个指定的时间. */
time_t DateMisc::makeTime(int year, int month, int day, int hour, int minu, int sec)
{
	struct tm tm;
	tm.tm_year = year;
	tm.tm_mon = month;
	tm.tm_mday = day;
	tm.tm_hour = hour;
	tm.tm_min = minu;
	tm.tm_sec = sec;
	return ::mktime(&tm);
}

/**
 *
 * 以ts为界, 将ts调至指定的小时.
 * 如果ts此时为2013-10-17 10:13:12.
 * 当hour为12时, 将返回2013-10-17 12:00:00
 * 当hour为9时, 将返回2013-10-18 09:00:00
 * hour = hour % 24, 即hour最大取值23.
 *
 * */
time_t DateMisc::roll2NextHh(time_t ts, uchar hour)
{
	hour %= 24;
	struct tm ft;
	::localtime_r(&ts, &ft);
	ts = (ts - ft.tm_hour * DateMisc::hour - ft.tm_min * DateMisc::minute - ft.tm_sec);
	if (hour > (time_t) ft.tm_hour)
		return (ts + hour * DateMisc::hour);
	else
		return (ts + DateMisc::day + hour * DateMisc::hour);
}

/** 参考roll2NextHh. */
time_t DateMisc::roll2NextHhmi(time_t ts, uchar hour, uchar minute)
{
	minute %= 60;
	hour %= 24;
	struct tm ft;
	::localtime_r(&ts, &ft);
	ts = (ts - ft.tm_hour * DateMisc::hour - ft.tm_min * DateMisc::minute - ft.tm_sec);
	if ((hour > (time_t) ft.tm_hour))
		return (ts + hour * DateMisc::hour) + minute * DateMisc::minute;
	else if ((hour == (time_t) ft.tm_hour) && (minute > ft.tm_min))
		return (ts + hour * DateMisc::hour) + minute * DateMisc::minute;
	return (ts + DateMisc::day + hour * DateMisc::hour) + minute * DateMisc::minute;
}

/** 参考roll2NextHh. */
time_t DateMisc::roll2NextHhmiss(time_t ts, uchar hour, uchar minute, uchar sec)
{
	sec %= 60;
	minute %= 60;
	hour %= 24;
	struct tm ft;
	::localtime_r(&ts, &ft);
	ts = (ts - ft.tm_hour * DateMisc::hour - ft.tm_min * DateMisc::minute - ft.tm_sec);
	if ((hour > (time_t) ft.tm_hour))
		return (ts + hour * DateMisc::hour) + minute * DateMisc::minute + sec;
	else
	{
		if ((hour == (time_t) ft.tm_hour))
		{
			if ((minute > ft.tm_min))
				return (ts + hour * DateMisc::hour) + minute * DateMisc::minute + sec;
			else if (minute == ft.tm_min && sec > ft.tm_sec)
				return (ts + hour * DateMisc::hour) + minute * DateMisc::minute + sec;
		}
	}
	return (ts + DateMisc::day + hour * DateMisc::hour) + minute * DateMisc::minute + sec;
}

DateMisc::~DateMisc()
{

}

