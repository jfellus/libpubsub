
//////////////////////////////////////////////////////////////////////////////
// EXAMPLE 5 : Multiple subscribers to an input channel with SHM transport //
//////////////////////////////////////////////////////////////////////////////

#include <libpubsub.h>
#include <unistd.h>
#include <stdio.h>
#include <utils/utils.h>


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
	int fd = publish_in("prout", [&](const char* buf, size_t len) {
		printf("[publisher] : %s\n", buf);
	});

	// Offer a TCP transport for this channel
	offer_transport("prout", "shm://:1024");

	sleep(1000);

	} catch(const char* e) {
		fprintf(stderr, "ERROR : %s\n", e);
	}

}

void B() {
	try {
	srand(time(NULL));
	int ID = rand();

	//////////////////////////
	// PEER B : SUBSCRIBER1

	// Connect to the publisher's host
	add_host("localhost:12212");

	// Subscribe to 'prout' using any available SHM transport
	int fd = subscribe_out("prout", "shm://");

	for(;;) {
		send(fd, SSTR("taradata_" << ID));
		usleep(100000);
	}

	} catch(const char* e) {
		fprintf(stderr, "ERROR : %s\n", e);
	}
}


int main(int argc, char **argv) {
	DBG_LEVEL = 2;
	if(argc < 2) USAGE();
	if(!strcmp(argv[1], "A")) A();
	else if(!strcmp(argv[1], "B")) B();
	else USAGE();
	return 0;
}
