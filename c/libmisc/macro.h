/*
 * macro.h
 *
 *  Created on: 2013-7-12
 *      Author: xuzewen
 */

#ifndef MACRO_H_
#define MACRO_H_

#ifdef WINDOWS
#include <winsock2.h>
#include <windows.h>
#include <winbase.h>
#include <Iphlpapi.h>
#include <ws2tcpip.h>
#include <process.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <assert.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <locale.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef __MINGW_H
#include <unistd.h>
#include <sys/time.h>
#include <semaphore.h>
#include <pthread.h>
#endif
#if defined(LINUX) || defined(MAC)
#include <unistd.h>
#include <netdb.h>
#include <sys/time.h>
#include <semaphore.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <regex.h>
#ifdef LINUX
#include <netinet/tcp.h>
#include <net/if_arp.h>
#include <sys/epoll.h>
#include <sys/syscall.h>
#include <execinfo.h>
#include <dlfcn.h>
#endif
#include <sys/mman.h>
#include <net/if.h>
#include <arpa/inet.h>
#endif
#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <queue>
#ifdef WINDOWS
#include <unordered_set>
#include <unordered_map>
#else
#include <tr1/unordered_set>
#include <tr1/unordered_map>
#endif
using namespace std;
using namespace std::tr1;

#define __LIBMISC_H__

#if !defined (__LIBMISC_H__) && !defined (LIBMISC)
#error only libmisc.h can be included directly.
#endif

#ifndef uchar
#define uchar													unsigned char			/** 无符号单字节. */
#endif
//
#ifndef ushort
#define ushort													unsigned short			/** 无符号双字节. */
#endif
//
#ifndef uint
#define uint													unsigned int			/** 无符号四字节. */
#endif
//
#ifndef ulong
#define ulong													unsigned long			/** 无符号四/八字节. 为避免平台的差异, 不建议使用. */
#endif
//
#ifndef ullong
#define ullong  												unsigned long long int	/** 无符号八字节. */
#endif
//
#define UCHAR_MAX_VALUE											0xFF					/** uchar极限值. */
#define USHORT_MAX_VALUE										0xFFFF					/** ushort极限值. */
#define UINT_MAX_VALUE											0xFFFFFFFF				/** uint极限值. */
#define ULONG_MAX_VALUE											0xFFFFFFFF				/** ulong极限值. */
#define ULLONG_MAX_VALUE										0xFFFFFFFFFFFFFFFFULL	/** ullong极限值. */
//
#define PRESENT													0x01					/** present. */
#define NOT_PRESENT												0x00					/** not present. */
#define INVALID													  -1					/** invalid. */
//
#define H2N_DIR													0x00					/** host to network direction. */
#define N2H_DIR													0x01					/** network to host direction. */
//
#ifdef LINUX
#define gettid() 												syscall(__NR_gettid)
#endif
#ifdef WINDOWS
#define gettid() 												GetCurrentThreadId()
#endif

#endif /* MACRO_H_ */
