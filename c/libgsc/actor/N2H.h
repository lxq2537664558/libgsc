/*
 * N2H.h
 *
 *  Created on: Jan 31, 2015
 *      Author: root
 */

#ifndef N2H_H_
#define N2H_H_

#if !defined (__LIBGSCH__) && !defined (LIBGSC)
#error only libgsc.h can be included directly.
#endif

#include "ActorNet.h"
class Gusr;
class Gn2htrans;

class N2H: public ActorNet
{
public:
	bool lz; /** 是否要延迟关闭. */
	Gusr* gusr; /** 用户层数据. */
	ullong gts; /** 连接产生的时间戳.*/
private:
	bool evnBegin(stmp_node* root); /** BEGIN. */
	bool evnBeginNoGusr(stmp_node* root); /** 基于STMP的长连接请求(鉴权通过前). */
	bool evnBeginOnGusr(stmp_node* root); /** 基于STMP的长连接请求(鉴权通过后). */
	bool evnBeginTmpConn(stmp_node* root, ullong uid, ullong sid); /** 基于STMP的短连接请求. */
	bool evnUni(stmp_node* root); /** UNI. */
	bool evnUniNoGusr(stmp_node* root); /** UNI(鉴权通过前). */
	bool evnUniOnGusr(stmp_node* root); /** UNI(鉴权通过后). */
public:
	N2H(uint cfd, int wk, struct sockaddr_in* peer);
	virtual ~N2H();
public:
	void evnDis(); /** 连接断开, 尝试delete this. */
	bool evnMsg(stmp_node* root); /** STMP消息到来, 对于N2H连接, libgsc只支持BEGIN和UNI, 原因是libgsc不会在N2H连接上发送请求. */
public:
	void sendEnd(Gn2htrans* gt); /** 当N2H与Gusr无关联时的STMP-END消息. */
	void sendUni(ushort cmd, Message* uni, string* ext = NULL); /** 当N2H与Gusr无关联时的STMP-UNI消息. */
	void lazyClose(); /** 延迟关闭. */
	string toString();
};

#endif /* N2H_H_ */
