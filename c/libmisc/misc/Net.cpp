/*
 * Net.cpp
 *
 *  Created on: 2013-4-26
 *      Author: xuzewen
 */

#include "Net.h"
#include "Logger.h"
#include "Misc.h"

Net::Net()
{

}

/** 返回整形ipv4地址的点分十进制字符串表现形式. */
void Net::ip2str(uint ip, char* str)
{
	sprintf(str, "%d.%d.%d.%d", (ip >> 24 & 0xFF), (ip >> 16 & 0xFF), (ip >> 8 & 0xFF), (ip & 0xFF));
}

/** 返回整形ipv4地址的点分十进制字符串表现形式. */
string Net::ip2str(uint ip)
{
	string str;
	SPRINTF_STRING(&str, "%d.%d.%d.%d", (ip >> 24 & 0xFF), (ip >> 16 & 0xFF), (ip >> 8 & 0xFF), (ip & 0xFF));
	return str;
}

/** 是否为域名. */
bool Net::isDomain(const char* host)
{
	for (uint i = 0; i < strlen(host); ++i)
	{
		if (host[i] == '.')
			continue;
		if (host[i] < '0' || host[i] > '9')
			return true;
	}
	return false;
}

/** 返回socket地址描述的字符串表现形式(点分十进制:端口)*/
void Net::sockaddr2str(struct sockaddr_in* addr, char* str)
{
	Net::ip2str(ntohl(addr->sin_addr.s_addr), str);
	sprintf(str + strlen(str), ":%d", ntohs(addr->sin_port));
}

/** 返回socket地址描述的字符串表现形式(点分十进制:端口)*/
string Net::sockaddr2str(struct sockaddr_in* addr)
{
	string str;
	uint ip = ntohl(addr->sin_addr.s_addr);
	SPRINTF_STRING(&str, "%d.%d.%d.%d", (ip >> 24 & 0xFF), (ip >> 16 & 0xFF), (ip >> 8 & 0xFF), (ip & 0xFF))
	SPRINTF_STRING(&str, ":%d", ntohs(addr->sin_port))
	return str;
}

/** 通过域名, 获得一个ip. */
void Net::getHostByName(const char* domain, string* ip)
{
	struct hostent* host = gethostbyname(domain);
	if (host != NULL && host->h_addr_list[0] != NULL)
	{
		struct in_addr addr;
		addr.s_addr = *((uint*) (host->h_addr_list[0]));
		char* x = inet_ntoa(addr);
		SPRINTF_STRING(ip, "%s", x);
	}
}

/** 尝试向host:port建立一个TCP连接, 返回-1时表示失败. */
int Net::tcpConnect(const char* host, ushort port)
{
	if (host == NULL || port == 0)
		return -1;
	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	struct sockaddr_in addr;
	Net::setAddr(host, port, &addr);
	if (connect(sock, (struct sockaddr*) &addr, sizeof(struct sockaddr)) == 0)
		return sock;
	Net::close(sock);
	return -1;
}

/** 非阻塞的TCP连接. */
int Net::tcpConnectNoBlocking(const char* host, ushort port, uint sec /** 超时时间(秒). */, uint rcvbuf, uint sndbuf)
{
	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	Net::setNoBlocking(sock);
	Net::setRCVBUF(sock, rcvbuf);
	Net::setSNDBUF(sock, sndbuf);
	Net::setLinger(sock);
	//
	struct sockaddr_in addr;
	Net::setAddr(host, port, &addr);
	int r = ::connect(sock, (struct sockaddr*) &addr, sizeof(struct sockaddr));
	if (r == 0) /** 连接成功. */
		return sock;
#ifdef WINDOWS
	if(WSAGetLastError() != WSAEWOULDBLOCK)
	goto loop;
#else
	if (errno != EINPROGRESS) /** 连接失败. */
		goto loop;
#endif
	//
	struct timeval tv;
	tv.tv_sec = sec;
	tv.tv_usec = 0;
	fd_set wset;
	FD_ZERO(&wset);
	FD_SET(sock, &wset);
	r = ::select(sock + 1, NULL, &wset, NULL, &tv);
	if (r <= 0)
		goto loop;
	if (FD_ISSET(sock, &wset))
	{
		int err;
		socklen_t len = sizeof(int);
		if (getsockopt(sock, SOL_SOCKET, SO_ERROR, (char* /** for windows. */) &err, &len) < 0 || err)
			goto loop;
	} else
	{
		loop: //
		Net::close(sock);
		return -1;
	}
	return sock;
}

/** 尝试在指定的本地地址和端口上建立一个TCP监听. 如果成功, 返回套接字句柄, 否则返回 -1. */
int Net::tcpListen(const char* host, ushort port)
{
	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	int x = SO_REUSEADDR;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*) &x, sizeof(int)) != 0)
	{
		Net::close(sock);
		return -1;
	}
	struct sockaddr_in addr;
	Net::setAddr(host, port, &addr);
	if (bind(sock, (struct sockaddr*) &addr, sizeof(struct sockaddr)) == 0 && listen(sock, 0xFFFF) == 0)
		return sock;
	Net::close(sock);
	return -1;
}

/** 在服务端套接字上等待一个tcp连接(阻塞), 成功时返回与peer相关的套接字句柄, 否则返回与平台有关的accept返回值. */
int Net::tcpAccept(int sock, struct sockaddr* peer)
{
	static socklen_t x = sizeof(struct sockaddr);
	return accept(sock, peer, &x);
}

/** 尝试在TCP套接字上接收一段数据. sock应该是阻塞模式的. */
int Net::tcpRecv(int sock, uchar* buff, int size)
{
	return ::recv(sock, (char*) buff, size, 0);
}

/** 尝试在TCP套接字上接收一段数据. sock应该是非阻塞模式的. */
int Net::tcpRecvNoBlocking(int sock, uchar* buff, int size)
{
#ifndef WINDOWS
	return ::recv(sock, buff, size, MSG_DONTWAIT);
#else
	return ::recv(sock, (char*)buff, size, 0);
#endif
}

/** 尝试在TCP套接字上接收一段数据, sock应该是阻塞模式的, 除非套接字异常, 函数总是返回size个字节. */
bool Net::tcpRecvN(int sock, uchar* buff, int size)
{
	if (size == 0)
		return false;
	int x = 0;
	while (1)
	{
		int len = recv(sock, (char*) buff + x, size - x, 0);
		if (len < 1)
			return false;
		x += len;
		if (x >= size)
			return true;
	}
	return false;
}

/** 尝试往TCP套接字上发送一段数据, 返回值依赖于sock本身的特性. */
int Net::tcpSend(int sock, uchar* buff, int size)
{
	return send(sock, (char*) buff, size, 0);
}

/** 以非阻塞模式绑定本地UDP端口, 并设置了大约512K的R/S缓冲区, 如果不关心本地addr, 可置为NULL. */
int Net::udpBind0(const char* host, ushort port, struct sockaddr_in* addr)
{
	int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	int x = SO_REUSEADDR;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*) &x, sizeof(int)) != 0)
	{
		Net::close(sock);
		return -1;
	}
	if (Net::setNoBlocking(sock) != 0)
	{
		Net::close(sock);
		return -1;
	}
	x = 512 * 1472;
	Net::setRCVBUF(sock, x);
	Net::setSNDBUF(sock, x);
	if (addr == NULL)
	{
		struct sockaddr_in xr;
		Net::setAddr(host, port, &xr);
		if (bind(sock, (struct sockaddr*) &xr, sizeof(struct sockaddr)) != 0)
		{
			Net::close(sock);
			return -1;
		}
	} else
	{
		Net::setAddr(host, port, addr);
		if (bind(sock, (struct sockaddr*) addr, sizeof(struct sockaddr)) != 0)
		{
			Net::close(sock);
			return -1;
		}
	}
	return sock;
}

/** 以阻塞模式绑定本地UDP端口, 并设置了大约512K的R/S缓冲区, 如果不关心本地addr, 可置为NULL. */
int Net::udpBind1(const char* host, ushort port, struct sockaddr_in* addr)
{
	int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	int x = SO_REUSEADDR;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*) &x, sizeof(int)) != 0)
	{
		Net::close(sock);
		return -1;
	}
	x = 512 * 1472;
	Net::setRCVBUF(sock, x);
	Net::setSNDBUF(sock, x);
	if (addr == NULL)
	{
		struct sockaddr_in xr;
		Net::setAddr(host, port, &xr);
		if (bind(sock, (struct sockaddr*) &xr, sizeof(struct sockaddr)) != 0)
		{
			Net::close(sock);
			return -1;
		}
	} else
	{
		Net::setAddr(host, port, addr);
		if (bind(sock, (struct sockaddr*) addr, sizeof(struct sockaddr)) != 0)
		{
			Net::close(sock);
			return -1;
		}
	}
	return sock;
}

/** 尝试在sock上接收一段报文, 远端地址被填充在from中.  */
int Net::udpRecv(int sock, uchar* buff, int size, struct sockaddr_in* from)
{
	static socklen_t x = sizeof(struct sockaddr);
	return recvfrom(sock, (char*) buff, size, 0, (struct sockaddr*) from, &x);
}

/** 向指定的地址发送一段UPD报文.  */
int Net::udpSend(int sock, uchar* buff, int size, struct sockaddr_in* to)
{
	return sendto(sock, (char*) buff, size, 0, (struct sockaddr*) to, sizeof(struct sockaddr));
}

/** 设置套接字为非阻塞. */
bool Net::setNoBlocking(int sock)
{
#ifdef WIN32
	u_long mode = 1;
	ioctlsocket(sock, FIONBIO, &mode);
#else
	int opt = fcntl(sock, F_GETFL);
	if (opt < 0)
	{
		LOG_ERROR("get FL failed, errno: %d", errno)
		return false;
	}
	opt |= O_NONBLOCK;
	if (fcntl(sock, F_SETFL, opt) < 0)
	{
		LOG_ERROR("set FL failed, errno: %d", errno)
		return false;
	}
#endif
	return true;
}

/** 简单粗暴的套接字关闭类型. */
bool Net::setLinger(int sock)
{
	struct linger ling = { 1, 0 };
	if (setsockopt(sock, SOL_SOCKET, SO_LINGER, (char*) &ling, sizeof(struct linger)) != 0)
	{
#ifdef WIN32
		LOG_ERROR("set SO_LINGER opt failed, sock: %d, errno: %ld", sock, GetLastError())
#else
		LOG_ERROR("set SO_LINGER opt failed, sock: %d, errno: %d", sock, errno)
#endif
		return false;
	}
	return true;
}

/** 设置套接字发送缓冲区. */
bool Net::setSNDBUF(int sock, int size)
{
	if (setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char*) &size, 4) != 0)
	{
#ifdef WIN32
		LOG_ERROR("set SO_SNDBUF opt failed, sock: %d, errno: %ld", sock, GetLastError())
#else
		LOG_ERROR("set SO_SNDBUF opt failed, sock: %d, errno: %d", sock, errno)
#endif
		return false;
	}
	return true;
}

/** 设置套接字接收缓冲区. */
bool Net::setRCVBUF(int sock, int size)
{
	if (setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char*) &size, 4) != 0)
	{
#ifdef WIN32
		LOG_ERROR("set SO_RCVBUF opt failed, sock: %d, errno: %ld", sock, GetLastError())
#else
		LOG_ERROR("set SO_RCVBUF opt failed, sock: %d, errno: %d", sock, errno)
#endif
		return false;
	}
	return true;
}

/** 调置套接字的发送超时. */
bool Net::setSNDTIMEO(int sock, int sec)
{
#ifdef WIN32
	int t = sec * 1000;
	if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char *) &t, sizeof(int)) != 0)
	{
		LOG_ERROR("set SO_SNDTIMEO opt failed, sock: %d, errno: %ld", sock, GetLastError())
		return false;
	}
#else
	struct timeval tv;
	tv.tv_sec = sec;
	tv.tv_usec = 0;
	if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char*) &tv, sizeof(struct timeval)) != 0)
	{
		LOG_ERROR("set SO_SNDTIMEO opt failed, sock: %d, errno: %d", sock, errno)
		return false;
	}
#endif
	return true;
}

/** 调置套接字的接收超时. */
bool Net::setRCVTIMEO(int sock, int ts)
{
#ifdef WIN32
	int t = ts * 1000;
	if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *) &t, sizeof(int)) != 0)
	{
		LOG_ERROR("set SO_RCVTIMEO opt failed, sock: %d, errno: %ld", sock, GetLastError())
		return false;
	}
#else
	struct timeval tv;
	tv.tv_sec = ts;
	tv.tv_usec = 0;
	if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*) &tv, sizeof(struct timeval)) != 0)
	{
		LOG_ERROR("set SO_RCVTIMEO opt failed, sock: %d, errno: %d", sock, errno)
		return false;
	}
#endif
	return true;
}

/** 设置TCP_NODELAY选项. */
bool Net::setNODELAY(int sock)
{
	int x = 1;
#ifdef WIN32
	if (setsockopt(sock, SOL_SOCKET, TCP_NODELAY, (char *) &x, sizeof(int)) != 0)
	{
		LOG_ERROR("set TCP_NODELAY opt failed, sock: %d, errno: %ld", sock, GetLastError())
		return false;
	}
#endif
#if defined(LINUX)
	if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char*) &x, sizeof(int)) != 0)
	{
		LOG_ERROR("set TCP_NODELAY opt failed, sock: %d, errno: %d", sock, errno)
		return false;
	}
#endif
	return true;
}

/** 将host和port设置到addr. */
void Net::setAddr(const char* host, int port, struct sockaddr_in* addr)
{
	addr->sin_family = AF_INET;
	addr->sin_port = htons(port);
	addr->sin_addr.s_addr = host == NULL ? htonl(INADDR_ANY) : inet_addr(host);
}

/** 大端在前. */
void Net::bigEndianInt(uint x, uchar* dat)
{
	dat[0] = (uchar) (x >> 24);
	dat[1] = (uchar) (x >> 16);
	dat[2] = (uchar) (x >> 8);
	dat[3] = (uchar) (x);
}

/** 小端在前. */
void Net::litEndianInt(uint x, uchar* dat)
{
	dat[0] = (uchar) (x);
	dat[1] = (uchar) (x >> 8);
	dat[2] = (uchar) (x >> 16);
	dat[3] = (uchar) (x >> 24);
}

/** 长整型转网络字节序(假设主机是小头在前). */
ullong Net::htonll(ullong h)
{
	ullong n = 0ULL;
	n |= ((h << 56) & 0xFF00000000000000ULL);
	n |= ((h << 40) & 0x00FF000000000000ULL);
	n |= ((h << 24) & 0x0000FF0000000000ULL);
	n |= ((h << 8) & 0x000000FF00000000ULL);
	n |= ((h >> 8) & 0x00000000FF000000ULL);
	n |= ((h >> 24) & 0x0000000000FF0000ULL);
	n |= ((h >> 40) & 0x000000000000FF00ULL);
	n |= ((h >> 56) & 0x00000000000000FFULL);
	return n;
}

/** 长整型转主机字节序(假设主机是小头在前). */
ullong Net::ntohll(ullong n)
{
	return Net::htonll(n);
}

/** 关闭套接字. */
void Net::close(int sock)
{
#ifdef WIN32
	::closesocket(sock);
#else
#ifdef ANDROID	/** 在Android系统中测试时, 发现仅仅调用close函数无法使阻塞的recv函数返回. */
	::shutdown(sock, SHUT_RDWR);
#endif
	::close(sock);
#endif
}

Net::~Net()
{

}
