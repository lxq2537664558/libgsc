/*
 * Gsc.cpp
 *
 *  Created on: 2012-7-30
 *      Author: xuzewen
 */

#include "Gsc.h"
#include "Cfg.h"
#include "../net/Lx.h"

uint Gsc::rr = 0;
unordered_map<ushort, Cb*> Gsc::evns; /** N2H上行业务消息回调. */
EvnCb* Gsc::ec = NULL; /** 事务结束回调. */
vector<ActorGusrDb*> Gsc::db; /** Gusr数据库(适用于短连接业务). */

Gsc::Gsc()
{

}

/** 初始化libgsc消息总线. */
bool Gsc::init()
{
	Cfg::init();
	if (!Lx::init())
		return false;
	//ActorAuto::init();
	return true;
}

/** 开启libgsc服务端口. */
bool Gsc::publish()
{
	return Lx::publish(Cfg::libgsc_server_host.c_str(), Cfg::libgsc_server_port);
}

/** 定时器振荡. */
void Gsc::hold()
{
	while (1)
	{
		Misc::sleep(Cfg::libgsc_quartz);
		for (int i = 0; i < Cfg::libgsc_worker; ++i)
		{
			(Lx::wks + i)->future([i]
			{
				(Lx::wks + i)->check(DateMisc::getMsec());
			});
		}
		if (Gsc::ec != NULL)
			Gsc::ec->quartz(DateMisc::getMsec());
	}
}

/** 注册EvnCb. */
void Gsc::regEvnCb(EvnCb* ec)
{
	Gsc::ec = ec;
}

/** 注册N2H上的消息回调. */
bool Gsc::regN2HEvn(ushort cmd, const Descriptor* begin, const Descriptor* end, const Descriptor* uni, void* cb, bool gusr)
{
	if (Gsc::evns.find(cmd) != Gsc::evns.end())
	{
		LOG_ERROR("duplicate cmd: %04X, begin: %s, end: %s, uni: %s", cmd, begin == NULL ? "NULL" : begin->name().c_str(), end == NULL ? "NULL" : end->name().c_str(), uni == NULL ? "NULL" : uni->name().c_str())
		return false;
	}
	LOG_INFO("n2h-msg, cmd: %04X, begin: %s, end: %s, uni: %s, gusr: %s", cmd, begin == NULL ? "NULL" : begin->name().c_str(), end == NULL ? "NULL" : end->name().c_str(), uni == NULL ? "NULL" : uni->name().c_str(), gusr ? "true" : "false")
	Gsc::evns[cmd] = new Cb(cmd, cb, begin, end, uni, gusr);
	return true;
}

/** 注册Gusr数据库(适用于短连接业务). */
void Gsc::addActorGusrDb(ActorGusrDb* db)
{
	Gsc::db.push_back(db);
}

/** 尽可能选择一个不忙的Gusr数据库处理Actor(适用于短连接业务). */
ActorGusrDb* Gsc::getGusrDb()
{
	for (uint i = 0; i < Gsc::db.size(); ++i)
	{
		if (Gsc::db.at(i)->isBusy())
			continue;
		return Gsc::db.at(i);
	}
	return Gsc::db.at(Misc::randomInt() % Gsc::db.size());
}

/** 为Actor选择一个工作线程(round-robin). */
uint Gsc::rrWk()
{
	return __sync_fetch_and_add(&Gsc::rr, 1) % Cfg::libgsc_worker;
}

/** 为Actor选择一个工作线程(散列). */
uint Gsc::hashWk(ullong id)
{
	return (uint) ((id & 0x7FFFFFFFFFFFFFFFL) % Cfg::libgsc_worker);
}

/** 打包一个STMP-BEGIN中的加密部分. */
stmp_pdu* Gsc::pkgBeginSecDat(const ushort cmd, const Message* begin, const string* ext)
{
	stmp_pdu* sec = (stmp_pdu*) malloc(sizeof(stmp_pdu));
	string pb = begin == NULL ? "" : begin->SerializeAsString();
	int size = pb.length() + (ext == NULL ? 0 : ext->length());
	size = size + (8 - (size % 8)) + 64; /** 只多不少. */
	sec->len = size;
	sec->rm = size;
	sec->p = 0;
	sec->buff = (uchar*) malloc(size);
	if (ext != NULL)
		stmpenc_add_bin(sec, STMP_TAG_ATT, (uchar*) ext->data(), ext->length());
	if (pb.length() != 0)
		stmpenc_add_bin(sec, STMP_TAG_DAT, (uchar*) pb.data(), pb.length());
	stmpenc_add_short(sec, STMP_TAG_CMD, cmd);
	stmpenc_add_tag(sec, STMP_TAG_SEC);
	return sec;
}

/** 打包一个STMP-BEGIN. */
stmp_pdu* Gsc::pkgBegin(const ushort cmd, const uint tid, const Message* beginx, const string* ext)
{
	stmp_pdu* begin = (stmp_pdu*) malloc(sizeof(stmp_pdu));
	string pb = beginx == NULL ? "" : beginx->SerializeAsString();
	int size = pb.length() + (ext == NULL ? 0 : ext->length());
	size = size + (8 - (size % 8)) + 64; /** 只多不少. */
	begin->len = size;
	begin->rm = size;
	begin->p = 0;
	begin->buff = (uchar*) malloc(size);
	if (ext != NULL)
		stmpenc_add_bin(begin, STMP_TAG_ATT, (uchar*) ext->data(), ext->length());
	if (pb.length() != 0)
		stmpenc_add_bin(begin, STMP_TAG_DAT, (uchar*) pb.data(), pb.length());
	stmpenc_add_short(begin, STMP_TAG_CMD, cmd);
	stmpenc_add_int(begin, STMP_TAG_STID, tid);
	stmpenc_add_tag(begin, STMP_TAG_TRANS_BEGIN);
	return begin;
}

/** 打包一个STMP-END中的加密部分. */
stmp_pdu* Gsc::pkgEndSecDat(const ushort ret, const Message* end, const string* ext)
{
	stmp_pdu* sec = (stmp_pdu*) malloc(sizeof(stmp_pdu));
	string pb = end == NULL ? "" : end->SerializeAsString();
	int size = pb.length() + (ext == NULL ? 0 : ext->length());
	size = size + (8 - (size % 8)) + 64; /** 只多不少. */
	sec->len = size;
	sec->rm = size;
	sec->p = 0;
	sec->buff = (uchar*) malloc(size);
	if (ext != NULL)
		stmpenc_add_bin(sec, STMP_TAG_ATT, (uchar*) ext->data(), ext->length());
	if (pb.length() != 0)
		stmpenc_add_bin(sec, STMP_TAG_DAT, (uchar*) pb.data(), pb.length());
	stmpenc_add_short(sec, STMP_TAG_RET, ret);
	stmpenc_add_tag(sec, STMP_TAG_SEC);
	return sec;
}

/** 打包一个STMP-END. */
stmp_pdu* Gsc::pkgEnd(const uint tid, const ushort ret, const Message* endx, const string* ext)
{
	stmp_pdu* end = (stmp_pdu*) malloc(sizeof(stmp_pdu));
	string pb = endx == NULL ? "" : endx->SerializeAsString();
	int size = pb.length() + (ext == NULL ? 0 : ext->length());
	size = size + (8 - (size % 8)) + 64; /** 只多不少. */
	end->len = size;
	end->rm = size;
	end->p = 0;
	end->buff = (uchar*) malloc(size);
	if (ext != NULL)
		stmpenc_add_bin(end, STMP_TAG_ATT, (uchar*) ext->data(), ext->length());
	if (pb.length() != 0)
		stmpenc_add_bin(end, STMP_TAG_DAT, (uchar*) pb.data(), pb.length());
	stmpenc_add_short(end, STMP_TAG_RET, ret);
	stmpenc_add_int(end, STMP_TAG_DTID, tid);
	stmpenc_add_tag(end, STMP_TAG_TRANS_END);
	return end;
}

/** 打包一个STMP-UNI中的加密部分. */
stmp_pdu* Gsc::pkgUniSecDat(const ushort cmd, const Message* uni, const string* ext)
{
	stmp_pdu* sp = (stmp_pdu*) malloc(sizeof(stmp_pdu));
	string pb = uni == NULL ? "" : uni->SerializeAsString();
	int size = pb.length() + (ext == NULL ? 0 : ext->length());
	//
	size = size + (8 - (size % 8)) + 64;
	sp->len = size;
	sp->rm = size;
	sp->p = 0;
	sp->buff = (uchar*) malloc(size);
	if (ext != NULL)
		stmpenc_add_bin(sp, STMP_TAG_ATT, (uchar*) ext->data(), ext->length());
	if (pb.length() != 0)
		stmpenc_add_bin(sp, STMP_TAG_DAT, (uchar*) pb.data(), pb.length());
	stmpenc_add_short(sp, STMP_TAG_CMD, cmd);
	stmpenc_add_tag(sp, STMP_TAG_SEC);
	return sp;
}

/** 解包一个STMP-BEGIN(适用于长连接). */
Cb* Gsc::unpkgBegin0(stmp_node* root, uint* tid, ushort* cmd, Message** begin, string** ext)
{
	*begin = NULL;
	*ext = NULL;
	if (stmpdec_get_int(root, STMP_TAG_STID, tid) != 0)
	{
		LOG_DEBUG("missing required field: STMP_TAG_STID")
		return NULL;
	}
	if (stmpdec_get_short(root, STMP_TAG_CMD, cmd) != 0)
	{
		LOG_DEBUG("missing required field: STMP_TAG_CMD")
		return NULL;
	}
	auto it = Gsc::evns.find(*cmd);
	if (it == Gsc::evns.end()) /* 不支持的命令字 . */
	{
		LOG_DEBUG("can not found call back for this cmd: %04X", *cmd)
		return NULL;
	}
	if (it->second->begin == NULL)
	{
		LOG_DEBUG("it`s an unexpected cmd: %04X", *cmd)
		return NULL;
	}
	uint len;
	uchar* dat = stmpdec_peek_bin(root, STMP_TAG_DAT, &len);
	if (dat == NULL)
	{
		LOG_DEBUG("missing required field: STMP_TAG_DAT")
		return NULL;
	}
	*begin = Gsc::newPbMsg(it->second->begin, dat, len);
	if (*begin == NULL) /** 为简单起见(避免在业务处理中频繁判断BEGIN是否为空), BEGIN中的pb对象为必选. */
	{
		LOG_DEBUG("can not parse %s from STMP-BEGIN", it->second->begin->name().c_str())
		return NULL;
	}
	dat = stmpdec_peek_bin(root, STMP_TAG_ATT, &len);
	if (dat != NULL)
		*ext = new string((char*) dat, len);
	return it->second;
}

/** 解包一个STMP-UNI. */
Cb* Gsc::unpkgUni(stmp_node* root, ushort* cmd, Message** uni, string** ext)
{
	*uni = NULL;
	*ext = NULL;
	if (stmpdec_get_short(root, STMP_TAG_CMD, cmd) != 0)
	{
		LOG_DEBUG("missing required field: STMP_TAG_CMD")
		return NULL;
	}
	auto it = Gsc::evns.find(*cmd);
	if (it == Gsc::evns.end()) /** 找不到回调. */
	{
		LOG_WARN("can not found call back for this cmd: %04X", *cmd); /* 不支持的命令字. */
		return NULL;
	}
	Cb* cb = it->second;
	uint len;
	uchar* dat = stmpdec_peek_bin(root, STMP_TAG_DAT, &len);
	if (dat != NULL)
		*uni = Gsc::newPbMsg(cb->uni, dat, len);
	uchar* att = stmpdec_peek_bin(root, STMP_TAG_ATT, &len);
	if (att != NULL)
		*ext = new string((char*) att, len);
	return cb;
}

/** 反射得到一个pb message对象. */
Message* Gsc::newPbMsg(const Descriptor* desc)
{
	const Message* pt = MessageFactory::generated_factory()->GetPrototype(desc);
	if (pt == NULL)
	{
		printf("can`t not found message type stub for this desc, pb name: %s", desc->name().c_str());
		return NULL;
	}
	return pt->New();
}

/** 反射得到一个pb message对象. */
Message* Gsc::newPbMsg(const Descriptor* desc, uchar* dat, int len)
{
	Message* msg = Gsc::newPbMsg(desc);
	if (msg == NULL)
		return NULL;
	if (msg->ParseFromArray(dat, len))
		return msg;
	LOG_ERROR("serialize message failed, name: %s", desc->name().c_str());
	delete msg;
	return NULL;
}

Gsc::~Gsc()
{

}

