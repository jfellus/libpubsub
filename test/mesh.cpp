/*
 * mesh.cpp
 *
 *  Created on: 18 oct. 2016
 *      Author: jfellus
 */

#include <libpubsub.h>
#include <signaling.h>
#include <channel.h>
#include <stdio.h>
#include <utils/utils.h>

int main(int argc, char **argv) {
	pubsub::DBG_LEVEL = 1;
	pubsub::init();


	if(argc>1) {
		pubsub::Subscription* s = pubsub::subscribe_channel("A");
		s->on_message = [&](const char* msg, size_t len) { DBG("Subscription to " << s->channel->name << " recv " << msg); };
		while(1) { sleep(1); }
	}
	else {
		pubsub::Channel* c = pubsub::publish_channel("A");
		c->on_message = [&](const char* msg, size_t len) { DBG("Publisher " << c->name << " recv " << msg); };
		for(int i=0; ; i++) { sleep(1); c->write(TOSTRING("TEST " << i)); }
	}

}
