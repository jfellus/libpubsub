/*
 * signaling.h
 *
 *  Created on: 28 oct. 2016
 *      Author: jfellus
 */

#ifndef SRC_SIGNALING_H_
#define SRC_SIGNALING_H_

#include "mesh.h"

namespace pubsub {

void on_host_open(Host* h);
void on_host_close(Host* h);
void digest_message(Host* h, string& s);


}

#endif /* SRC_SIGNALING_H_ */
