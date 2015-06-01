/*
 * N2H.cpp
 *
 *  Created on: Jan 31, 2015
 *      Author: root
 */

#include "N2H.h"
#include "../core/Gsc.h"
#include "../core/Gstat.h"
#include "../net/Lx.h"

N2H::N2H(uint cfd, int wk, sockaddr_in* peer) :
		ActorNet(ACTOR_N2H, "N2H", wk)
{
	this->lz = false;
	this->cfd = cfd;
	this->est = true;
	this->gusr = NULL;
	memcpy(&this->peer, peer, sizeof(struct sockaddr_in));
	this->gts = DateMisc::getMsec();
}

/** STMP消息到来, 对于N2H连接, libgsc只支持BEGIN和UNI, 原因是libgsc不会在N2H连接上发送请求. */
bool N2H::evnMsg(stmp_node* root)
{
	switch (root->self.t)
	{
	case STMP_TAG_TRANS_BEGIN:
		return this->evnBegin(root);
	case STMP_TAG_TRANS_UNI:
		return this->evnUni(root);
	default:
		LOG_DEBUG("unsupported STMP transaction: %02X", root->self.t)
		return false;
	}
}

/** BEGIN. */
bool N2H::evnBegin(stmp_node* root)
{
	ullong uid;
	if (stmpdec_get_long(root, STMP_TAG_UID, &uid) != 0) /** 无UID, 被认为是长连接. */
		return this->gusr == NULL ? this->evnBeginNoGusr(root) : this->evnBeginOnGusr(root);
	//
	ullong sid;
	if (stmpdec_get_long(root, STMP_TAG_SID, &sid) != 0) /** 否则就是短连接, 有UID时, 一定要有SID. */
	{
		LOG_DEBUG("missing required field: STMP_TAG_SID.")
		return false;
	}
	return this->evnBeginTmpConn(root, uid, sid);
}

/** 基于STMP的长连接请求(鉴权通过前). */
bool N2H::evnBeginNoGusr(stmp_node* root)
{
	uint tid;
	ushort cmd;
	Message* begin;
	string* ext;
	Cb* cb = Gsc::unpkgBegin0(root, &tid, &cmd, &begin, &ext);
	if (cb == NULL)
		return false;
	if (cb->gusr != false || cb->cb == NULL) /** 不应该在这里收到. */
	{
		LOG_DEBUG("it`s an unexpected cmd: %04X", cb->cmd)
		if (begin != NULL)
			delete begin;
		if (ext != NULL)
			delete ext;
		this->close();
	}
	if (Logger::isTrace())
	{
		LOG_TRACE("\n  <-- BEGIN(%s): {%s}, END(%s), CMD: %04X, TID: %08X\n", //
				cb->begin->name().c_str(), begin == NULL ? "NULL": begin->ShortDebugString().c_str(),//
				cb->end->name().c_str(), cmd, tid)
	}
	((void (*)(N2H* n2h, Gn2htrans* gt, Message* begin)) (cb->cb))(this, new Gn2htrans(this, cmd, tid, begin, ext), begin);
	return true;
}

/** 基于STMP的长连接请求(鉴权通过后). */
bool N2H::evnBeginOnGusr(stmp_node* root)
{
	uint tid;
	if (stmpdec_get_int(root, STMP_TAG_STID, &tid) != 0)
	{
		LOG_DEBUG("missing required field: STMP_TAG_STID")
		return false;
	}
	uint len;
	uchar* sec = stmpdec_peek_bin(root, STMP_TAG_DAT, &len);
	if (sec == NULL)
	{
		LOG_DEBUG("missing required field: STMP_TAG_DAT.")
		return false;
	}
	Gn2htrans* gt = new Gn2htrans(this, 0, tid, NULL, NULL); /** 未初始化. */
	uchar* cpy = (uchar*) malloc(len);
	memcpy(cpy, sec, len);
	Gusr* gusr = this->gusr;
	gusr->future([gusr, cpy, len, gt]
	{
		gusr->procBegin0(cpy, len, gt);
		free(cpy);
	});
	return true;
}

/** 基于STMP的短连接请求. */
bool N2H::evnBeginTmpConn(stmp_node* root, ullong uid, ullong sid)
{
	if (Gsc::db.size() == 0) /** 未注册短连接服务. */
	{
		LOG_DEBUG("no ActorGusrDb registered.")
		return false;
	}
	uint len;
	uchar* dat = stmpdec_peek_bin(root, STMP_TAG_DAT, &len);
	if (dat == NULL)
	{
		LOG_DEBUG("missing required field: STMP_TAG_DAT")
		return false;
	}
	ActorGusrDb* db = Gsc::getGusrDb();
	N2H* n2h = this;
	n2h->ref(); /** 增加一个引用, 避免被delete. */
	uchar* cpy = (uchar*) malloc(len); /** dat在栈上, 需copy. */
	memcpy(cpy, dat, len);
	db->future([db, n2h, uid, sid, cpy, len]
	{
		ullong csid;
		Gusr* gusr = db->refGusr(uid, &csid); /** 前往数据库查询. */
		if(gusr == NULL) /** 找不到用户数据. */
		{
			LOG_DEBUG("can not found Gusr for cid: %016llX", uid)
			free(cpy);
			n2h->future([n2h]
					{
						n2h->close();
						n2h->unRef(); /** 释放上面的引用. */
					});
		}
		else
		{
			gusr->future([gusr, n2h, cpy, db, sid, csid, len]
					{
						Gn2h* gt = new Gn2h(gusr, n2h, sid, 0, 0, NULL, NULL); /** 未初始化, Gn2h内部引用并释放Gusr, (上面调用了n2h->ref)释放n2h. */
						gusr->procBegin1(cpy, len, gt, csid);
						db->future([db, gusr]
								{
									db->unRefGusr(gusr); /** 处理完成后, 释放db对Gusr的引用. */
								});
						free(cpy);
					});
		}
	});
	return true;
}

/** UNI. */
bool N2H::evnUni(stmp_node* root)
{
	return this->gusr == NULL ? this->evnUniNoGusr(root) : this->evnUniOnGusr(root);
}

/** UNI(鉴权通过前). */
bool N2H::evnUniNoGusr(stmp_node* root)
{
	ushort cmd;
	if (stmpdec_get_short(root, STMP_TAG_CMD, &cmd) != 0)
	{
		LOG_DEBUG("missing required field: STMP_TAG_CMD")
		return false;
	}
	auto it = Gsc::evns.find(cmd);
	if (it == Gsc::evns.end()) /** 找不到回调. */
	{
		LOG_WARN("can not found call back for this cmd: %04X", cmd); /* 不支持的命令字. */
		return false;
	}
	Cb* cb = it->second;
	if (cb->cb == NULL)
	{
		LOG_DEBUG("it`s an unexpected cmd: %04X", cmd)
		return false;
	}
	Message* uni = NULL;
	string* ext = NULL;
	uint len;
	uchar* dat = stmpdec_peek_bin(root, STMP_TAG_DAT, &len);
	if (dat != NULL && cb->uni != NULL) /** 允许无pb部分. */
		uni = Gsc::newPbMsg(cb->uni, dat, len);
	uchar* att = stmpdec_peek_bin(root, STMP_TAG_ATT, &len);
	if (att != NULL)
		ext = new string((char*) att, len);
	//
	LOG_TRACE("\n  <-- UNI(%s): {%s}, CMD: %04X, PEER: %s\n", cb->uni == NULL ? "NULL" : cb->uni->name().c_str(), uni == NULL ? "NULL": uni->ShortDebugString().c_str(), cb->cmd, Net::sockaddr2str(&this->peer).c_str())
	((void (*)(N2H* n2h, Message* uni, string* ext)) (cb->cb))(this, uni, ext);
	if (Gsc::ec)
		Gsc::ec->gn2hRecvUni(this, cb->cmd, uni, ext);
	//
	if (uni != NULL)
		delete uni;
	if (ext != NULL)
		delete ext;
	return true;
}

/** UNI(鉴权通过后). */
bool N2H::evnUniOnGusr(stmp_node* root)
{
	uint len;
	uchar* sec = stmpdec_peek_bin(root, STMP_TAG_DAT, &len);
	if (sec == NULL)
	{
		LOG_DEBUG("missing required field: STMP_TAG_DAT.")
		return false;
	}
	uchar* cpy = (uchar*) malloc(len);
	memcpy(cpy, sec, len);
	this->gusr->future([this, cpy, len]
	{
		this->gusr->procUni(cpy, len);
		free(cpy);
	});
	return true;
}

/** 连接断开, 尝试delete this. */
void N2H::evnDis()
{
	this->est = false;
	LOG_TRACE("have a connection lost, cfd: %d, peer: %s", this->cfd, Net::sockaddr2str(&this->peer).c_str());
	if (this->gusr != NULL)
	{
		Gusr* gusr = this->gusr;
		this->gusr = NULL;
		this->unRef(); /** 如果gusr不为NULL, 则一定存在相互引用. */
		gusr->future([gusr]
		{
			gusr->n2h = NULL;
			gusr->evnDis();
			gusr->unRef();
		});
	}
	this->del();
}

/** 当N2H与Gusr无关联时的STMP-END消息. */
void N2H::sendEnd(Gn2htrans* gt)
{
	if (Logger::isTrace())
	{
		auto it = Gsc::evns.find(gt->cmd);
		if (it != Gsc::evns.end())
		{
			Cb* cb = it->second;
			LOG_TRACE("\n  --> END(%s): {%s}, BEGIN(%s): {%s}, RET: %04X, CMD: %04X, TID: %08X, PEER: %s\n", //
					cb->end->name().c_str(), gt->endx == NULL ? "NULL": gt->endx->ShortDebugString().c_str(),//
					cb->begin->name().c_str(), gt->begin == NULL ? "NULL": gt->begin->ShortDebugString().c_str(),//
					gt->ret, gt->cmd, gt->tid, Net::sockaddr2str(&this->peer).c_str())
		} else
		{
			LOG_FAULT("it`s a bug, cmd: %04X", gt->cmd)
		}
	}
	stmp_pdu* sp = Gsc::pkgEnd(gt->tid, gt->ret, gt->endx, gt->extend);
	uint len;
	uchar* dat = stmpenc_take(sp, &len);
	this->send(dat, len);
	free(sp->buff);
	free(sp);
	gt->finish();
}

/** 当N2H与Gusr无关联时的STMP-UNI消息. */
void N2H::sendUni(ushort cmd, Message* uni, string* ext)
{
	//未实现.
	if (Gsc::ec)
		Gsc::ec->gn2hSendUni(this, cmd, uni, ext);
}

/** 延迟关闭. */
void N2H::lazyClose()
{
	if (this->est)
	{
		this->lz = true;
		Lx::getGwk()->addLazyCloseN2h(this);
	}
}

string N2H::toString()
{
	string str;
	SPRINTF_STRING(&str, "N2H: rc: %d, rf: %s, wk: %d, peer: %s, est: %s, cfd: %d, dlen: %d, uid: %llu", //
			this->rc, this->rf ? "true" : "false", this->wk, //
			Net::sockaddr2str(&this->peer).c_str(), this->est ? "true" : "false", //
			this->cfd, this->dlen, this->gusr == NULL ? 0 : this->gusr->uid)
	return str;
}

N2H::~N2H()
{
	Gstat::inc(LIBGSC_N2H_DELETE);
}

