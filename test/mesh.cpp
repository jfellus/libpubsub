/*
 * mesh.cpp
 *
 *  Created on: 18 oct. 2016
 *      Author: jfellus
 */

#include <libpubsub.h>
#include <signaling.h>
#include <stdio.h>

int main(int argc, char **argv) {
	pubsub::DBG_LEVEL = 1;
	pubsub::init();

	while(1) {
		getchar();
		pubsub::commit();
	}

}
