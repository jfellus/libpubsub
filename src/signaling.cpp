/*
 * signaling.cpp

 *
 *  Created on: 27 oct. 2016
 *      Author: jfellus
 */

#include "signaling.h"
#include "mesh.h"
#include "utils/utils.h"
#include "channel.h"

namespace pubsub {


static pthread_mutex_t mut = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
static void LOCK() { pthread_mutex_lock(&mut); }
static void UNLOCK() { pthread_mutex_unlock(&mut); }


void digest_message(Host* h, string& s) {
	LOCK();
	if(str_starts_with(s, "PUBLISH=")) {
		apply_publish_statement(h, str_after(s, "="));
	} else if(str_starts_with(s, "UNPUBLISH=")) {
		apply_unpublish_statement(h, str_after(s, "="));
	}

	UNLOCK();
}

void on_host_open(Host* h) {
	LOCK();
	broadcast_published_channels();
	UNLOCK();
}

void on_host_close(Host* h) {
	LOCK();
	close_all_channels(h);
	UNLOCK();
}

}
