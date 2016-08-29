/*
 * pipe.cpp
 *
 *  Created on: 29 août 2016
 *      Author: jfellus
 */

#include "pipe.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../utils/utils.h"
#include <linux/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

namespace pubsub {

ServerPipe::ServerPipe(const char* channel, bool isInput, size_t bufsize) {
	filename = SSTR("/tmp/" <<  channel << ".pub");
	this->bufsize = bufsize;
	mode_t m = umask(0);
	mknod(filename.c_str(), S_IFIFO|0666, 0);
	umask(m);

	if(isInput) {
		bStop = false;
		thread = std::thread([&]() {
			if ((fd = open(filename.c_str(), O_RDONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) == -1) {
				printf("Couldn't open fifo %s : %s\n", filename.c_str(), strerror(errno));
				return;
			}
			printf("ok\n");
			char* buf = new char[this->bufsize];
			while(!bStop) {
				read(fd, buf, this->bufsize);
				on_receive(buf, this->bufsize);
			}
		});
	} else {
		if ((fd = open(filename.c_str(), O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) == -1)
			throw("Cannot open output file\n");
	}
}

void ServerPipe::send(const char* buf, size_t len) {
	write(fd, buf, len);
}

ClientPipe::ClientPipe(const char* channel, bool isInput, size_t bufsize) {
	filename = SSTR("/tmp/" <<  channel << ".pub");
	if(isInput) {
		if ((fd = open(filename.c_str(), O_RDONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) == -1)
			throw("Cannot open fifo\n");
		bStop = false;
		thread = std::thread([&]() {
			char* buf = new char[bufsize];
			while(!bStop) {
				read(fd, buf, bufsize);
				on_receive(buf, bufsize);
			}
		});
	} else {
		if ((fd = open(filename.c_str(), O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) == -1)
			throw("Cannot open fifo\n");
	}
}

void ClientPipe::send(const char* buf, size_t len) {
	write(fd, buf, len);
}

}
