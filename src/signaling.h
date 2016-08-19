/*
 * signaling.h
 *
 *  Created on: 19 août 2016
 *      Author: jfellus
 */

#ifndef SRC_SIGNALING_H_
#define SRC_SIGNALING_H_

#include "utils/utils.h"
#include "utils/Socket.h"
#include <map>
#include <semaphore.h>

namespace pubsub {

#define SIGNALING_PORT 12212


class SignalingClient;

/**
 * A signaling TCP server that broadcast channel declarations between a set of hosts (peers).
 * Local modifications are notified by means of "commits", that are automatically sent to all
 * participating peers.
 */
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

	/** Wait for all hosts to be up to date with the current local commit */
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



void commit();

extern SignalingServer* signalingServer;

}

#endif /* SRC_SIGNALING_H_ */
