/*
 * common.cpp
 *
 *  Created on: 18 août 2016
 *      Author: jfellus
 */

#include "common.h"
#include "hosts.h"

namespace pubsub {

static bool bInit = false;

SignalingServer* signalingServer = NULL;



void init() {
	if(bInit) return;
	bInit = true;

	signalingServer = new SignalingServer();
}


}



