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
#include <functional>
#include <utility>
#include <semaphore.h>
#include <string>

using namespace std;

class TCPSocket;

class TCPServer {
public:
	int port;

	std::thread thread;
	int sockfd;
	std::vector<TCPSocket*> connections;

	bool bStop;
public:
	TCPServer(int port);
	TCPServer(int firstPort, int lastPort); // Find a free port in range [firstPort,lastPort]
	virtual ~TCPServer();

	void close();

	void broadcast(const char* buf) { broadcast(buf, strlen(buf)+1); }
	void broadcast(const char* buf, size_t len);

	std::function<void(TCPSocket* s)> on_open;

protected:
	void bind(int port);
	void run();
};

class TCPSocket {
public:
	std::string ip;
	int port;
	std::thread thread;
	int fd;
	struct sockaddr_in serv_addr;
	bool bStop;
	pthread_mutex_t mut;
	bool isClient;
	sem_t semConnected;
	bool bLinebuffer;
	bool bReconnect;

	std::function<void()> on_open;
	std::function<void()> on_close;
	std::function<void(const char*, size_t)> cbRecv;

public:
	TCPSocket();
	TCPSocket(const char* ip, int port);
	TCPSocket(int fd, const char* ip, int port, bool bAutorun = true);
	virtual ~TCPSocket();

	void connect(const char* ip, int port);
	void reconnect();

	bool wait_connected();
	void close(bool bDontReconnect = false);

	virtual void on_receive(char* buf, size_t len) {}

	bool write(const string& s) { return write(s.c_str()); }
	bool write(const char* buf) { return write(buf, strlen(buf)+1); }
	bool write(const char* buf, size_t len);


	void remove_listeners();
	void run();
};

#endif /* SRC_UTILS_SOCKET_H_ */
