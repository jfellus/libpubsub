/*
 * signaling.h
 *
 *  Created on: 27 oct. 2016
 *      Author: jfellus
 */

#ifndef SRC_SIGNALING_H_
#define SRC_SIGNALING_H_

#include "mesh.h"

namespace pubsub {


// Interface
void broadcast_state();
//////////////

void commit();

void dump_states();


}

#endif /* SRC_SIGNALING_H_ */
