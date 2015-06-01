/*
 * HttpH2n.h
 *
 *  Created on: Apr 2, 2015 4:54:21 PM
 *      Author: xuzewen
 */

#ifndef HTTPH2N_H_
#define HTTPH2N_H_

#if !defined (__LIBGSCH__) && !defined (LIBGSC)
#error only libgsc.h can be included directly.
#endif

#include "ActorNet.h"

class HttpH2n: public ActorNet
{
private:
	bool sendHttpRequest(); /** 发送一个HTTP请求. */
public:
	function<void(string* rsp)> rspCb;
	function<void()> failCb;
	function<void()> tmCb;
	string req; /** 请求内容. */
	string host; /** 主机名. */
	bool send; /** 是否已发送请求. */
	ullong tts; /** 超时时间, 这是一个未来的时间, 也就是到了这个时间就超时(毫秒). */
public:
	HttpH2n(const char* host, int port, int sec = 5 /** 从连接到响应的时间限制(秒). */);
	virtual ~HttpH2n();
public:
	void evnSend(); /** 写事件. */
	void evnDis(); /** 连接已失去. */
	bool evnMsg4Stmp(stmp_node* root);
	bool evnMsg4Http(uchar* dat, uint len); /** HTTP响应. */
	string toString();
public:
	void future(const char* req /** 包括头和正文. */, function<void(string * rsp)> rsp, function<void()> fail, function<void()> tm); /** 发起一个HTTP请求. */
};

#endif /* HTTPH2N_H_ */
