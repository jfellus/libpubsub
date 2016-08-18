/*
 * example1.cpp
 *
 *  Created on: 18 août 2016
 *      Author: jfellus
 */

#include <libpubsub.h>

using namespace pubsub;


int main(int argc, char **argv) {

	try {

	////////////////////////
	// PART 1 : PUBLISH

	// Publish an input channel with name 'prout'
	publish_in("prout", [&](const char* buf, size_t len) {
		printf("PROUT RECV : %s\n", buf);
	});

	// Offer a TCP transport for this channel
	offer_transport("prout", "tcp://:12000");



	//////////////////////////
	// PART 2 : SUBSCRIBE

	// Subscribe to 'prout' using any available TCP transport
	int fd = subscribe_out("prout", "tcp://");

	// Send data repeatedly
	for(;;) {
		send(fd, "ta mere la pute",3);
		usleep(100000);
	}



	} catch(const char* e) {
		fprintf(stderr, "ERROR : %s\n", e);
	}
}