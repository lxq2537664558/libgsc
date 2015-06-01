/*
 * H2N.cpp
 *
 *  Created on: Jan 31, 2015
 *      Author: root
 */

#include "H2N.h"
#include "../core/Gsc.h"
#include "../core/Gh2ntrans.h"
#include "../core/Cfg.h"
#include "../core/Cb.h"
#include "../net/Lx.h"

H2N::H2N(const char* host, int port, const char* name) :
		ActorNet(ACTOR_H2N, name, Gsc::rrWk() % Cfg::libgsc_worker /** 轮询工作线程. */)
{
	Net::setAddr(host, port, &this->peer);
	this->est = false;
	this->ready = false;
	this->tid = 0xFEEDBEEE;
	this->hbts = 0ULL;
}

/** 连接断开事件. */
void H2N::evnDis()
{
	this->ready = false;
	this->est = false;
	this->disc();
	this->connect();
}

/** STMP消息到来, 对于H2N连接, libgsc只支持END和UNI. */
bool H2N::evnMsg(stmp_node* root)
{
	switch (root->self.t)
	{
	case STMP_TAG_TRANS_END:
		return this->evnStmpEnd(root);
	case STMP_TAG_TRANS_UNI:
		return this->evnStmpUni(root);
	default:
		LOG_DEBUG("unsupported STMP transaction: %02X", root->self.t)
		return false;
	}
}

/** END. */
bool H2N::evnStmpEnd(stmp_node* root)
{
	return this->ready ? this->evnStmpEndOnGusr(root) : this->evnStmpEndNoGusr(root);
}

/** END(鉴权通过前). */
bool H2N::evnStmpEndNoGusr(stmp_node* root)
{
	uint tid;
	if (stmpdec_get_int(root, STMP_TAG_DTID, &tid) != 0)
	{
		LOG_DEBUG("missing required field: STMP_TAG_DTID")
		return false;
	}
	ushort ret;
	if (stmpdec_get_short(root, STMP_TAG_RET, &ret) != 0)
	{
		LOG_DEBUG("missing required field: STMP_TAG_RET")
		return false;
	}
	auto it = this->gts.find(tid);
	if (it == this->gts.end()) /** 找不到事务, 可能是已超时. */
	{
		LOG_WARN("can not found Gh2ntrans for tid: %08X, may be it was timeout.", tid);
		return true;
	}
	Gh2ntrans* gt = it->second;
	gt->ret = ret;
	this->gts.erase(it); /** 弹出事务. */
	auto iter = this->evns.find(gt->cmd);
	if (iter == this->evns.end()) /** 找不到回调. */
	{
		LOG_FAULT("it`s a bug, cmd: %04X", gt->cmd);
		delete gt;
		return false;
	}
	Cb* cb = iter->second;
	if (cb->end == NULL)
	{
		LOG_DEBUG("it`s an unexpected cmd: %04X", cb->cmd)
		delete gt;
		return false;
	}
	uint len;
	uchar* pb = stmpdec_peek_bin(root, STMP_TAG_DAT, &len);
	if (pb != NULL)
		gt->end = Gsc::newPbMsg(cb->end, pb, len);
	uchar* att = stmpdec_peek_bin(root, STMP_TAG_ATT, &len);
	if (att != NULL)
		gt->extend = new string((char*) att, len);
	LOG_TRACE("\n  <-- END(%s): {%s}, BEGIN(%s): {%s}, RET: %04X, CMD: %04X, TID: %08X, PEER: %s\n", //
			cb->end->name().c_str(), gt->end == NULL ? "NULL": gt->end->ShortDebugString().c_str(),//
			cb->begin->name().c_str(), gt->begin == NULL ? "NULL": gt->begin->ShortDebugString().c_str(),//
			gt->ret, gt->cmd, gt->tid, Net::sockaddr2str(&this->peer).c_str())
	gt->endCb(gt->ret, gt->end, gt->extend);
	if (Gsc::ec)
		Gsc::ec->gh2nTrans(gt); /** 事务输出. */
	delete gt;
	return true;
}

/** END(鉴权通过后). */
bool H2N::evnStmpEndOnGusr(stmp_node* root)
{
	uint tid;
	if (stmpdec_get_int(root, STMP_TAG_DTID, &tid) != 0)
	{
		LOG_DEBUG("missing required field: STMP_TAG_DTID")
		return false;
	}
	auto it = this->gts.find(tid);
	if (it == this->gts.end()) /** 找不到事务, 可能是已超时. */
	{
		LOG_WARN("can not found Gh2ntrans for tid: %08X, may be it was timeout.", tid);
		return true;
	}
	uint len;
	uchar* sec = stmpdec_peek_bin(root, STMP_TAG_DAT, &len);
	if (sec == NULL)
	{
		LOG_WARN("missing required field: STMP_TAG_DAT.")
		return false;
	}
	uint size;
	uchar* dat = this->decryp(sec, len, &size); /** 解密. */
	stmp_node node;
	if (stmpdec_unpack(dat, size, &node) != 0)
	{
		LOG_DEBUG("STMP protocol error.")
		free(dat);
		return false;
	}
	ushort ret;
	if (stmpdec_get_short(&node, STMP_TAG_RET, &ret) != 0)
	{
		LOG_DEBUG("missing required field: STMP_TAG_RET")
		loop: //
		free(dat);
		stmpdec_free(&node);
		return false;
	}
	Gh2ntrans* gt = it->second;
	gt->ret = ret;
	this->gts.erase(it); /** 弹出事务. */
	auto iter = this->evns.find(gt->cmd);
	if (iter == this->evns.end()) /** 找不到回调. */
	{
		LOG_FAULT("it`s a bug, cmd: %04X", gt->cmd);
		delete gt;
		goto loop;
	}
	Cb* cb = iter->second;
	if (cb->end == NULL)
	{
		LOG_DEBUG("it`s an unexpected cmd: %04X", cb->cmd)
		delete gt;
		goto loop;
	}
	uchar* pb = stmpdec_peek_bin(&node, STMP_TAG_DAT, &len);
	if (pb != NULL)
		gt->end = Gsc::newPbMsg(cb->end, pb, len);
	uchar* att = stmpdec_peek_bin(&node, STMP_TAG_ATT, &len);
	if (att != NULL)
		gt->extend = new string((char*) att, len);
	LOG_TRACE("\n  <-- END(%s): {%s}, BEGIN(%s): {%s}, RET: %04X, CMD: %04X, TID: %08X, PEER: %s\n", //
			cb->end->name().c_str(), gt->end == NULL ? "NULL": gt->end->ShortDebugString().c_str(),//
			cb->begin->name().c_str(), gt->begin == NULL ? "NULL": gt->begin->ShortDebugString().c_str(),//
			gt->ret, gt->cmd, gt->tid, Net::sockaddr2str(&this->peer).c_str())
	gt->endCb(gt->ret, gt->end, gt->extend);
	if (Gsc::ec)
		Gsc::ec->gh2nTrans(gt); /** 事务输出. */
	delete gt;
	free(dat);
	stmpdec_free(&node);
	return true;
}

/** UNI. */
bool H2N::evnStmpUni(stmp_node* root)
{
	return this->ready ? this->evnStmpUniOnGusr(root) : this->evnStmpUniNoGusr(root);
}

/** UNI(鉴权通过前). */
bool H2N::evnStmpUniNoGusr(stmp_node* root)
{
	ushort cmd;
	if (stmpdec_get_short(root, STMP_TAG_CMD, &cmd) != 0)
	{
		LOG_DEBUG("missing required field: STMP_TAG_CMD")
		return false;
	}
	auto it = this->nots.find(cmd);
	if (it == this->nots.end()) /** 找不到回调. */
	{
		LOG_WARN("can not found call back for this cmd: %04X", cmd); /* 不支持的命令字. */
		return false;
	}
	Cb* cb = it->second;
	if (cb->uni == NULL || cb->cb == NULL)
	{
		LOG_DEBUG("it`s an unexpected cmd: %04X", cmd)
		return false;
	}
	Message* uni = NULL;
	string* ext = NULL;
	uint len;
	uchar* dat = stmpdec_peek_bin(root, STMP_TAG_DAT, &len);
	if (dat != NULL)
		uni = Gsc::newPbMsg(cb->uni, dat, len);
	uchar* att = stmpdec_peek_bin(root, STMP_TAG_ATT, &len);
	if (att != NULL)
		ext = new string((char*) att, len);
	//
	LOG_TRACE("\n  <-- UNI(%s): {%s}, CMD: %04X, PEER: %s\n", cb->uni->name().c_str(), uni == NULL ? "NULL": uni->ShortDebugString().c_str(), cb->cmd, Net::sockaddr2str(&this->peer).c_str())
	((void (*)(H2N* h2n, Message* uni, string* ext)) (cb->cb))(this, uni, ext);
	if (Gsc::ec != NULL)
		Gsc::ec->gh2nRecvUni(this, cb->cmd, uni, ext);
	//
	if (uni != NULL)
		delete uni;
	if (ext != NULL)
		delete ext;
	return true;
}

/** UNI(鉴权通过后). */
bool H2N::evnStmpUniOnGusr(stmp_node* root)
{
	uint len;
	uchar* sec = stmpdec_peek_bin(root, STMP_TAG_DAT, &len);
	if (sec == NULL)
	{
		LOG_WARN("missing required field: STMP_TAG_DAT.")
		return false;
	}
	uint size;
	uchar* dat = this->decryp(sec, len, &size); /** 解密. */
	stmp_node node;
	if (stmpdec_unpack(dat, size, &node) != 0)
	{
		LOG_DEBUG("STMP protocol error.")
		free(dat);
		return false;
	}
	bool ret = this->evnStmpUniNoGusr(&node);
	free(dat);
	stmpdec_free(&node);
	return ret;
}

/** 尝试向remote发送一个STMP-BEGIN, 并缓存H2N事务. */
void H2N::sendBegin(Gh2ntrans* gt)
{
	if (Logger::isTrace())
	{
		auto it = this->evns.find(gt->cmd);
		if (it == this->evns.end())
		{
			LOG_FAULT("it`s a bug, cmd: %04X", gt->cmd)
			delete gt;
			return;
		}
		Cb* cb = it->second;
		LOG_TRACE("\n  --> BEGIN(%s): {%s}, END(%s), CMD: %04X, TID: %08X, PEER: %s\n", //
				cb->begin->name().c_str(), gt->begin == NULL ? "NULL": gt->begin->ShortDebugString().c_str(),//
				cb->end->name().c_str(), gt->cmd, gt->tid, Net::sockaddr2str(&this->peer).c_str())
	}
	this->gts[gt->tid] = gt; /** 总是缓存. */
	if (!this->est) /** 连接还未建立. */
		return;
	if (!this->ready) /** 还未鉴权通过. */
	{
		stmp_pdu* sp = Gsc::pkgBegin(gt->cmd, gt->tid, gt->begin, gt->extbegin);
		uint len;
		uchar* dat = stmpenc_take(sp, &len);
		this->send(dat, len);
		free(sp->buff);
		free(sp);
		return;
	}
	stmp_pdu* sp = Gsc::pkgBeginSecDat(gt->cmd, gt->begin, gt->extbegin);
	uint len;
	uchar* dat = stmpenc_take(sp, &len);
	uint out;
	uchar* cryp = this->encryp(dat, len, &out);
	free(sp->buff);
	free(sp);
	//
	stmp_pdu* begin = (stmp_pdu*) malloc(sizeof(stmp_pdu));
	int size = out;
	size = size + (8 - (size % 8)) + 64; /** 只多不少. */
	begin->len = size;
	begin->rm = size;
	begin->p = 0;
	begin->buff = (uchar*) malloc(size);
	stmpenc_add_bin(begin, STMP_TAG_DAT, cryp, out);
	stmpenc_add_int(begin, STMP_TAG_STID, gt->tid);
	stmpenc_add_tag(begin, STMP_TAG_TRANS_BEGIN);
	dat = stmpenc_take(begin, &len);
	//
	this->send(dat, len);
	//
	free(cryp);
	free(begin->buff);
	free(begin);
}

/** 尝试向remote发送一个STMP-UNI. */
void H2N::sendUni(ushort cmd, Message* uni, string* ext)
{
	//未实现.
	if (Gsc::ec)
		Gsc::ec->gh2nSendUni(this, cmd, uni, ext);
}

/** 做两件事, 1, 检查超时的事务. 2, 决定是否要发心跳. */
void H2N::check(ullong now)
{
	this->checkTrans(now);
	this->checkHeartBeat(now);
}

/** 检查超时的事务. */
void H2N::checkTrans(ullong now)
{
	for (auto it = this->gts.begin(); it != this->gts.end();)
	{
		Gh2ntrans* gt = it->second;
		if (now - gt->gts >= Cfg::libgsc_h2n_trans_timeout) /** 超时. */
		{
			this->gts.erase(it++);
			gt->tm = true;
			LOG_WARN("have a H2N transaction timeout, elap: %llu, tid: %08X", now - gt->gts, gt->tid)
			gt->tmCb();
			//
			if (Gsc::ec)
				Gsc::ec->gh2nTrans(gt); /** 事务输出. */
			delete gt;
		} else
			++it;
	}
}

/** 检查心跳. */
void H2N::checkHeartBeat(ullong now)
{
	if (!this->ready) /** 鉴权未通过, 不发送心跳. */
		return;
	if (now - this->hbts < Cfg::libgsc_peer_heartbeat)
		return;
	this->hbts = now;
	ushort cmd;
	Message* msg = NULL;
	this->heartbeatReq(&cmd, &msg);
	this->future(cmd, msg, [this, cmd](ushort ret, Message* end, string* ext)
	{
		this->heartbeatRsp(ret, end, false);
	},
	/** timeout. */
	[this]
	{
		this->heartbeatRsp(RET_FAILURE, NULL, true);
		this->close();
	});
}

/** 尝试向远端发起连接. */
void H2N::connect()
{
	if (this->est)
		return;
	this->ref();
	pthread_t t;
	if (pthread_create(&t, NULL, H2N::svc, this) != 0)
		LOG_FAULT("no more thread can be create, peer: %s", Net::sockaddr2str(&this->peer).c_str())
	pthread_detach(t);
}

void* H2N::svc(void* arg)
{
	static bool first = true;
	//
	H2N* h2n = (H2N*) arg;
	uint ip = ntohl(h2n->peer.sin_addr.s_addr);
	string str;
	SPRINTF_STRING(&str, "%d.%d.%d.%d", (ip >> 24 & 0xFF), (ip >> 16 & 0xFF), (ip >> 8 & 0xFF), (ip & 0xFF))
	while (1)
	{
		if (first) /* 仅第一次无需等待. */
			first = false;
		else
			Misc::sleep(Cfg::libgsc_h2n_reconn);
		//
		int cfd = Net::tcpConnect(str.c_str(), ntohs(h2n->peer.sin_port));
		if (cfd < 1)
		{
			LOG_WARN("can not connect to remote-addr: %s", Net::sockaddr2str(&h2n->peer).c_str())
			continue;
		}
		//
		((Actor*) h2n)->future([h2n, cfd]
		{
			h2n->cfd = cfd;
			h2n->est = true;
			Lx::setFdAtt(cfd);
			Lx::getGwk()->addActorNet(h2n);
			Lx::getGwk()->addCfd4Recv(cfd);
			h2n->estb();
			h2n->unRef();
		});
		break;
	}
	return NULL;
}

/** 注册消息回调. */
bool H2N::regEvn(ushort cmd, const Descriptor* begin, const Descriptor* end, const Descriptor* uni, void (*cb)(H2N* h2n, Message* uni, string* ext), bool gusr)
{
	if (this->evns.find(cmd) != Gsc::evns.end())
	{
		LOG_ERROR("duplicate cmd: %04X, begin: %s, end: %s, uni: %s", cmd, begin == NULL ? "NULL" : begin->name().c_str(), end == NULL ? "NULL" : end->name().c_str(), uni == NULL ? "NULL" : uni->name().c_str())
		return false;
	}
	LOG_INFO("h2n-msg, cmd: %04X, begin: %s, end: %s, uni: %s, gusr: %s", cmd, begin == NULL ? "NULL" : begin->name().c_str(), end == NULL ? "NULL" : end->name().c_str(), uni == NULL ? "NULL" : uni->name().c_str(), gusr ? "true" : "false")
	this->evns[cmd] = new Cb(cmd, (void*) cb, begin, end, uni, gusr);
	return true;
}

/** 开启一个H2N事务. */
void H2N::future(ushort cmd, Message* begin, function<void(ushort ret, Message* end, string* ext)> end, function<void()> tm, string* extbegin)
{
	Gh2ntrans* gt = new Gh2ntrans(this, cmd, begin, end, tm, extbegin);
	Actor::future([this, gt]
	{
		this->ref();
		gt->tid = this->nexTid();
		this->sendBegin(gt);
	});
}

uint H2N::nexTid()
{
	return ++this->tid;
}

string H2N::toString()
{
	string str;
	SPRINTF_STRING(&str, "H2N(%s), rc: %d, rf: %s, wk: %d, peer: %s, est: %s, cfd: %d, dlen: %d, ready: %s, tid: %08X, gts: %lu", //
			this->name.c_str(), this->rc, this->rf ? "true" : "false", this->wk, //
			Net::sockaddr2str(&this->peer).c_str(), this->est ? "true" : "false", //
			this->cfd, this->dlen, this->ready ? "true" : "false", this->tid, this->gts.size())
	return str;
}

H2N::~H2N()
{

}

