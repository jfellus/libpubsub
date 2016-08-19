/*
 * common.cpp
 *
 *  Created on: 18 août 2016
 *      Author: jfellus
 */

#include "common.h"
#include "hosts.h"
#include "channel.h"
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

namespace pubsub {

static bool bInit = false;

SignalingServer* signalingServer = NULL;



static void _sigint_handler(int signo) {
	exit(0);
}

static void _at_exit() {
	uninit();
}

void init() {
	if(bInit) return;
	bInit = true;

	atexit(_at_exit);
	signal(SIGINT, _sigint_handler);

	signalingServer = new SignalingServer();
}


void uninit() {
	signalingServer->close();
	close_all_endpoints();
}


}



