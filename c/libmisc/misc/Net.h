/*
 * Net.h
 *
 *  Created on: 2013-4-26
 *      Author: xuzewen
 */

#ifndef NET_H_
#define NET_H_

#if !defined (__LIBMISC_H__) && !defined (LIBMISC)
#error only libmisc.h can be included directly.
#endif

#include "../macro.h"

class Net
{
private:
	Net();
	virtual ~Net();
public:
	static void ip2str(uint ip, char* str); /** 返回整形ipv4地址的点分十进制字符串表现形式. */
	static string ip2str(uint ip); /** 返回整形ipv4地址的点分十进制字符串表现形式. */
	static bool isDomain(const char* host); /** 是否为域名. */
	static void sockaddr2str(struct sockaddr_in* addr, char* str); /** 返回socket地址描述的字符串表现形式(点分十进制:端口)*/
	static string sockaddr2str(struct sockaddr_in* addr); /** 返回socket地址描述的字符串表现形式(点分十进制:端口)*/
	static void getHostByName(const char* domain, string* ip); /** 通过域名, 获得一个ip. */
	static int tcpConnect(const char* host, ushort port); /** 尝试向host:port建立一个TCP连接, 返回-1时表示失败. */
	static int tcpConnectNoBlocking(const char* host, ushort port, uint sec /** 超时时间(秒). */, uint rcvbuf = 512 * 1024, uint sndbuf = 512 * 1024); /** 非阻塞的TCP连接. */
	static int tcpListen(const char* host, ushort port); /** 尝试在指定的本地地址和端口上建立一个TCP监听. 如果成功, 返回套接字句柄, 否则返回 -1. */
	static int tcpAccept(int sock, struct sockaddr* peer); /** 在服务端套接字上等待一个tcp连接(阻塞), 成功时返回与peer相关的套接字句柄, 否则返回与平台有关的accept返回值. */
	static int tcpRecv(int sock, uchar* buff, int size); /** 尝试在TCP套接字上接收一段数据. sock应该是阻塞模式的. */
	static int tcpRecvNoBlocking(int sock, uchar* buff, int size); /** 尝试在TCP套接字上接收一段数据. sock应该是非阻塞模式的. */
	static bool tcpRecvN(int sock, uchar* buff, int size); /** 尝试在TCP套接字上接收一段数据, sock应该是阻塞模式的, 除非套接字异常, 函数总是返回size个字节. */
	static int tcpSend(int sock, uchar* buff, int size); /** 尝试往TCP套接字上发送一段数据, 返回值依赖于sock本身的特性.  */
	static int udpBind0(const char* host, ushort port, struct sockaddr_in* addr); /** 以非阻塞模式绑定本地UDP端口, 并设置了大约512K的R/S缓冲区, 如果不关心本地addr, 可置为NULL. */
	static int udpBind1(const char* host, ushort port, struct sockaddr_in* addr); /** 以阻塞模式绑定本地UDP端口, 并设置了大约512K的R/S缓冲区, 如果不关心本地addr, 可置为NULL. */
	static int udpRecv(int sock, uchar* buff, int size, struct sockaddr_in* from); /** 尝试在sock上接收一段报文, 远端地址被填充在from中.  */
	static int udpSend(int sock, uchar* buff, int size, struct sockaddr_in* to); /** 向指定的地址发送一段UPD报文.  */
	static bool setNoBlocking(int sock); /** 设置套接字为非阻塞. */
	static bool setLinger(int sock); /** 简单粗暴的套接字关闭类型. */
	static bool setSNDBUF(int sock, int size); /** 设置套接字发送缓冲区. */
	static bool setRCVBUF(int sock, int size); /** 设置套接字接收缓冲区. */
	static bool setSNDTIMEO(int sock, int sec); /** 调置套接字的发送超时. */
	static bool setRCVTIMEO(int sock, int sec); /** 调置套接字的接收超时. */
	static bool setNODELAY(int sock); /** 设置TCP_NODELAY选项. */
	static void setAddr(const char* host, int port, struct sockaddr_in* addr); /** 将host和port设置到addr. */
	static void bigEndianInt(uint x, uchar* dat); /** 大端在前. */
	static void litEndianInt(uint x, uchar* dat); /** 小端在前. */
	static ullong htonll(ullong h); /** 长整型转网络字节序(假设主机是小头在前). */
	static ullong ntohll(ullong n); /** 长整型转主机字节序(假设主机是小头在前). */
	static void close(int sock); /** 关闭套接字. */
};

#endif /* NET_H_ */
