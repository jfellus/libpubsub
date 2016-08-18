/*
 * Socket.h
 *
 *  Created on: 18 août 2016
 *      Author: jfellus
 */

#ifndef SRC_UTILS_SOCKET_H_
#define SRC_UTILS_SOCKET_H_

#include <thread>
#include <vector>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

class TCPSocket;

class TCPServer {
public:
	std::thread thread;
	int sockfd;
	std::vector<TCPSocket*> connections;
public:
	TCPServer(int port);
	virtual ~TCPServer() {}

	void broadcast(const char* buf) { broadcast(buf, strlen(buf)+1); }
	void broadcast(const char* buf, size_t len);

	virtual void on_receive(TCPSocket* connection, char* buf, size_t len) {}

};

class TCPSocket {
public:
	std::string ip;
	int port;
	std::thread thread;
	int fd;
	struct sockaddr_in serv_addr;
public:
	TCPSocket(const char* ip, int port);
	TCPSocket(int fd, const char* ip, int port);
	virtual ~TCPSocket();

	virtual void on_receive(char* buf, size_t len) = 0;

	void write(const char* buf) { write(buf, strlen(buf)+1); }
	void write(const char* buf, size_t len);

protected:
	void run();
};

#endif /* SRC_UTILS_SOCKET_H_ */
