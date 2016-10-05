/*
 * common.cpp
 *
 *  Created on: 18 août 2016
 *      Author: jfellus
 */

#include "common.h"
#include "hosts.h"
#include "channel.h"
#include "signaling.h"
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

namespace pubsub {

static bool bInit = false;

static pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;



static void _sigint_handler(int signo) {
	exit(0);
}

static void _at_exit() {
	uninit();
}

void init() {
	pthread_mutex_lock(&mut);
	if(!bInit) {
		bInit = true;

		atexit(_at_exit);
		signal(SIGINT, _sigint_handler);

		signalingServer = new SignalingServer();
	}
	pthread_mutex_unlock(&mut);
}


void uninit() {
	pthread_mutex_lock(&mut);
	signalingServer->close();
	close_all_endpoints();
	pthread_mutex_unlock(&mut);
}


}



