/*
 * Gusr.h
 *
 *  Created on: Jan 31, 2015
 *      Author: root
 */

#ifndef GUSR_H_
#define GUSR_H_

#if !defined (__LIBGSCH__) && !defined (LIBGSC)
#error only libgsc.h can be included directly.
#endif

#include "../actor/Actor.h"
#include "../core/Gn2htrans.h"
#include "../core/Gn2h.h"

class Gusr: public Actor
{
private:
	stmp_pdu* pkgUni(ushort cmd, Message* uni, string* ext); /** 构造一个加密过的UNI消息. */
public:
	ullong uid; /** 应用id. */
	N2H* n2h; /** 关联的连接.*/
public:
	Gusr(ullong uid, const char* name);
	virtual ~Gusr();
	virtual void evnDis() = 0; /** 关联的N2H连接断开. */
	virtual uchar* encryp(uchar* dat, uint len, uint* out) = 0; /** 消息出栈时的加密函数. */
	virtual uchar* decryp(uchar* dat, uint len, uint* out) = 0; /** 消息入栈时的解密函数. */
public:
	void sendEnd0(Gn2htrans* gt); /** 加密, 并将事务前转到自身关联的N2H上出栈(长连接). */
	void sendEnd1(Gn2h* gt); /** 加密, 并将事务前转到自身关联的N2H上出栈(短连接). */
	void sendUni(ushort cmd, Message* uni, string* ext = NULL); /** 加密, 并将消息前转到自身关联的N2H上出栈. */
	void procBegin0(uchar* dat, uint len, Gn2htrans* gt); /** 处理长连接上的STMP-BEGIN. */
	void procBegin1(uchar* dat, uint len, Gn2h* gt, ullong sid); /** 处理短连接上的STMP-BEGIN. */
	void procUni(uchar* dat, uint len); /** 处理连接上的STMP-UNI, 只有长连接支持STMP-UNI. */
	bool isOnline(); /** 网络连接是否正常. */
	void kick(ushort cmd, Message* uni); /** 踢下线(将导致连接被延迟关闭). */
	void kickNoMsg(); /** 踢下线, 无消息提示(连接被立即关闭). */
};

#endif /* GUSR_H_ */
