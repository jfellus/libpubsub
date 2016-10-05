/*
 * shm.cpp
 *
 *  Created on: 25 août 2016
 *      Author: jfellus
 */

#include "shm.h"

#include <semaphore.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include "utils.h"

namespace pubsub {

// SHMServer

SHMServer::SHMServer(const char* name, bool isInput, size_t bufsize) {
	this->name = name;
	this->bufsize = bufsize;
	print_infos("%lu", bufsize);

	fd = shm_open(SSTR("/" << name), O_CREAT | O_RDWR | O_TRUNC, 0666);
	ftruncate(fd, bufsize + sizeof(int));
	ptr = mmap(0, bufsize + sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (ptr == MAP_FAILED) { close(fd); throw("mmap"); }

	sem_unlink(SSTR("/semread_" << name));
	sem_unlink(SSTR("/semwrite_" << name));
	semRead = sem_open(SSTR("/semread_" << name), O_CREAT, S_IRUSR | S_IWUSR, 0);
	semWrite = sem_open(SSTR("/semwrite_" << name), O_CREAT, S_IRUSR | S_IWUSR, 0);
	sem_post(semWrite);

	nbClients = (int*)&((char*)ptr)[bufsize];

	if(isInput) {
		bStop = false;
		thread = std::thread([&]() {
			while(!bStop) { read(); }
		});
	}
}

SHMServer::~SHMServer() {
	bStop = true;
	munmap(ptr, bufsize);
	shm_unlink(SSTR("/" << name));
	sem_destroy(semWrite);
	sem_destroy(semRead);
	sem_close(semWrite);
	sem_close(semRead);
	sem_unlink(SSTR("/semread_" << name));
	sem_unlink(SSTR("/semwrite_" << name));
	unlink(SSTR("/tmp/shm_" << name << "_infos"));
}

void SHMServer::print_infos(const char* str, ...) {
	va_list vl;
	va_start(vl,str);
	FILE* fInfos = fopen(SSTR("/tmp/shm_" << name << "_infos"), "w");
	vfprintf(fInfos, str, vl);
	fclose(fInfos);
	va_end(vl);
}

void SHMServer::write(const char* buf, size_t len) {
	sem_wait(semWrite);
	memcpy(ptr, buf, len);
	sem_post(semRead);
}

void SHMServer::write() {
	sem_wait(semWrite);
	sem_post(semRead);
}


void SHMServer::read() {
	sem_wait(semRead);
	on_receive((char*)ptr, bufsize);
	sem_post(semWrite);
}

void SHMServer::read(char* buf, size_t len) {
	sem_wait(semRead);
	memcpy(buf, ptr, len);
	sem_post(semWrite);
}

// SHMClient

SHMClient::SHMClient(const char* name, bool isInput) {
	this->name = name;
	scan_infos("%lu", &bufsize);

	if(!bufsize) { fprintf(stderr, "Unspecified size for shm channel %s\n", name); }
	fd = shm_open(SSTR("/" << name), O_CREAT | O_RDWR, 0666);
	ptr = mmap(0, bufsize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (ptr == MAP_FAILED) { close(fd); throw("mmap"); }

	semRead = sem_open(SSTR("/semread_" << name), O_CREAT, S_IRUSR | S_IWUSR, 0);
	semWrite = sem_open(SSTR("/semwrite_" << name), O_CREAT, S_IRUSR | S_IWUSR, 1);

	bStop = false;

	if(isInput) {
		thread = std::thread([&]() {
			while(!bStop) { read(); }
		});
	}
}

SHMClient::~SHMClient() {
	bStop = true;
	munmap(ptr, bufsize);
	sem_destroy(semWrite);
	sem_destroy(semRead);
	sem_close(semWrite);
	sem_close(semRead);
}

void SHMClient::scan_infos(const char* str, ...) {
	va_list vl;
	va_start(vl,str);
	FILE* fInfos = fopen(SSTR("/tmp/shm_" << name << "_infos"), "r");
	vfscanf(fInfos, str, vl);
	fclose(fInfos);
	va_end(vl);
}

void SHMClient::write(const char* buf, size_t len) {
	sem_wait(semWrite);
	memcpy(ptr, buf, len);
	sem_post(semRead);
}

void SHMClient::write() {
	sem_wait(semWrite);
	sem_post(semRead);
}

void SHMClient::read() {
	sem_wait(semRead);
	on_receive((char*)ptr, bufsize);
	sem_post(semWrite);
}

void SHMClient::read(char* buf, size_t len) {
	sem_wait(semRead);
	memcpy(buf, ptr, len);
	sem_post(semWrite);
}

}
