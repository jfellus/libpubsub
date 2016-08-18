/*
 * common.h
 *
 *  Created on: 18 août 2016
 *      Author: jfellus
 */

#ifndef SRC_COMMON_H_
#define SRC_COMMON_H_

#include "utils/utils.h"

namespace pubsub {

typedef void (*DataCallback) (const char* buf, size_t len);


void init();

}



#endif /* SRC_COMMON_H_ */
