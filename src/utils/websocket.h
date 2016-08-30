/*
 * websocket.h
 *
 *  Created on: 14 juil. 2016
 *      Author: jfellus
 */

#ifndef SRC_UTIL_WEBSOCKET_H_
#define SRC_UTIL_WEBSOCKET_H_

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <libwebsockets.h>
#include <unordered_map>
#include <thread>

// Data structures

class IWebSocketPeer;
class IWebSocketServer;


/////////////////////
// WebSocketServer //
/////////////////////


class IWebSocketServer {
public:
	int bQuit = 0;
	struct libwebsocket_context *context;
	std::thread thread;

public:
	IWebSocketServer(int port);
	virtual ~IWebSocketServer() { close(); }

	virtual IWebSocketPeer* createPeer(struct libwebsocket *ws) = 0;

	void close() { bQuit = 1; }

	void start();
	virtual void run();
};



////////////////////
// IWebSocketPeer //
////////////////////

class IWebSocketPeer {
	struct libwebsocket *ws;
public:
	IWebSocketPeer(struct libwebsocket *ws) : ws(ws) {}
	virtual ~IWebSocketPeer() {}

	virtual void onMessage(char* msg) = 0;

	int send(const char* msg);
};


#endif /* SRC_UTIL_WEBSOCKET_H_ */
