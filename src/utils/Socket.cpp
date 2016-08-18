/*
 * Socket.cpp
 *
 *  Created on: 18 août 2016
 *      Author: jfellus
 */

#include "Socket.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

// TCPSocket

TCPSocket::TCPSocket(const char* ip, int port) {
	this->ip = ip;
	this->port = port;
	struct hostent * server = gethostbyname(ip);
	if(!server) throw "Couldn't resolve host";

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) throw "ERROR opening socket";

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(port);

	if (connect(fd,(const sockaddr*)&serv_addr,sizeof(serv_addr)) < 0) throw "ERROR connecting";

	run();
}

TCPSocket::TCPSocket(int fd, const char* ip, int port) {
	this->fd = fd;
	this->ip = ip;
	this->port = port;
	run();
}

void TCPSocket::run() {
	thread = std::thread([&](){
		char buf[1024];
		while(1) {
			int n = read(fd,buf,1024);
			if (n < 0) throw "ERROR reading from socket";
			buf[n] = 0;
			on_receive(buf, n);
		}
	});
}


TCPSocket::~TCPSocket() {
	close(fd);
}


void TCPSocket::write(const char* buf, size_t len) {
	int n = ::write(fd, (const void*) buf, len);
	if (n < 0) throw "ERROR writing to socket";
}



// Server

TCPServer::TCPServer(int port) {
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) throw "ERROR opening socket";

	struct sockaddr_in serv_addr, cli_addr;
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port);
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) throw "ERROR on binding";
	listen(sockfd,1024);

	class Connection : public TCPSocket {
	public:
		TCPServer* server;
		Connection(TCPServer* server, int fd, const char* ip, int port) : TCPSocket(fd,ip,port), server(server) {}
		void on_receive(char* buf, size_t len) {
			server->on_receive(this, buf, len);
		}
	};

	thread = std::thread([&](){
		socklen_t clilen = sizeof(cli_addr);
		while(1) {
			int fd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
			char *ip = inet_ntoa(cli_addr.sin_addr);
			if (fd < 0) throw "ERROR on accept";
			connections.push_back(new Connection(this, fd, ip, cli_addr.sin_port));
		}
	});
}

void TCPServer::broadcast(const char* buf, size_t len) {
	for(TCPSocket* connection : connections) connection->write(buf, len);
}
