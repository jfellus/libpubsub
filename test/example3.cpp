
//////////////////////////////////////////////////////////////////////////////
// EXAMPLE 3 : Multiple subscribers to an output channel with TCP transport //
//////////////////////////////////////////////////////////////////////////////

#include <libpubsub.h>
#include <unistd.h>
#include <stdio.h>


using namespace pubsub;


void USAGE() {
	printf("Please provide an argument (A or B)\n");
	exit(0);
}

void A() {

	try {

	////////////////////////
	// PEER A : PUBLISHER

	// Publish an input channel with name 'prout'
	int fd = publish_out("prout");

	// Offer a TCP transport for this channel
	offer_transport("prout", "tcp://:12000");

	for(;;) {
		send(fd, "taradata");
		usleep(100000);
	}

	} catch(const char* e) {
		fprintf(stderr, "ERROR : %s\n", e);
	}

}

void B() {
	try {

	//////////////////////////
	// PEER B : SUBSCRIBER1

	// Connect to the publisher's host
	add_host("localhost:12212");

	// Subscribe to 'prout' using any available TCP transport
	int fd = subscribe_in("prout", "tcp://", [&](const char* buf, size_t len) {
		printf("[subscriber] : %s\n", buf);
	});

	sleep(100);

	} catch(const char* e) {
		fprintf(stderr, "ERROR : %s\n", e);
	}
}


int main(int argc, char **argv) {
	if(argc < 2) USAGE();
	if(!strcmp(argv[1], "A")) A();
	else if(!strcmp(argv[1], "B")) B();
	else USAGE();
	return 0;
}
