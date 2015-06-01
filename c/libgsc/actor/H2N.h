/*
 * H2N.h
 *
 *  Created on: Jan 31, 2015
 *      Author: root
 */

#ifndef H2N_H_
#define H2N_H_

#if !defined (__LIBGSCH__) && !defined (LIBGSC)
#error only libgsc.h can be included directly.
#endif

#include "ActorNet.h"
class Cb;
class Gh2ntrans;

#define CAST_H2N_UNI_CB(x)																	(void (*)(H2N* h2n, Message* uni, string* ext))(x)
#define REG_H2N_NOGUSR(___CMD___, ___BEGIN___, ___END___) 									(this->regEvn(___CMD___, ___BEGIN___::descriptor(), ___END___::descriptor(), NULL, NULL, false));
#define REG_H2N_ONGUSR(___CMD___, ___BEGIN___, ___END___) 									(this->regEvn(___CMD___, ___BEGIN___::descriptor(), ___END___::descriptor(), NULL, NULL, true));
#define REG_H2N_NOGUSR_UNI(___CMD___, ___UNI___, ___CB___)			 						(this->regEvn(___CMD___, NULL, NULL, ___UNI___::descriptor(), (CAST_H2N_UNI_CB(___CB___)), false));
#define REG_H2N_ONGUSR_UNI(___CMD___, ___UNI___, ___CB___)			 						(this->regEvn(___CMD___, NULL, NULL, ___UNI___::descriptor(), (CAST_H2N_UNI_CB(___CB___)), true));

class H2N: public ActorNet
{
private:
	uint tid; /** 事务ID发生器. */
	unordered_map<uint, Gh2ntrans*> gts; /** 缓存的待出栈和已出栈但未收到END的BEGIN. */
	unordered_map<ushort, Cb*> evns; /** H2N下行消息(总是由H2N发起的BEGIN). */
	unordered_map<ushort, Cb*> nots; /** H2N上行通知消息回调. */
	ullong hbts; /** 上次发送心跳的时间戳. */
protected:
	bool ready; /** 是否鉴权通过, 由应用层设置. */
private:
	static void* svc(void* arg);
	void checkTrans(ullong now); /** 检查超时的事务. */
	void checkHeartBeat(ullong now); /** 检查心跳. */
private:
	bool evnStmpEnd(stmp_node* root); /** END. */
	bool evnStmpEndNoGusr(stmp_node* root); /** END(鉴权通过前). */
	bool evnStmpEndOnGusr(stmp_node* root); /** END(鉴权通过后). */
	bool evnStmpUni(stmp_node* root); /** UNI. */
	bool evnStmpUniNoGusr(stmp_node* root); /** UNI(鉴权通过前). */
	bool evnStmpUniOnGusr(stmp_node* root); /** UNI(鉴权通过后). */
public:
	H2N(const char* host, int port, const char* name);
	virtual ~H2N();
	virtual void estb() = 0; /** 连接已建立. */
	virtual void disc() = 0; /** 连接已失去. */
	virtual void regCb() = 0; /** 注册网络事件. */
	virtual uchar* encryp(uchar* dat, uint len, uint* out) = 0; /** 消息出栈时的加密函数. */
	virtual uchar* decryp(uchar* dat, uint len, uint* out) = 0; /** 消息入栈时的解密函数. */
	virtual void heartbeatReq(ushort* cmd, Message** begin) = 0; /** 心跳请求. */
	virtual void heartbeatRsp(ushort ret, Message* end, bool tm /** 是否超时. */) = 0; /** 心跳响应. */
	void future(ushort cmd, Message* begin, function<void(ushort ret, Message* end, string* ext)> end, function<void()> tm, string* extbegin = NULL); /** 开启一个H2N事务. */
	uint nexTid(); /** 获得一个事务ID. */
public:
	void connect(); /** 尝试向远端发起连接. */
	void evnDis(); /** 连接断开事件. */
	bool evnMsg(stmp_node* root); /** STMP消息到来. */
	void check(ullong now); /** 做两件事, 1, 检查超时的事务. 2, 决定是否要发心跳. */
	void sendBegin(Gh2ntrans* gt); /** 尝试向remote发送一个STMP-BEGIN, 并缓存H2N事务. */
	void sendUni(ushort cmd, Message* uni, string* ext = NULL); /** 尝试向remote发送一个STMP-UNI. */
	bool regEvn(ushort cmd, const Descriptor* begin, const Descriptor* end, const Descriptor* uni, void (*cb)(H2N* h2n, Message* uni, string* ext), bool gusr); /** 注册消息回调. */
	string toString();
};

#endif /* H2N_H_ */
