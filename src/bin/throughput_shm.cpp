
#include <libpubsub.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <thread>
#include <utils/utils.h>

////////////////////////////////////////////////////////////////
//                                                            //
//          throughput utility                                //
//                                                            //
// This program publishes an input channel                    //
// and monitor the throughput flowing in input                //
//                                                            //
////////////////////////////////////////////////////////////////


int BUFSIZE = 1000000;

int USAGE() {
	fprintf(stderr, "USAGE : throughput <in|out [bufsize]>\n");
	return 1;
}



static std::thread th;
long bytes = 0;
long msg = 0;
void publish_in(const char* channel) {
	th = std::thread([&]() {
		for(;;) {
			bytes = 0; msg = 0;
			sleep(1);
			float kb = bytes/8.0/1024.0;
			if(kb < 1000) printf("%0.2fko/s [%ld msg/s]\n", kb, msg);
			else {
				float mb = kb/1024.0;
				if(mb < 1000) {
					printf("%0.2fMo/s [%ld msg/s]\n", mb, msg);
				}
				else printf("%0.2fGo/s [%ld msg/s]\n", mb/1024.0, msg);
			}
		}
	});

	int fd = pubsub::publish_in(channel, [&](const char* buf, size_t len) {
		bytes += len;
		msg++;
	});
	pubsub::offer_transport(channel, SSTR("shm://localhost:" << BUFSIZE));
	getchar();
}


char* buf = NULL;
void subscribe_out(const char* channel) {
	buf = new char[BUFSIZE];
	int fd = pubsub::subscribe_out(channel, "shm://");
	for(int i=0; i<BUFSIZE; i++) buf[i] = i%255;
	for(;;) {
		pubsub::send(fd, buf, BUFSIZE);
	}
}


int main(int argc, char **argv) {
	try {
	if(argc<=1) return USAGE();

	if(!strcmp(argv[1], "out")) {
		pubsub::add_host("localhost:12212");
		if(argc>=3) BUFSIZE = atoi(argv[2]);
		subscribe_out("__throughput");
	}
	else publish_in("__throughput");

	return 0;
	} catch(const char* e) { printf("ERROR : %s\n", e); }
}
