/*
 * Cfg.h
 *
 *  Created on: Feb 3, 2015 9:41:47 AM
 *      Author: xuzewen
 */

#ifndef CFG_H_
#define CFG_H_

#if !defined (__LIBGSCH__) && !defined (LIBGSC)
#error only libgsc.h can be included directly.
#endif

#include "Gsc.h"

class Cfg
{
public:
	static string libgsc_server_host;
	static int libgsc_server_port;
	static int libgsc_worker;
	static int libgsc_peer_limit;
	static int libgsc_peer_mtu;
	static int libgsc_peer_rcvbuf;
	static int libgsc_peer_sndbuf;
	static ullong libgsc_peer_heartbeat;
	static ullong libgsc_n2h_zombie;
	static ullong libgsc_n2h_lazy_close;
	static ullong libgsc_h2n_reconn;
	static ullong libgsc_h2n_trans_timeout;
	static ullong libgsc_quartz;
	//
	static string libgsc_log_host;
	static int libgsc_log_port;
	static string libgsc_log_level;
private:
	Cfg();
	virtual ~Cfg();
public:
	static void init();
};

#endif /* CFG_H_ */
