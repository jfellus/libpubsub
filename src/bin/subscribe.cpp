
#include <libpubsub.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


////////////////////////////////////////////////////////////////
//                                                            //
//          subscribe utility                                 //
//                                                            //
// This program subscribes to an (input or output) channel,   //
// simply taking input from stdin and                         //
// writing output to stdout                                   //
//                                                            //
////////////////////////////////////////////////////////////////


int USAGE() {
	fprintf(stderr, "USAGE : subscribe [in|out] <channel_name>\n");
	return 1;
}


void subscribe_in(const char* channel) {
	int fd = pubsub::subscribe_in(channel, "tcp://", [&](const char* buf, size_t len) {
		write(1, (const void*) buf, len);
	});

	getchar();
}


void subscribe_out(const char* channel) {
	int fd = pubsub::subscribe_out(channel, "tcp://");
	int n = 0;
	char buf[1024];
	while((n=read(0, buf, 1024)) > 0) {
		pubsub::send(fd, buf, n);
	}
}


int main(int argc, char **argv) {
	try {
	if(argc<=1) return USAGE();

	pubsub::add_host("localhost:12212");

	if(argc<=2) subscribe_in(argv[1]);
	else if(!strcmp(argv[1], "out")) subscribe_out(argv[2]);
	else subscribe_in(argv[2]);

	return 0;
	} catch(const char* e) { printf("ERROR : %s\n", e); }
}
