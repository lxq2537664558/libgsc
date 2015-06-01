/*
 * Gusr.cpp
 *
 *  Created on: Jan 31, 2015
 *      Author: root
 */

#include "Gusr.h"
#include "../core/Cfg.h"
#include "../core/Gsc.h"
#include "../net/Lx.h"

Gusr::Gusr(ullong uid, const char* name) :
		Actor(ACTOR_ITC, name, Gsc::hashWk(uid))
{
	this->uid = uid;
	this->n2h = NULL;
}

/** 处理长连接上的STMP-BEGIN. */
void Gusr::procBegin0(uchar* dat, uint len, Gn2htrans* gt)
{
	uint out;
	uchar* msg = this->decryp(dat, len, &out); /** 将length身后的字段解密. */
	stmp_node root;
	if (stmpdec_unpack(msg, out, &root) != 0)
	{
		loop: //
		N2H* n2h = this->n2h;
		n2h->future([n2h, gt] /** 前往N2H关闭连接. */
		{
			n2h->close();
			delete gt; /** 删除事务. */
		});
		free(msg);
		return;
	}
	if (stmpdec_get_short(&root, STMP_TAG_CMD, &gt->cmd) != 0)
	{
		LOG_DEBUG("missing required field: STMP_TAG_CMD")
		stmpdec_free(&root);
		goto loop;
	}
	auto it = Gsc::evns.find(gt->cmd);
	if (it == Gsc::evns.end()) /* 不支持的命令字 . */
	{
		LOG_DEBUG("can not found call back for this cmd: %04X", gt->cmd)
		stmpdec_free(&root);
		goto loop;
	}
	Cb* cb = it->second;
	if (cb->gusr != true || cb->begin == NULL) /** 不应该在这里收到. */
	{
		LOG_DEBUG("it`s an unexpected cmd: %04X", it->first)
		stmpdec_free(&root);
		goto loop;
	}
	uchar* pb = stmpdec_peek_bin(&root, STMP_TAG_DAT, &len);
	if (pb == NULL)
	{
		LOG_DEBUG("missing required field: STMP_TAG_DAT")
		stmpdec_free(&root);
		goto loop;
	}
	gt->begin = Gsc::newPbMsg(cb->begin, pb, len);
	if (gt->begin == NULL) /** 为简单起见(避免在业务处理中频繁判断BEGIN是否为空), BEGIN中的pb对象为必选. */
	{
		LOG_DEBUG("can not parse %s from STMP-BEGIN", it->second->begin->name().c_str())
		stmpdec_free(&root);
		goto loop;
	}
	uchar* att = stmpdec_peek_bin(&root, STMP_TAG_ATT, &len);
	if (att != NULL)
		gt->extbegin = new string((char*) att, len);
	stmpdec_free(&root);
	//
	if (Logger::isTrace())
	{
		LOG_TRACE("\n  <-- BEGIN(%s): {%s}, END(%s), UID: %llu, CMD: %04X, TID: %08X, PEER: %s\n", //
				cb->begin->name().c_str(), gt->begin == NULL ? "NULL" : gt->begin->ShortDebugString().c_str(),//
				cb->end->name().c_str(), this->uid, gt->cmd, gt->tid, Net::sockaddr2str(&gt->n2h->peer).c_str())
	}
	((void (*)(Gusr* gusr, Gn2htrans* gt, Message* begin)) (cb->cb))(this, gt, gt->begin);
}

/** 处理短连接上的STMP-BEGIN. */
void Gusr::procBegin1(uchar* dat, uint len, Gn2h* gt, ullong sid)
{
	uint out;
	uchar* msg = this->decryp(dat, len, &out); /** 将cid身后的字段解密. */
	stmp_node root;
	if (stmpdec_unpack(msg, out, &root) != 0)
	{
		loop: //
		gt->n2h->future([this, gt] /** 前往N2H关闭连接. */
		{
			gt->n2h->close();
			delete gt; /** 删除事务. */
		});
		free(msg);
		return;
	}
	Cb* cb = Gsc::unpkgBegin0(&root, &gt->tid, &gt->cmd, &gt->begin, &gt->extbegin);
	stmpdec_free(&root);
	if (cb == NULL || cb->cb == NULL)
	{
		LOG_DEBUG("it`s an unexpected cmd: %04X", gt->cmd)
		goto loop;
	}
	free(msg);
	//
	if (Logger::isTrace())
	{
		LOG_TRACE("\n  <-- BEGIN(%s): {%s}, END(%s), UID: %llu, SID: %llu, CMD: %04X, TID: %08X, PEER: %s\n", //
				cb->begin->name().c_str(), gt->begin == NULL ? "NULL" : gt->begin->ShortDebugString().c_str(),//
				cb->end->name().c_str(), this->uid, gt->sid, gt->cmd, gt->tid, Net::sockaddr2str(&gt->n2h->peer).c_str())
	}
	if (gt->sid != sid) /** 上行的会话ID与Gusr当前的csid不一致. */
	{
		LOG_DEBUG("session id does not match, may be have a new session created on this gusr, uid: %llu, gusr-sid: %llu, gt-sid: %llu", this->uid, sid, gt->sid)
		gt->end(RET_SID_ERROR);
		//lazy-close.
		return;
	}
	((void (*)(Gusr* gusr, Gn2h* gt, Message* begin)) (cb->cb))(this, gt, gt->begin);
}

/** 处理连接上的STMP-UNI, 只有长连接支持STMP-UNI. */
void Gusr::procUni(uchar* dat, uint len)
{
	uint out;
	uchar* msg = this->decryp(dat, len, &out); /** 将length身后的字段解密. */
	stmp_node root;
	if (stmpdec_unpack(msg, out, &root) != 0)
	{
		loop: //
		this->n2h->future([this] /** 前往N2H关闭连接. */
		{
			this->n2h->close();
		});
		free(msg);
		return;
	}
	ushort cmd;
	if (stmpdec_get_short(&root, STMP_TAG_CMD, &cmd) != 0)
	{
		LOG_DEBUG("missing required field: STMP_TAG_CMD")
		stmpdec_free(&root);
		goto loop;
	}
	auto it = Gsc::evns.find(cmd);
	if (it == Gsc::evns.end()) /** 找不到回调. */
	{
		LOG_WARN("can not found call back for this cmd: %04X", cmd); /* 不支持的命令字. */
		stmpdec_free(&root);
		goto loop;
	}
	Cb* cb = it->second;
	if (cb->uni == NULL || cb->cb == NULL)
	{
		LOG_DEBUG("it`s an unexpected cmd: %04X", cmd)
		stmpdec_free(&root);
		goto loop;
	}
	Message* uni = NULL;
	string* ext = NULL;
	uint size;
	uchar* pb = stmpdec_peek_bin(&root, STMP_TAG_DAT, &size);
	if (dat != NULL)
		uni = Gsc::newPbMsg(cb->uni, pb, len);
	uchar* att = stmpdec_peek_bin(&root, STMP_TAG_ATT, &size);
	if (att != NULL)
		ext = new string((char*) att, size);
	stmpdec_free(&root);
	free(msg);
	//
	LOG_TRACE("\n  <-- UNI(%s): {%s}, CMD: %04X, PEER: %s\n", cb->uni->name().c_str(), uni == NULL ? "NULL": uni->ShortDebugString().c_str(), cb->cmd, Net::sockaddr2str(&this->n2h->peer).c_str())
	((void (*)(Gusr* gusr, Message* uni, string* ext)) (cb->cb))(this, uni, ext);
	if (Gsc::ec)
		Gsc::ec->gn2hRecvUniOnGusr(this, cb->cmd, uni, ext);
	//
	if (uni != NULL)
		delete uni;
	if (ext != NULL)
		delete ext;
	return;
}

/** 加密, 并将事务前转到自身关联的N2H上出栈(长连接). */
void Gusr::sendEnd0(Gn2htrans* gt)
{
	if (Logger::isTrace())
	{
		auto it = Gsc::evns.find(gt->cmd);
		if (it != Gsc::evns.end())
		{
			Cb* cb = it->second;
			LOG_TRACE("\n  --> END(%s): {%s}, BEGIN(%s): {%s}, UID: %llu, RET: %04X, CMD: %04X, TID: %08X, PEER: %s\n", //
					cb->end->name().c_str(), gt->endx == NULL ? "NULL": gt->endx->ShortDebugString().c_str(),//
					cb->begin->name().c_str(), gt->begin == NULL ? "NULL": gt->begin->ShortDebugString().c_str(),//
					this->uid, gt->ret, gt->cmd, gt->tid, Net::sockaddr2str(&gt->n2h->peer).c_str())
		} else
		{
			LOG_FAULT("it`s a bug, cmd: %04X", gt->cmd)
		}
	}
	stmp_pdu* sp = Gsc::pkgEndSecDat(gt->ret, gt->endx, gt->extend);
	uint len;
	uchar* dat = stmpenc_take(sp, &len);
	uint out;
	uchar* cryp = this->encryp(dat, len, &out);
	free(sp->buff);
	free(sp);
	//
	stmp_pdu* end = (stmp_pdu*) malloc(sizeof(stmp_pdu));
	int size = out;
	size = size + (8 - (size % 8)) + 64; /** 只多不少. */
	end->len = size;
	end->rm = size;
	end->p = 0;
	end->buff = (uchar*) malloc(size);
	stmpenc_add_bin(end, STMP_TAG_DAT, cryp, out);
	stmpenc_add_int(end, STMP_TAG_DTID, gt->tid);
	stmpenc_add_tag(end, STMP_TAG_TRANS_END);
	dat = stmpenc_take(end, &len);
	//
	gt->n2h->future([gt, end, dat, len]
	{
		gt->n2h->send(dat, len);
		free(end->buff);
		free(end);
		gt->finish();
	});
	free(cryp);
}

/** 加密, 并将事务前转到自身关联的N2H上出栈(短连接). */
void Gusr::sendEnd1(Gn2h* gt)
{
	if (Logger::isTrace())
	{
		auto it = Gsc::evns.find(gt->cmd);
		if (it != Gsc::evns.end())
		{
			Cb* cb = it->second;
			LOG_TRACE("\n  --> END(%s): {%s}, BEGIN(%s): {%s}, UID: %llu, RET: %04X, CMD: %04X, TID: %08X, PEER: %s\n", //
					cb->end->name().c_str(), gt->endx == NULL ? "NULL": gt->endx->ShortDebugString().c_str(),//
					cb->begin->name().c_str(), gt->begin == NULL ? "NULL": gt->begin->ShortDebugString().c_str(),//
					this->uid, gt->ret, gt->cmd, gt->tid, Net::sockaddr2str(&gt->n2h->peer).c_str())
		} else
		{
			LOG_FAULT("it`s a bug, cmd: %04X", gt->cmd)
		}
	}
	stmp_pdu* sp = Gsc::pkgEndSecDat(gt->ret, gt->endx, gt->extend);
	uint len;
	uchar* dat = stmpenc_take(sp, &len);
	uint out;
	uchar* cryp = this->encryp(dat, len, &out); /* 加密. */
	free(sp->buff);
	free(sp);
	//
	stmp_pdu* end = (stmp_pdu*) malloc(sizeof(stmp_pdu));
	int size = out;
	size = size + (8 - (size % 8)) + 64; /** 只多不少. */
	end->len = size;
	end->rm = size;
	end->p = 0;
	end->buff = (uchar*) malloc(size);
	stmpenc_add_bin(end, STMP_TAG_DAT, cryp, out);
	stmpenc_add_int(end, STMP_TAG_DTID, gt->tid);
	stmpenc_add_tag(end, STMP_TAG_TRANS_END);
	dat = stmpenc_take(end, &len);
	//
	gt->n2h->future([gt, end, dat, len]
	{
		gt->n2h->send(dat, len);
		gt->finish();
		free(end->buff);
		free(end);
	});
	free(cryp);
}

/** 加密, 并将消息前转到自身关联的N2H上出栈. */
void Gusr::sendUni(ushort cmd, Message* uni, string* ext)
{
	if (Logger::isTrace())
	{
		auto it = Gsc::evns.find(cmd);
		if (it != Gsc::evns.end())
		{
			Cb* cb = it->second;
			LOG_TRACE("\n  --> UNI(%s): {%s}, UID: %llu, CMD: %04X, PEER: %s\n", //
					cb->uni->name().c_str(), uni == NULL ? "NULL" : uni->ShortDebugString().c_str(),//
					this->uid, cmd, this->n2h == NULL ? "NULL" : Net::sockaddr2str(&this->n2h->peer).c_str())
		} else
		{
			LOG_FAULT("it`s a bug, cmd: %04X", cmd)
		}
	}
	if (this->n2h == NULL)
	{
		if (Gsc::ec)
			Gsc::ec->gn2hSendUniOnGusr(this, cmd, uni, ext);
		return;
	}
	stmp_pdu* sp = this->pkgUni(cmd, uni, ext);
	this->n2h->future([this, sp]
	{
		uint len;
		uchar* dat = stmpenc_take(sp, &len);
		this->n2h->send(dat, len);
		free(sp->buff);
		free(sp);
	});
	if (Gsc::ec)
		Gsc::ec->gn2hSendUniOnGusr(this, cmd, uni, ext);
	if (uni != NULL)
		delete uni;
	if (ext != NULL)
		delete ext;
}

/** 网络连接是否正常. */
bool Gusr::isOnline()
{
	return this->n2h != NULL;
}

/** 踢下线(将导致连接被延迟关闭). */
void Gusr::kick(ushort cmd, Message* uni)
{
	stmp_pdu* sp = this->pkgUni(cmd, uni, NULL);
	N2H* n2h = this->n2h;
	n2h->future([n2h, sp]
	{
		uint len;
		uchar* dat = stmpenc_take(sp, &len);
		n2h->send(dat, len);
		free(sp->buff);
		free(sp);
		n2h->lts = DateMisc::getMsec() + Cfg::libgsc_n2h_lazy_close;
		n2h->lazyClose();
		if (n2h->gusr != NULL) /** 如果gusr不为NULL, 则一定存在相互引用. */
		{
			Gusr* gusr = n2h->gusr;
			gusr->future([gusr]
					{
						/** gusr->n2h = NULL; 由于这里是并行, 可能将老的连接连接踢下线前, gusr->n2h已经被新的N2H覆盖, 因此, 这里不能置为NULL. */
						gusr->unRef();
					});
			n2h->gusr = NULL; /** N2H亦不再与Gusr关联. */
			n2h->unRef();
		}
	});
	this->n2h = NULL; /** 连接置空. */
	if (Gsc::ec)
		Gsc::ec->gn2hSendUniOnGusr(this, cmd, uni, NULL);
	if (uni != NULL)
		delete uni;
}

/** 踢下线, 无消息提示(连接被立即关闭). */
void Gusr::kickNoMsg()
{
	N2H* n2h = this->n2h;
	n2h->future([n2h]
	{
		Lx::getGwk()->removeActorNet(n2h); /** 关闭连接. */
		/**
		 *
		 * 这里为了达到静默关闭目的, 没调用n2h->close();
		 *
		 * */
		if (n2h->gusr != NULL) /** 如果gusr不为NULL, 则一定存在相互引用. */
		{
			Gusr* gusr = n2h->gusr;
			gusr->future([gusr]
					{
						/** gusr->n2h = NULL; 由于这里是并行, 可能将老的连接连接踢下线前, gusr->n2h已经被新的N2H覆盖, 因此, 这里不能置为NULL. */
						gusr->unRef();
					});
			n2h->gusr = NULL; /** N2H亦不再与Gusr关联. */
			n2h->unRef();
		}
		n2h->del();
	});
	this->n2h = NULL; /** 连接置空. */
}

/** 构造一个加密过的UNI消息. */
stmp_pdu* Gusr::pkgUni(ushort cmd, Message* uni, string* ext)
{
	stmp_pdu* sp = Gsc::pkgUniSecDat(cmd, uni, ext);
	uint len;
	uchar* dat = stmpenc_take(sp, &len);
	uint out;
	uchar* cryp = this->encryp(dat, len, &out); /* 加密. */
	free(sp->buff);
	free(sp);
	//
	sp = (stmp_pdu*) malloc(sizeof(stmp_pdu));
	int size = out;
	size = size + (8 - (size % 8)) + 64; /** 只多不少. */
	sp->len = size;
	sp->rm = size;
	sp->p = 0;
	sp->buff = (uchar*) malloc(size);
	stmpenc_add_bin(sp, STMP_TAG_DAT, cryp, out);
	stmpenc_add_tag(sp, STMP_TAG_TRANS_UNI);
	free(cryp);
	return sp;
}

Gusr::~Gusr()
{

}

