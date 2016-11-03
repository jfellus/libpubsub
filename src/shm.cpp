/*
 * shm.cpp
 *
 *  Created on: 28 oct. 2016
 *      Author: jfellus
 */


#include "shm.h"
#include <semaphore.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>

namespace pubsub {


static size_t get_file_size(int fd) {
	struct stat s;
	fstat(fd, &s);
	return s.st_size;
}



char* SHM::open(size_t bufsize) {
	this->path = path;
	this->bufsize = bufsize;

	fd = shm_open(SSTR("/" << path), O_CREAT | O_RDWR | O_TRUNC, 0666);
	if(bufsize) ftruncate(fd, bufsize + sizeof(int));
	else this->bufsize = get_file_size(fd);

	buf = (char*) mmap(0, this->bufsize + sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (buf == MAP_FAILED) { close(fd); throw("mmap"); }

	return buf;
}


}
