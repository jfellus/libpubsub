
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
	fprintf(stderr, "USAGE : publish [-h host1,host2,...] <channel_name>\n");
	return 1;
}


int main(int argc, char **argv) {
	try {
		if(argc<1) return USAGE();

		int n = 0;
		char buf[1024];

//		pubsub::DBG_LEVEL = 1000;

		pubsub::parse_args(argc, argv);


		Channel* c = publish(argv[1]);
		c->on_message = [&](const char* buf, size_t len) {
			write(1, (const void*) buf, len);
		};

		while((n=read(0, buf, 1024)) > 0) c->write(buf, n);

		return 0;
	} catch(const char* e) { printf("ERROR : %s\n", e); }
}
