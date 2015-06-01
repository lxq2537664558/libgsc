/*
 * Cfg.cpp
 *
 *  Created on: Feb 3, 2015 9:41:47 AM
 *      Author: xuzewen
 */

#include "Cfg.h"

string Cfg::libgsc_server_host = "0.0.0.0";
int Cfg::libgsc_server_port = 1225;
int Cfg::libgsc_worker = 4;
int Cfg::libgsc_peer_limit = 1024;
int Cfg::libgsc_peer_mtu = 8192;
int Cfg::libgsc_peer_rcvbuf = 8192;
int Cfg::libgsc_peer_sndbuf = 8192;
ullong Cfg::libgsc_peer_heartbeat = 5 * 1000;
ullong Cfg::libgsc_n2h_zombie = 3 * 1000;
ullong Cfg::libgsc_n2h_lazy_close = 3 * 1000;
ullong Cfg::libgsc_h2n_reconn = 3 * 1000;
ullong Cfg::libgsc_h2n_trans_timeout = 5 * 1000;
ullong Cfg::libgsc_quartz = 50;
string Cfg::libgsc_log_host = "127.0.0.1";
int Cfg::libgsc_log_port = 1224;
string Cfg::libgsc_log_level = "INFO";

Cfg::Cfg()
{

}

void Cfg::init()
{
	Cfg::libgsc_server_host = Misc::getSetEnvStr(LIBGSC_SERVER_HOST, Cfg::libgsc_server_host.c_str());
	Cfg::libgsc_server_port = Misc::getSetEnvInt(LIBGSC_SERVER_PORT, Cfg::libgsc_server_port);
	Cfg::libgsc_worker = Misc::getSetEnvInt(LIBGSC_WORKER, Cfg::libgsc_worker);
	Cfg::libgsc_peer_limit = Misc::getSetEnvInt(LIBGSC_PEER_LIMIT, Cfg::libgsc_peer_limit);
	Cfg::libgsc_peer_mtu = Misc::getSetEnvInt(LIBGSC_PEER_MTU, Cfg::libgsc_peer_mtu);
	Cfg::libgsc_peer_rcvbuf = Misc::getSetEnvInt(LIBGSC_PEER_RCVBUF, Cfg::libgsc_peer_rcvbuf);
	Cfg::libgsc_peer_sndbuf = Misc::getSetEnvInt(LIBGSC_PEER_SNDBUF, Cfg::libgsc_peer_sndbuf);
	Cfg::libgsc_peer_heartbeat = Misc::getSetEnvInt(LIBGSC_PEER_HEARTBEAT, Cfg::libgsc_peer_heartbeat);
	Cfg::libgsc_n2h_zombie = Misc::getSetEnvInt(LIBGSC_N2H_ZOMBIE, Cfg::libgsc_n2h_zombie);
	Cfg::libgsc_n2h_lazy_close = Misc::getSetEnvInt(LIBGSC_N2H_LAZY_CLOSE, Cfg::libgsc_n2h_lazy_close);
	Cfg::libgsc_h2n_reconn = Misc::getSetEnvInt(LIBGSC_H2N_RECONN, Cfg::libgsc_h2n_reconn);
	Cfg::libgsc_h2n_trans_timeout = Misc::getSetEnvInt(LIBGSC_H2N_TRANS_TIMEOUT, Cfg::libgsc_h2n_trans_timeout);
	Cfg::libgsc_quartz = Misc::getSetEnvInt(LIBGSC_QUARTZ, Cfg::libgsc_quartz);
	//
	Cfg::libgsc_log_host = Misc::getSetEnvStr(LIBGSC_LOG_HOST, Cfg::libgsc_log_host.c_str());
	Cfg::libgsc_log_port = Misc::getSetEnvInt(LIBGSC_LOG_PORT, Cfg::libgsc_log_port);
	Cfg::libgsc_log_level = Misc::getSetEnvStr(LIBGSC_LOG_LEVEL, Cfg::libgsc_log_level.c_str());
	//
	list<string> envs;
	Misc::getEnvs(&envs);
	LOG_INFO("\n----------------------------------------------------------------")
	for (auto& it : envs)
	{
		if (strstr(it.c_str(), "LIBGSC_"))
			LOG_INFO("%s", it.c_str())
	}
	LOG_INFO("\n----------------------------------------------------------------")
}

Cfg::~Cfg()
{
	// TODO Auto-generated destructor stub
}

