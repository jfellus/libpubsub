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

using namespace std;

namespace pubsub {

class SHM {
public:
	string path;
	char* buf;
	size_t bufsize;
	int fd;

	std::function<void(const char* msg, size_t len)> cbRecv;

	SHM(const string& shmPath, size_t bufsize = 0) {
		path = shmPath;
		buf = open(bufsize);
	}

	virtual ~SHM() {

	}

	void write(const char* msg, size_t len) {
		if(msg == buf) write();
		else {
			memcpy(buf, msg, len);
			write();
		}
	}

	void write() {
		// TODO sem_???
	}

protected:
	char* open(size_t bufsize);
};

}

#endif /* SRC_SHM_H_ */

