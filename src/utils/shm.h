/*
 * shm.h
 *
 *  Created on: 25 août 2016
 *      Author: jfellus
 */

#ifndef SRC_UTILS_SHM_H_
#define SRC_UTILS_SHM_H_


#include <semaphore.h>
#include <string>
#include <thread>


namespace pubsub {

class SHMClient {
private:
	std::string name;
	int fd;
	void* ptr;
	size_t bufsize;
	sem_t *semRead, *semWrite;
	bool bStop;
	std::thread thread;

public:
	SHMClient(const char* name, bool isInput = false);
	virtual ~SHMClient();

	void scan_infos(const char* str, ...);

	char* get_mem() { return (char*)ptr; }
	void write();
	void write(const char* buf, size_t len);

	virtual void on_receive(char* buf, size_t len) {}

protected:
	void read();
	void read(char* buf, size_t len);

};

class SHMServer {
private:
	std::string name;
	int fd;
	void* ptr;
	size_t bufsize;
	sem_t *semRead, *semWrite;
	bool bStop;
	std::thread thread;
	int* nbClients;

public:
	SHMServer(const char* name, bool isInput, size_t bufsize);
	virtual ~SHMServer();

	void print_infos(const char* str, ...);

	char* get_mem() { return (char*)ptr; }
	void write();
	virtual void write(const char* buf, size_t len);

	virtual void on_receive(char* buf, size_t len) {}

protected:
	void read();
	void read(char* buf, size_t len);

};


}



#endif /* SRC_UTILS_SHM_H_ */
