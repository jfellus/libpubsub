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
#include <functional>

// TCPSocket

TCPSocket::TCPSocket() : mut(PTHREAD_MUTEX_INITIALIZER) {
	sem_init(&semConnected, 0, 0);
	bReconnect = true;
	this->ip = "";
	this->port = 0;
	bStop = true;
	fd = -1;
	isClient = true;
}

TCPSocket::TCPSocket(const char* ip, int port) : mut(PTHREAD_MUTEX_INITIALIZER) {
	sem_init(&semConnected, 0, 0);
	bReconnect = true;
	bStop = true;
	connect(ip, port);
}

TCPSocket::TCPSocket(int fd, const char* ip, int port, bool bAutorun) {
	sem_init(&semConnected, 0, 0);
	bReconnect = true;
	this->fd = fd;
	this->ip = ip;
	this->port = port;
	bStop = true;
	isClient = false;
	int one = 1;
	setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));

	if(bAutorun) run();
}

void TCPSocket::connect(const char* ip, int port) {
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

	isClient = true;

	run();
}

void TCPSocket::run() {
	thread = std::thread([&](){
		do {
			if(isClient) {
				while (::connect(fd,(const sockaddr*)&serv_addr,sizeof(serv_addr)) < 0) usleep(100000);
				int one = 1;
				setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
			}
			char buf[5024];
			bStop = false;

			sem_post(&semConnected);
			if(on_open) on_open();

			while(!bStop) {
				int n = recv(fd, buf, 5024, 0);
				if(n<=0) break;
				for(int i=0; i<n; i++) {
					int j;
					for(j=i; j<n; j++) if(!buf[j] || buf[j]=='\n') break;
					if(j==n) std::cerr << "[tcp-recv] Error : received message doesn't fit in " << 5024 << "bytes buffer\n";
					if(j-i) {
						if(cbRecv) cbRecv(&buf[i], j-i);
						else on_receive(&buf[i], j-i);
					}
					i = j;
				}
			}
			::close(fd);

			if(on_close) on_close();
			if(bReconnect && isClient) {
				struct hostent * server = gethostbyname(ip.c_str());
				if(!server) throw "Couldn't resolve host";
				fd = socket(AF_INET, SOCK_STREAM, 0);
				if (fd < 0) throw "ERROR opening socket";

				bzero((char *) &serv_addr, sizeof(serv_addr));
				serv_addr.sin_family = AF_INET;
				bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
				serv_addr.sin_port = htons(port);

				isClient = true;
			}
		} while(bReconnect && isClient);
	});
}



TCPSocket::~TCPSocket() {
	close();
}

bool TCPSocket::wait_connected() {
	sem_wait(&semConnected);
	return true;
}

void TCPSocket::close(bool bDontReconnect) {
	if(bDontReconnect) bReconnect = false;
	bStop = true;
	::close(fd);
}


bool TCPSocket::write(const char* buf, size_t len) {
	if(bStop) {std::cerr << "[tcp-send] " << ip << ":" << port << " Not ready\n"; return false;}
	pthread_mutex_lock(&mut);
	int n = send(fd, buf, len, MSG_NOSIGNAL);
	pthread_mutex_unlock(&mut);
	if(n!=len) {std::cerr << "[tcp-send] " << ip << ":" << port << " Couldn't send " << len << " bytes (only " << n << ") sent\n"; return false;}
	return true;
}

void TCPSocket::remove_listeners() {
	on_close = NULL;
	on_open = NULL;
	cbRecv = NULL;
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
	bStop = true;
	for(TCPSocket* s : connections) s->close();
	::close(sockfd);
}

void TCPServer::bind(int port) {
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) throw "ERROR opening socket";

	int one = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
	setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));

	struct sockaddr_in serv_addr, cli_addr;
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port);

	if (::bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) throw "ERROR on binding";
	listen(sockfd,1024);


	this->port = port;
}

void TCPServer::run() {
	bStop = false;
	thread = std::thread([&](){

		try {
		struct sockaddr_in cli_addr;
		socklen_t clilen = sizeof(cli_addr);
		while(!bStop) {
			int fd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
			if (fd < 0) throw "ERROR on accept";
			char ip[256]; strcpy(ip, inet_ntoa(cli_addr.sin_addr));

			TCPSocket* c = new TCPSocket(fd, ip, cli_addr.sin_port, false);
			if(on_open) on_open(c);
			std::function<void()> _on_close = c->on_close;
			c->on_close = [&,_on_close]() { if(_on_close) _on_close(); vector_remove(connections, (TCPSocket*)c); };
			connections.push_back(c);
			c->run();
		}

		::close(sockfd);

		} catch(const char* e) {printf("ERROR : %s\n", e);}
	});
}

void TCPServer::broadcast(const char* buf, size_t len) {
	for(TCPSocket* connection : connections) connection->write(buf, len);
}
