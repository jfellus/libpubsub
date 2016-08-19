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
#include <netinet/tcp.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "utils.h"
#include <errno.h>

// TCPSocket

TCPSocket::TCPSocket(const char* ip, int port) : mut(PTHREAD_MUTEX_INITIALIZER) {
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
	int one = 1;
	setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));


	run();
}

TCPSocket::TCPSocket(int fd, const char* ip, int port) {
	this->fd = fd;
	this->ip = ip;
	this->port = port;
	int one = 1;
	setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));

	run();
}

void TCPSocket::run() {
	bStop = false;
	thread = std::thread([&](){
		try {
		char buf[1024];
		while(!bStop) {
			int n = recv(fd,buf,1024, 0);
			if (n < 0) throw "ERROR reading from socket";
			else if(n == 0) break;
			buf[n] = 0;
			for(int i=0; i<n; i++) {
				on_receive(&buf[i], n);
				while(buf[i]) i++;
			}
		}
		::close(fd);
		if(on_close) on_close();

		} catch(const char* e) {printf("ERROR : %s (%s)\n", e, strerror(errno));}
	});
}


TCPSocket::~TCPSocket() {
	close();
}

void TCPSocket::close() {
	bStop = true;
	::close(fd);
}


void TCPSocket::write(const char* buf, size_t len) {
	if(bStop) return;
	pthread_mutex_lock(&mut);
	int n = ::send(fd, (const void*) buf, len, MSG_NOSIGNAL);
	if (n < 0) throw "ERROR writing to socket";
	pthread_mutex_unlock(&mut);
}



// Server

TCPServer::TCPServer(int port) {
	bind(port);
	run();
}

TCPServer::TCPServer(int firstPort, int lastPort) {
	int port;
	for(port = firstPort; port<=lastPort; port++) {
		try {
			bind(port);
			break;
		} catch(const char* e) {}
	}
	if(port>lastPort) throw "No free port";
	run();
}

TCPServer::~TCPServer() {
	close();
}

void TCPServer::close() {
	printf("[tcp] close\n");
	bStop = true;
	::close(sockfd);
	for(TCPSocket* s : connections) s->close();
}

void TCPServer::bind(int port) {
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) throw "ERROR opening socket";

	struct sockaddr_in serv_addr, cli_addr;
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port);

	if (::bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) throw "ERROR on binding";
	listen(sockfd,1024);

	int one = 1;
	setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));

	this->port = port;
}

void TCPServer::run() {
	class Connection : public TCPSocket {
	public:
		TCPServer* server;
		Connection(TCPServer* server, int fd, const char* ip, int port) : TCPSocket(fd,ip,port), server(server) {}
		virtual ~Connection() {}
		virtual void on_receive(char* buf, size_t len) {
			server->on_receive(this, buf, len);
		}
	};

	bStop = false;
	thread = std::thread([&](){

		try {
		struct sockaddr_in cli_addr;
		socklen_t clilen = sizeof(cli_addr);
		while(!bStop) {
			int fd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
			char *ip = inet_ntoa(cli_addr.sin_addr);
			if (fd < 0) throw "ERROR on accept";

			Connection* c = new Connection(this, fd, ip, cli_addr.sin_port);
			c->on_close = [&]() { vector_remove(connections, (TCPSocket*)c); };
			connections.push_back(c);
		}

		::close(sockfd);

		} catch(const char* e) {printf("ERROR : %s\n", e);}
	});
}

void TCPServer::broadcast(const char* buf, size_t len) {
	for(TCPSocket* connection : connections) connection->write(buf, len);
}
