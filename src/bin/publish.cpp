
#include <libpubsub.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>


//////////////////////////////////////////////////////////
//                                                      //
//          publish utility                             //
//                                                      //
// This program publish an (input or output) channel,   //
// simply taking input from stdin and                   //
// writing output to stdout                             //
//                                                      //
//////////////////////////////////////////////////////////


int USAGE() {
	fprintf(stderr, "USAGE : publish [in|out] <channel_name>\n");
	return 1;
}




void publish_in(const char* channel) {
	int fd = pubsub::publish_in(channel, [&](const char* buf, size_t len) {
		write(1, (const void*) buf, len);
	});

	pubsub::offer_transport(channel, "tcp://");

	getchar();
}


void publish_out(const char* channel) {
	int fd = pubsub::publish_out(channel);

	pubsub::offer_transport(channel, "tcp://");

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


	if(argc<=2) publish_in(argv[1]);
	else if(!strcmp(argv[1], "out")) publish_out(argv[2]);
	else publish_in(argv[2]);

	return 0;
	} catch(const char* e) { printf("ERROR : %s\n", e); }
}
