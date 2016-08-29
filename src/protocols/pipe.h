/*
 * pipe.h
 *
 *  Created on: 29 août 2016
 *      Author: jfellus
 */

#ifndef SRC_PROTOCOLS_PIPE_H_
#define SRC_PROTOCOLS_PIPE_H_

#include "protocols.h"
#include <thread>

namespace pubsub {


class ServerPipe : public Server {
public:
	std::string filename;
	std::thread thread;
	size_t bufsize;
	bool bStop;
	int fd;
public:
	ServerPipe(const char* channel, bool isInput, size_t bufsize);

	virtual void send(const char* buf, size_t len);

	virtual void on_receive(const char* buf, size_t len) { if(cb) cb(buf, len); }

};

class ClientPipe : public Client {
public:
	std::string filename;
	std::thread thread;
	bool bStop;
	int fd;
public:
	ClientPipe(const char* channel, bool isInput, size_t bufsize);

	virtual void send(const char* buf, size_t len);

	virtual void on_receive(const char* buf, size_t len) { if(cb) cb(buf, len); }

};



}



#endif /* SRC_PROTOCOLS_PIPE_H_ */
