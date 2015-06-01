/*
 * Logger.cpp
 *
 *  Created on: 2010-12-1
 *      Author: xuzewen
 */

#include "Logger.h"
#include "Misc.h"
#include "Net.h"
#include "../stmp/stmp.h"

#define MAX_LOGFILE_SIZE										1024 * 1024 * 2000		/** log文件尺寸: ~=2G. */

enum
{
	/** level: record, trace, debug, info, warning, error, fault, operation. */
	LOG_LEVEL_RECO = 0x00, LOG_LEVEL_TRACE, LOG_LEVEL_DEBUG, LOG_LEVEL_INFO, LOG_LEVEL_WARN, LOG_LEVEL_ERROR, LOG_LEVEL_FAULT, LOG_LEVEL_OPER
};

static const char* LOG_LEV_STR[] = { "RECORD", "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FAULT", "OPER" };

bool Logger::c = false;
int Logger::o = LOG_STDOUT;
int Logger::lev = LOG_LEVEL_INFO;
int Logger::netfd = 0;
int Logger::port = 0;
char Logger::path[PATH_MAX] = { 0 };
#ifdef LINUX
pthread_mutex_t Logger::mutex;
pthread_cond_t Logger::cond;
pthread_condattr_t Logger::attr;
#endif
queue<char*> Logger::logs;
string Logger::host;
string Logger::ne;

Logger::Logger()
{

}

/** 日志功能总是应该先初始化后再使用. */
void Logger::init(const char* ne /** network-equipment, 进程唯一标识, 用于日志归属识别. */, const char* path, const char* host, int port, int o)
{
	Logger::o = o & 0x07;
	Logger::lev = LOG_LEVEL_INFO; /** default INFO. */
	Logger::host.assign(host == NULL ? "" : host);
	Logger::port = port;
	Logger::ne = ne == NULL ? "" : string(ne);
	if (path == NULL || strlen(path) == 0)
		memcpy(Logger::path, "./", strlen("./"));
	else
		memcpy(Logger::path, path, strlen(path) > PATH_MAX - 1 ? PATH_MAX - 1 : strlen(path));
#ifdef LINUX
	Misc::sleep(200);
	if (::getppid() == 1) /** daemon process, 在后台进程时, 总是输出到文件. */
		Logger::o |= LOG_FILE;
	if (Logger::isOutPutNet())
	{
		pthread_mutex_init(&Logger::mutex, NULL);
		pthread_condattr_setclock(&Logger::attr, CLOCK_MONOTONIC);
		pthread_cond_init(&Logger::cond, &Logger::attr);
		pthread_t t;
		if (pthread_create(&t, NULL, Logger::svc, NULL) == -1)
		{
			LOG_FAULT("no more thread can be create.")
			abort();
		}
		pthread_detach(t);
		Misc::sleep(500); /** wait for the connection establish with logger-server. */
	}
#endif
}

/** 是否将日志输出到stdout. */
bool Logger::isOutPutStdout()
{
	return Logger::o & LOG_STDOUT;
}

/** 是否将日志输出到文件. */
bool Logger::isOutPutFile()
{
	return Logger::o & LOG_FILE;
}

/** 是否将日志输出到网络. */
bool Logger::isOutPutNet()
{
	return Logger::o & (LOG_NET & 0x04);
}

/** 设置日志级别到RECO. */
void Logger::setRecord()
{
	Logger::setLev(LOG_LEVEL_RECO);
}

/** 设置日志级别到TRACE. */
void Logger::setTrace()
{
	Logger::setLev(LOG_LEVEL_TRACE);
}

/** 设置日志级别到DEBUG. */
void Logger::setDebug()
{
	Logger::setLev(LOG_LEVEL_DEBUG);
}

/** 设置日志级别到INFO. */
void Logger::setInfo()
{
	Logger::setLev(LOG_LEVEL_INFO);
}

/** 设置日志级别到WARN. */
void Logger::setWarn()
{
	Logger::setLev(LOG_LEVEL_WARN);
}

/** 设置日志级别到ERROR. */
void Logger::setError()
{
	Logger::setLev(LOG_LEVEL_ERROR);
}

/** 设置日志级别到FAULT. */
void Logger::setFault()
{
	Logger::setLev(LOG_LEVEL_FAULT);
}

/** 设置日志级别到OPER. */
void Logger::setOper()
{
	Logger::setLev(LOG_LEVEL_OPER);
}

/** 设置日志级别. */
void Logger::setLev(int lev)
{
	if (lev <= LOG_LEVEL_OPER && lev >= LOG_LEVEL_RECO)
		Logger::lev = lev;
}

/** RECO是否可写. */
bool Logger::isRecord()
{
	return Logger::lev <= LOG_LEVEL_RECO;
}

/** TRACE是否可写. */
bool Logger::isTrace()
{
	return Logger::lev <= LOG_LEVEL_TRACE;
}

/** DEBUG是否可写. */
bool Logger::isDebug()
{
	return Logger::lev <= LOG_LEVEL_DEBUG;
}

/** INFO是否可写. */
bool Logger::isInfo()
{
	return Logger::lev <= LOG_LEVEL_INFO;
}

/** WARN是否可写. */
bool Logger::isWarn()
{
	return Logger::lev <= LOG_LEVEL_WARN;
}

/** ERROR是否可写. */
bool Logger::isError()
{
	return Logger::lev <= LOG_LEVEL_ERROR;
}

/** FAULT是否可写. */
bool Logger::isFault()
{
	return Logger::lev <= LOG_LEVEL_FAULT;
}

/** OPER是否可写. */
bool Logger::isOper()
{
	return Logger::lev <= LOG_LEVEL_OPER;
}

/** 返回日志级别的字符串形式. */
const char* Logger::getLevStr()
{
	return LOG_LEV_STR[Logger::lev];
}

/** 一些时候, 进程可能不能立即知道自己的唯一标识, 因此这里提供一个更新方法. */
void Logger::updateNe(const char* ne)
{
	Logger::ne = string(ne);
}

/** 以字符串的形式设置日志级别. */
void Logger::setLevel(const char* lev)
{
	if (strcmp(lev, LOG_LEV_STR[LOG_LEVEL_RECO]) == 0)
		Logger::setRecord();
	else if (strcmp(lev, LOG_LEV_STR[LOG_LEVEL_TRACE]) == 0)
		Logger::setTrace();
	else if (strcmp(lev, LOG_LEV_STR[LOG_LEVEL_DEBUG]) == 0)
		Logger::setDebug();
	else if (strcmp(lev, LOG_LEV_STR[LOG_LEVEL_WARN]) == 0)
		Logger::setWarn();
	else if (strcmp(lev, LOG_LEV_STR[LOG_LEVEL_ERROR]) == 0)
		Logger::setError();
	else if (strcmp(lev, LOG_LEV_STR[LOG_LEVEL_FAULT]) == 0)
		Logger::setFault();
	else if (strcmp(lev, LOG_LEV_STR[LOG_LEVEL_OPER]) == 0)
		Logger::setOper();
	else
		Logger::setInfo();
}

/** 日志输出. */
void Logger::output(char* log)
{
	if (Logger::isOutPutStdout())
		printf("%s\n", log);
#ifdef LINUX
	if (Logger::isOutPutNet())
	{
		Logger::send(log);
		return;
	}
#endif
	if (Logger::isOutPutFile())
		Logger::write(log);
#ifndef WINDOWS
	free(log);
#endif
}

/** 将日志写入本地文件. */
void Logger::write(char* log)
{
	char file[0x20] = { 0 };
	sprintf(file, "%s%s.log", Logger::path, DateMisc::yyyymmdd());
	struct stat filestat;
	if (stat(file, &filestat) != -1 && filestat.st_size > MAX_LOGFILE_SIZE)
	{
		char nfile[0x20] = { 0 };
		sprintf(nfile, "%s.bak", file);
		remove(nfile);
		rename(file, nfile);
	}
	FILE *fp;
	if ((fp = fopen(file, "a+")) == NULL)
		return;
	fprintf(fp, "%s\n", log);
	fclose(fp);
}

/** 尝试发送一段日志到远端日志服务器. */
void Logger::send(char* log)
{
#ifdef LINUX
	if (Logger::size() > LOG_NET_CACHE)
	{
		LOG_WARN("too many log cached, limit: %u", LOG_NET_CACHE)
		free(log);
		return;
	}
	if (strlen(log) > LOG_NET_PDU_SIZE) /** 超出单条日志尺寸. */
	{
		LOG_FAULT("over the LOG_NET_PDU_SIZE: %u", (int )strlen(log))
		free(log);
		return;
	}
	bool nll = false;
	pthread_mutex_lock(&Logger::mutex);
	nll = Logger::logs.empty();
	Logger::logs.push(log);
	pthread_mutex_unlock(&Logger::mutex);
	if (nll)
		pthread_cond_signal(&Logger::cond);
#endif
}

/** 尝试连接到远端日志服务器. */
void Logger::connect()
{
	while (true)
	{
		int sock = Net::tcpConnect(Logger::host.c_str(), Logger::port);
		if (sock < 1)
		{
			LOG_ERROR("can not connect to logger-server: %s:%d", Logger::host.c_str(), Logger::port)
			Misc::sleep(3000);
			continue;
		}
		if (!Net::setSNDBUF(sock, 64 * 1024) || !Net::setLinger(sock)) /** 套接字是阻塞的. */
		{
			Net::close(sock);
			Misc::sleep(3000);
			continue;
		}
		Logger::netfd = sock;
		LOG_INFO("got a connection from logger-server: %s:%d", Logger::host.c_str(), Logger::port)
		break;
	}
}

/** 返回缓存的要发送到日志服务器的记录数. */
int Logger::size()
{
#ifdef LINUX
	int size;
	pthread_mutex_lock(&Logger::mutex);
	size = Logger::logs.size();
	pthread_mutex_unlock(&Logger::mutex);
	return size;
#else
	return 0;
#endif
}

/** 尝试连接到远端日志服务器, 并发送初始消息. */
void* Logger::svc(void* arg)
{
#ifdef LINUX
	Logger::connect();
	while (true)
	{
		if (Logger::ne.length() < 1) /** 等待设置网元标识. */
		{
			Misc::sleep(100);
			continue;
		}
		char* log = NULL;
		pthread_mutex_lock(&Logger::mutex);
		while (Logger::logs.empty())
		{
			struct timespec ts;
			clock_gettime(CLOCK_MONOTONIC, &ts);
			ts.tv_sec += 10; /** 心跳间隔. */
			if (pthread_cond_timedwait(&Logger::cond, &Logger::mutex, &ts) == ETIMEDOUT)
				break;
		}
		if (!Logger::logs.empty())
		{
			log = Logger::logs.front();
			Logger::logs.pop();
		}
		pthread_mutex_unlock(&Logger::mutex);
		if (log == NULL)
		{
			Logger::sendHeartBeat();
			continue;
		}
		/** GS-0001,15:14:49.994[INFO]16018(../net/Lx.cpp svc 78)worker-thread start successfully, index: 01, epoll-fd: 00000006, pipe-read: 0000000B, pipe-write: 0000000C. */
		string nlog = Logger::ne + "," + log;
		int size = nlog.length();
		size = size + (8 - (size % 8)) + 64;
		stmp_pdu* sp = (stmp_pdu*) malloc(sizeof(stmp_pdu));
		sp->len = size;
		sp->rm = size;
		sp->p = 0;
		sp->buff = (uchar*) malloc(size);
		stmpenc_add_bin(sp, STMP_TAG_ATT, (uchar*) nlog.c_str(), nlog.length());
		stmpenc_add_short(sp, STMP_TAG_CMD, 0x0000);
		stmpenc_add_tag(sp, STMP_TAG_TRANS_UNI);
		uint len;
		uchar* dat = stmpenc_take(sp, &len);
		free(log);
		loop: //
		if (Net::tcpSend(Logger::netfd, dat, len) != (int) len)
		{
			LOG_ERROR("logger-server socket exception: %s:%d", Logger::host.c_str(), Logger::port)
			Net::close(Logger::netfd);
			Logger::netfd = 0;
			Misc::sleep(3000);
			Logger::connect();
			goto loop;
		}
		free(sp->buff);
		free(sp);
	}
#endif
	return NULL;
}

/** 尝试发送心跳到日志服务器. */
void Logger::sendHeartBeat()
{
	static bool init = true;
	static const char* hb = "heart-beat";
	static uchar* dat = NULL;
	static uint len = 0;
	if (init)
	{
		int size = strlen(hb);
		size = size + (8 - (size % 8)) + 64;
		stmp_pdu* sp = (stmp_pdu*) malloc(sizeof(stmp_pdu));
		sp->len = size;
		sp->rm = size;
		sp->p = 0;
		sp->buff = (uchar*) malloc(size);
		stmpenc_add_bin(sp, STMP_TAG_ATT, (uchar*) hb, strlen(hb));
		stmpenc_add_short(sp, STMP_TAG_CMD, 0x0000);
		stmpenc_add_tag(sp, STMP_TAG_TRANS_UNI);
		dat = stmpenc_take(sp, &len);
		init = false;
	}
	Net::tcpSend(Logger::netfd, dat, len);
}

Logger::~Logger()
{

}

