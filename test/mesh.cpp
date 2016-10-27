/*
 * mesh.cpp
 *
 *  Created on: 18 oct. 2016
 *      Author: jfellus
 */

#include <libpubsub.h>
#include <signaling.h>
#include <channel.h>
#include <stdio.h>

int main(int argc, char **argv) {
	pubsub::DBG_LEVEL = 1;
	pubsub::init();

	int i = 0;
	int j = 0;
	while(1) {
		char c = (char) getchar();
		if(argc>1) {
			if(c == 's') pubsub::subscribe_channel(TOSTRING((char)(argv[1][0]-1) << "_" << j++));
			else pubsub::publish_channel(TOSTRING(argv[1] << "_" << i++));
		}
	}

}
