
////////////////////////////////////////////////////////////////////////////////////////
// EXAMPLE 2 : Publish an input channel, offer a TCP transport and subscribe remotely //
////////////////////////////////////////////////////////////////////////////////////////

#include <libpubsub.h>


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
	publish_in("prout", [&](const char* buf, size_t len) {
		printf("PROUT RECV : %s\n", buf);
	});

	// Offer a TCP transport for this channel
	offer_transport("prout", "tcp://:12000");

	for(;;) sleep(1);

	} catch(const char* e) {
		fprintf(stderr, "ERROR : %s\n", e);
	}

}

void B() {
	try {

	//////////////////////////
	// PEER B : SUBSCRIBER

	// Connect to the publisher's host
	add_host("localhost:12212");

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

int main(int argc, char **argv) {
	if(argc < 2) USAGE();
	if(!strcmp(argv[1], "A")) A();
	else if(!strcmp(argv[1], "B")) B();
	else USAGE();
	return 0;
}
