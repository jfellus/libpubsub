
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
	fprintf(stderr, "USAGE : subscribe <channel_name>\n");
	return 1;
}


int main(int argc, char **argv) {
	try {
		if(argc<1) return USAGE();

		int n = 0;
		char buf[1024];
//		pubsub::DBG_LEVEL = 1000;

		pubsub::parse_args(argc, argv);

		Subscription* s = subscribe(argv[1]);
		s->on_message = [&](const char* buf, size_t len) {
			write(1, (const void*) buf, len);
		};

		while((n=read(0, buf, 1024)) > 0) s->write(buf, n);

		return 0;
	} catch(const char* e) { printf("ERROR : %s\n", e); }
}
