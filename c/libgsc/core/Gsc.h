/*
 * Gsc.h
 *
 *  Created on: 2012-7-30
 *      Author: xuzewen
 */

#ifndef GSC_H_
#define GSC_H_

#if !defined (__LIBGSCH__) && !defined (LIBGSC)
#error only libgsc.h can be included directly.
#endif

#include "../actor/ActorGusrDb.h"
#include "../actor/HttpH2n.h"
#include "Gstat.h"
#include "Cb.h"
#include "EvnCb.h"

#define REG_N2H_NOGUSR(___CMD___, ___BEGIN___, ___END___, ___CB___)					    		(Gsc::regN2HEvn(___CMD___, ___BEGIN___::descriptor(), ___END___::descriptor(), NULL, (void*)(___CB___), false));
#define REG_N2H_ONGUSR(___CMD___, ___BEGIN___, ___END___, ___CB___)					    		(Gsc::regN2HEvn(___CMD___, ___BEGIN___::descriptor(), ___END___::descriptor(), NULL, (void*)(___CB___), true));
#define REG_N2H_UNI_NOGUSR(___CMD___, ___UNI___, ___CB___)		    							(Gsc::regN2HEvn(___CMD___, NULL, NULL, ___UNI___::descriptor(), (void*)(___CB___), false));
#define REG_N2H_UNI_ONGUSR(___CMD___, ___UNI___, ___CB___)		    							(Gsc::regN2HEvn(___CMD___, NULL, NULL, ___UNI___::descriptor(), (void*)(___CB___), true));
#define REG_TMPN2H(___CMD___, ___BEGIN___, ___END___, ___CB___)							    	(Gsc::regN2HEvn(___CMD___, ___BEGIN___::descriptor(), ___END___::descriptor(), NULL, (void*)(___CB___), true));
//
#define LIBGSC_SERVER_HOST																		"LIBGSC_SERVER_HOST" /** 服务地址. */
#define LIBGSC_SERVER_PORT																		"LIBGSC_SERVER_PORT" /** 服务端口. */
#define LIBGSC_WORKER																			"LIBGSC_WORKER" /** 消息总线线程数. */
#define LIBGSC_PEER_LIMIT																		"LIBGSC_PEER_LIMIT" /** 允许的最大连接数. */
#define LIBGSC_PEER_MTU																			"LIBGSC_PEER_MTU" /** 上行报文尺寸限制. */
#define LIBGSC_PEER_RCVBUF																		"LIBGSC_PEER_RCVBUF" /** 连接上的接收缓冲区尺寸. */
#define LIBGSC_PEER_SNDBUF																		"LIBGSC_PEER_SNDBUF" /** 连接上的发送缓冲区尺寸. */
#define LIBGSC_PEER_HEARTBEAT																	"LIBGSC_PEER_HEARTBEAT" /** 连接上的心跳间隔(毫秒), 同时用于H2N的心跳发送和N2H的心跳检测. */
#define LIBGSC_N2H_ZOMBIE																		"LIBGSC_N2H_ZOMBIE" /** 当一个N2H连接上来后, 等待其第一个消息的时间(毫秒), 超过此时间连接将被强制断开. */
#define LIBGSC_N2H_LAZY_CLOSE																	"LIBGSC_N2H_LAZY_CLOSE" /** libgsc上所有连接都是linger(-1), 因此引入此选项, 用于N2H套接字延迟关闭(毫秒). */
#define LIBGSC_H2N_RECONN																		"LIBGSC_H2N_RECONN" /** H2N重连等待(毫秒). */
#define LIBGSC_H2N_TRANS_TIMEOUT																"LIBGSC_H2N_TRANS_TIMEOUT" /** H2N网络事务超时时间(毫秒), 即发送请求后等待响应的时间. */
#define LIBGSC_QUARTZ																			"LIBGSC_QUARTZ" /** 定时器精度(毫秒). */
//
#define LIBGSC_LOG_HOST																			"LIBGSC_LOG_HOST"	/** 日志服务器地址. */
#define LIBGSC_LOG_PORT																			"LIBGSC_LOG_PORT"	/** 日志服务器端口. */
#define LIBGSC_LOG_LEVEL																		"LIBGSC_LOG_LEVEL"	/** 日志级别. */

class Gsc
{
private:
	static uint rr;
public:
	static EvnCb* ec; /** 事务结束回调. */
	static vector<ActorGusrDb*> db; /** Gusr数据库(适用于短连接业务). */
	static unordered_map<ushort, Cb*> evns; /** N2H上行业务消息回调. */
private:
	Gsc();
	virtual ~Gsc();
public:
	static bool init(); /** 初始化libgsc消息总线. */
	static bool publish(); /** 开启libgsc服务端口. */
	static void hold(); /** 定时器振荡. */
	static void regEvnCb(EvnCb* ec); /** 注册EvnCb. */
	static bool regN2HEvn(ushort cmd, const Descriptor* begin, const Descriptor* end, const Descriptor* uni, void* cb, bool gusr); /** 注册N2H上的消息回调. */
	static bool regN2HTmpEvn(ushort cmd, const Descriptor* begin, const Descriptor* end, const Descriptor* uni, void* cb); /** 注册N2H上的消息回调(适用于短连接). */
	static void addActorGusrDb(ActorGusrDb* db); /** 注册Gusr数据库(适用于短连接业务). */
	static ActorGusrDb* getGusrDb(); /** 尽可能选择一个不忙的Gusr数据库处理Actor(适用于短连接业务). */
	static uint rrWk(); /** 为Actor选择一个工作线程(round-robin). */
	static uint hashWk(ullong id); /** 为Actor选择一个工作线程(散列). */
	static stmp_pdu* pkgBeginSecDat(const ushort cmd, const Message* begin, const string* ext); /** 打包一个STMP-BEGIN中的加密部分. */
	static stmp_pdu* pkgBegin(const ushort cmd, const uint tid, const Message* begin, const string* ext); /** 打包一个STMP-BEGIN. */
	static stmp_pdu* pkgEndSecDat(const ushort ret, const Message* end, const string* ext); /** 打包一个STMP-END中的加密部分. */
	static stmp_pdu* pkgEnd(const uint tid, const ushort ret, const Message* end, const string* ext); /** 打包一个STMP-END. */
	static stmp_pdu* pkgUniSecDat(const ushort cmd, const Message* uni, const string* ext); /** 打包一个STMP-UNI中的加密部分. */
	static Cb* unpkgBegin0(stmp_node* root, uint* tid, ushort* cmd, Message** begin, string** ext); /** 解包一个STMP-BEGIN(适用于长连接). */
	static Cb* unpkgUni(stmp_node* root, ushort* cmd, Message** begin, string** ext); /** 解包一个STMP-UNI. */
	static Message* newPbMsg(const Descriptor* desc); /** 反射得到一个pb message对象. */
	static Message* newPbMsg(const Descriptor* desc, uchar* dat, int len); /** 反射得到一个pb message对象. */
};

#endif /* GSC_H_ */
