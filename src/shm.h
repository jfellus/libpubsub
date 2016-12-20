/*
 * shm.h
 *
 *  Created on: 28 oct. 2016
 *      Author: jfellus
 */

#ifndef SRC_SHM_H_
#define SRC_SHM_H_

#include "utils/utils.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <functional>


using namespace std;

namespace pubsub {

/** Synchronous SHM */
class SHM {
public:
	char* shm;
	size_t len;

	std::function<void(const char* msg, size_t len)> cbRecv;

	SHM(char* shm, size_t len) : shm(shm), len(len) {

	}

	virtual ~SHM() {

	}

	void write(const char* msg, size_t len) {
		if(msg == shm) write();
		else {
			memcpy(shm, msg, len);
			write();
		}
	}

	void write() {
		// TODO sem_???
	}
};

}

#endif /* SRC_SHM_H_ */

