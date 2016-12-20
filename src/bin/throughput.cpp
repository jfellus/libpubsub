
#include <libpubsub.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <thread>

////////////////////////////////////////////////////////////////
//                                                            //
//          throughput utility                                //
//                                                            //
// This program publishes an input channel                    //
// and monitor the throughput flowing in input                //
//                                                            //
////////////////////////////////////////////////////////////////


int BUFSIZE = 512000;

int USAGE() {
	fprintf(stderr, "USAGE : throughput <in|out [bufsize]>\n");
	return 1;
}



static std::thread th;
long bytes = 0;
void publish_in(const char* channel) {
	th = std::thread([&]() {
		for(;;) {
			bytes = 0;
			sleep(1);
			float kb = bytes/8.0/1024.0;
			if(kb < 1000) printf("%0.2fko/s\n", kb);
			else {
				float mb = kb/1024.0;
				if(mb < 1000) {
					printf("%0.2fMo/s\n", mb);
				}
				else printf("%0.2fGo/s\n", mb/1024.0);
			}
		}
	});

	Channel* c = publish(channel);
	c->on_message = [&](const char* buf, size_t len) {
		bytes += len;
	};
	getchar();
}


void subscribe_out(const char* channel) {
	Subscription* s = subscribe(channel);
	char* buf = new char[BUFSIZE];
	for(int i=0; i<BUFSIZE; i++) buf[i] = i%255;
	buf[BUFSIZE-1] = '\n';
	for(;;) {
		s->write(buf, BUFSIZE);
	}
}


int main(int argc, char **argv) {
	try {
	if(argc<=1) return USAGE();
	pubsub::DBG_LEVEL = 1;

	pubsub::parse_args(argc, argv);


	if(!strcmp(argv[1], "out")) {
		if(argc>=3) BUFSIZE = atoi(argv[2]);
		subscribe_out("__throughput");
	}
	else publish_in("__throughput");

	return 0;
	} catch(const char* e) { printf("ERROR : %s\n", e); }
}
