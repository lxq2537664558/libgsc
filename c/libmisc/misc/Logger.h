/*
 * Logger.h
 *
 *  Created on: 2010-12-1
 *      Author: xuzewen
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#if !defined (__LIBMISC_H__) && !defined (LIBMISC)
#error only libmisc.h can be included directly.
#endif

#include "DateMisc.h"
#include "../macro.h"

enum
{
	LOG_STDOUT = 0x01, /** 使能标准输出. */
	LOG_FILE = 0x02, /** 使能文件输出. */
	LOG_NET = 0x04, /** 使能网络输出. */
};

#ifdef WINDOWS
#define PATH_MAX																			4096
#endif

#define LOG_NET_PDU_SIZE																	65532				/** 发往logger服务器的单个日志记录的长度限制. */
#define LOG_NET_CACHE																		0x10000				/** 缓存的要发往logger服务器的记录数限制. */

class Logger
{
private:
	static bool c;
	static int o;
	static int lev;
	static int netfd;
	static int port;
	static char path[PATH_MAX];
#ifdef LINUX
	static pthread_mutex_t mutex;
	static pthread_cond_t cond;
	static pthread_condattr_t attr;
#endif
	static queue<char*> logs;
	static string host;
	static string ne;
private:
	Logger();
	virtual ~Logger();
	static void write(char* log);
	static void send(char* log);
	static void setLev(int lev);
	static void connect();
	static void sendHeartBeat();
	static int size();
	static void* svc(void* arg);
public:
	static void init(const char* ne /** network-equipment, 进程唯一标识, 用于日志归属识别. */= NULL, const char* path = NULL, const char* host = NULL, int port = 0, int o = LOG_STDOUT); /** 日志功能总是应该先初始化后再使用. */
	static void output(char* log); /** 日志输出. */
	static bool isOutPutStdout(); /** 是否将日志输出到stdout. */
	static bool isOutPutFile(); /** 是否将日志输出到文件. */
	static bool isOutPutNet(); /** 是否将日志输出到网络. */
	static void setRecord(); /** 设置日志级别到RECO. */
	static void setTrace(); /** 设置日志级别到TRACE. */
	static void setDebug(); /** 设置日志级别到DEBUG. */
	static void setInfo(); /** 设置日志级别到INFO. */
	static void setWarn(); /** 设置日志级别到WARN. */
	static void setError(); /** 设置日志级别到ERROR. */
	static void setFault(); /** 设置日志级别到FAULT. */
	static void setOper(); /** 设置日志级别到OPER. */
	static bool isRecord(); /** RECO是否可写. */
	static bool isTrace(); /** TRACE是否可写. */
	static bool isDebug(); /** DEBUG是否可写. */
	static bool isInfo(); /** INFO是否可写. */
	static bool isWarn(); /** WARN是否可写. */
	static bool isError(); /** ERROR是否可写. */
	static bool isFault(); /** FAULT是否可写. */
	static bool isOper(); /** OPER是否可写. */
	static void setLevel(const char* lev); /** 以字符串的形式设置日志级别. */
	static const char* getLevStr(); /** 返回日志级别的字符串形式. */
	static void updateNe(const char* ne); /** 一些时候, 进程可能不能立即知道自己的唯一标识, 因此这里提供一个更新方法. */
};

#define __RECO__(str, format, ...) SPRINTF_CSTR((str), "%s[RECO]%ld(%s %s %d)" format, DateMisc::hhmissms(), (long int)gettid(), __FILE__, __FUNCTION__, __LINE__,  ##__VA_ARGS__);
#define __TRAC__(str, format, ...) SPRINTF_CSTR((str), "%s[TRAC]%ld(%s %s %d)" format, DateMisc::hhmissms(), (long int)gettid(), __FILE__, __FUNCTION__, __LINE__,  ##__VA_ARGS__);
#define __DEBU__(str, format, ...) SPRINTF_CSTR((str), "%s[DEBU]%ld(%s %s %d)" format, DateMisc::hhmissms(), (long int)gettid(), __FILE__, __FUNCTION__, __LINE__,  ##__VA_ARGS__);
#define __INFO__(str, format, ...) SPRINTF_CSTR((str), "%s[INFO]%ld(%s %s %d)" format, DateMisc::hhmissms(), (long int)gettid(), __FILE__, __FUNCTION__, __LINE__,  ##__VA_ARGS__);
#define __WARN__(str, format, ...) SPRINTF_CSTR((str), "%s[WARN]%ld(%s %s %d)" format, DateMisc::hhmissms(), (long int)gettid(), __FILE__, __FUNCTION__, __LINE__,  ##__VA_ARGS__);
#define __ERRO__(str, format, ...) SPRINTF_CSTR((str), "%s[ERRO]%ld(%s %s %d)" format, DateMisc::hhmissms(), (long int)gettid(), __FILE__, __FUNCTION__, __LINE__,  ##__VA_ARGS__);
#define __FAUL__(str, format, ...) SPRINTF_CSTR((str), "%s[FAUL]%ld(%s %s %d)" format, DateMisc::hhmissms(), (long int)gettid(), __FILE__, __FUNCTION__, __LINE__,  ##__VA_ARGS__);
#define __OPER__(str, format, ...) SPRINTF_CSTR((str), "%s[OPER]%ld(%s %s %d)" format, DateMisc::hhmissms(), (long int)gettid(), __FILE__, __FUNCTION__, __LINE__,  ##__VA_ARGS__);

#ifdef WINDOWS
#define LOG_RECORD(format, ...)	\
if(Logger::isRecord()) \
{	\
	char ___cstr___[0x1000] = { 0 };	\
	__RECO__(___cstr___, format, ##__VA_ARGS__)	\
	Logger::output(___cstr___);	\
}

#define LOG_TRACE(format, ...)	\
if(Logger::isTrace()) \
{	\
	char ___cstr___[0x1000] = { 0 };	\
	__TRAC__(___cstr___, format, ##__VA_ARGS__)	\
	Logger::output(___cstr___);	\
}

#define LOG_DEBUG(format, ...)	\
if(Logger::isDebug()) \
{	\
	char ___cstr___[0x1000] = { 0 };	\
	__DEBU__(___cstr___, format, ##__VA_ARGS__)	\
	Logger::output(___cstr___);	\
}

#define LOG_INFO(format, ...)	\
if(Logger::isInfo()) \
{	\
	char ___cstr___[0x1000] = { 0 };	\
	__INFO__(___cstr___, format, ##__VA_ARGS__)	\
	Logger::output(___cstr___);	\
}

#define LOG_WARN(format, ...)	\
if(Logger::isWarn()) \
{	\
	char ___cstr___[0x1000] = { 0 };	\
	__WARN__(___cstr___, format, ##__VA_ARGS__)	\
	Logger::output(___cstr___);	\
}

#define LOG_ERROR(format, ...)	\
if(Logger::isError()) \
{	\
	char ___cstr___[0x1000] = { 0 };	\
	__ERRO__(___cstr___, format, ##__VA_ARGS__)	\
	Logger::output(___cstr___);	\
}

#define LOG_FAULT(format, ...)	\
if(Logger::isFault()) \
{	\
	char ___cstr___[0x1000] = { 0 };	\
	__FAUL__(___cstr___, format, ##__VA_ARGS__)	\
	Logger::output(___cstr___);	\
}

#define LOG_OPER(format, ...)	\
if(Logger::isOper()) \
{	\
	char ___cstr___[0x1000] = { 0 };	\
	__OPER__(___cstr___, format, ##__VA_ARGS__)	\
	Logger::output(___cstr___);	\
}

#define SPRINTF_STRING(__string__, format, ...) \
{	\
		char buff[0x400 * 10] = {0};	\
		::sprintf(buff, format, ##__VA_ARGS__);	\
		(__string__)->append(buff);	\
}

#define SPRINTF_CSTR(__cstr__, format, ...) \
{	\
		::sprintf(__cstr__, format, ##__VA_ARGS__);	\
}
#else
#define LOG_RECORD(format, ...)	\
if(Logger::isRecord()) \
{	\
	char* ___cstr___ = NULL;	\
	__RECO__(___cstr___, format, ##__VA_ARGS__)	\
	Logger::output(___cstr___);	\
}

#define LOG_TRACE(format, ...)	\
if(Logger::isTrace()) \
{	\
	char* ___cstr___ = NULL;	\
	__TRAC__(___cstr___, format, ##__VA_ARGS__)	\
	Logger::output(___cstr___);	\
}

#define LOG_DEBUG(format, ...)	\
if(Logger::isDebug()) \
{	\
	char* ___cstr___ = NULL;	\
	__DEBU__(___cstr___, format, ##__VA_ARGS__)	\
	Logger::output(___cstr___);	\
}

#define LOG_INFO(format, ...)	\
if(Logger::isInfo()) \
{	\
	char* ___cstr___ = NULL;	\
	__INFO__(___cstr___, format, ##__VA_ARGS__)	\
	Logger::output(___cstr___);	\
}

#define LOG_WARN(format, ...)	\
if(Logger::isWarn()) \
{	\
	char* ___cstr___ = NULL;	\
	__WARN__(___cstr___, format, ##__VA_ARGS__)	\
	Logger::output(___cstr___);	\
}

#define LOG_ERROR(format, ...)	\
if(Logger::isError()) \
{	\
	char* ___cstr___ = NULL;	\
	__ERRO__(___cstr___, format, ##__VA_ARGS__)	\
	Logger::output(___cstr___);	\
}

#define LOG_FAULT(format, ...)	\
if(Logger::isFault()) \
{	\
	char* ___cstr___ = NULL;	\
	__FAUL__(___cstr___, format, ##__VA_ARGS__)	\
	Logger::output(___cstr___);	\
}

#define LOG_OPER(format, ...)	\
if(Logger::isOper()) \
{	\
	char* ___cstr___ = NULL;	\
	__OPER__(___cstr___, format, ##__VA_ARGS__)	\
	Logger::output(___cstr___);	\
}

#define SPRINTF_STRING(__string__, format, ...) \
{	\
		char* __str__ = NULL;	\
		::asprintf(&__str__, format, ##__VA_ARGS__);	\
		(__string__)->append(__str__);	\
		::free(__str__);	\
}

#define SPRINTF_CSTR(__cstr__, format, ...) \
{	\
		::asprintf(&__cstr__,format, ##__VA_ARGS__);	\
}
#endif

#endif /* LOGGER_H_ */
