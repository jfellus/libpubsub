/*
 * signaling.h
 *
 *  Created on: 19 août 2016
 *      Author: jfellus
 */

#ifndef SRC_SIGNALING_H_
#define SRC_SIGNALING_H_

#include "utils/utils.h"
#include "utils/websocket.h"
#include "utils/Socket.h"
#include <map>
#include <semaphore.h>

namespace pubsub {

#define SIGNALING_DEFAULT_PORT 12212
#define SIGNALING_WEBSOCKET_PORT 12312


class SignalingClient;
class SignalingWebsocketServer;

/**
 * A signaling TCP server that broadcast channel declarations between a set of hosts (peers).
 * Local modifications are notified by means of "commits", that are automatically sent to all
 * participating peers.
 */
class SignalingServer : public TCPServer {
private:
	SignalingWebsocketServer* websocketServer;

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
	SignalingClient(SignalingServer* server, const char* ip, int port = SIGNALING_DEFAULT_PORT) : TCPSocket(ip, port) {
		this->server = server;
	}

	virtual void on_receive(char* buf, size_t len) {
		server->on_receive(this, buf, len);
	}

};


class SignalingWebsocketPeer : public IWebSocketPeer {
public:
	SignalingWebsocketPeer(struct libwebsocket *ws);

	virtual ~SignalingWebsocketPeer() {}

	virtual void onMessage(char* msg) {
		printf("MSG (%lu) : %s\n", (long)this, msg);
	}
};

class SignalingWebsocketServer : public IWebSocketServer {
public:
	SignalingWebsocketServer() : IWebSocketServer(SIGNALING_WEBSOCKET_PORT) {}
	virtual IWebSocketPeer* createPeer(struct libwebsocket *ws) { return new SignalingWebsocketPeer(ws); }
};



void commit();

extern SignalingServer* signalingServer;

}

#endif /* SRC_SIGNALING_H_ */
