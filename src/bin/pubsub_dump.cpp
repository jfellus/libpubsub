
#include <libpubsub.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>


//////////////////////////////////////////////////////////
//                                                      //
//          libpubsub dump utility                      //
//                                                      //
// This program displays information about              //
// libpubsub channels                                   //
//
//                                                      //
//////////////////////////////////////////////////////////


int USAGE() {
	fprintf(stderr, "USAGE : pubsub_dump\n");
	return 1;
}




int main(int argc, char **argv) {
	try {

		pubsub::parse_args(argc, argv);


		pubsub::dump_hosts();
		pubsub::dump_channels();

		return 0;
	} catch(const char* e) { printf("ERROR : %s\n", e); }
}
