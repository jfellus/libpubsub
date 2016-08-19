/*
 * hosts.h
 *
 *  Created on: 18 août 2016
 *      Author: jfellus
 */

#ifndef SRC_HOSTS_H_
#define SRC_HOSTS_H_

#include "utils/Socket.h"
#include <map>
#include <semaphore.h>

namespace pubsub {


#define SIGNALING_PORT 12212


void commit();
void broadcast_published_channels();

void register_host(const char* url);
bool has_host(const char* url);


class SignalingClient;

class SignalingServer : public TCPServer {
private:
	std::vector<SignalingClient*> signalingClients;
	std::map<TCPSocket*,int> states;

	sem_t semStates;
public:
	int localCommit;
public:
	SignalingServer();
	virtual ~SignalingServer();

	void connect(const char* ip);
	void close();

	void broadcast(const char* msg);

	void sync();

	virtual void on_receive(TCPSocket* connection, char* buf, size_t len);

	// States

	void set_all_states(int state);
	void wait_all_states(int state);
	void post_state(TCPSocket* connection, int state);

	// Debug

	void dump();
};


class SignalingClient : public TCPSocket {
private:
	SignalingServer* server;
public:
	SignalingClient(SignalingServer* server, const char* ip, int port = SIGNALING_PORT) : TCPSocket(ip, port) {
		this->server = server;
	}

	virtual void on_receive(char* buf, size_t len) {
		server->on_receive(this, buf, len);
	}

};


}

#endif /* SRC_HOSTS_H_ */
