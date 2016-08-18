/*
 * hosts.h
 *
 *  Created on: 18 août 2016
 *      Author: jfellus
 */

#ifndef SRC_HOSTS_H_
#define SRC_HOSTS_H_

#include "utils/Socket.h"

namespace pubsub {


#define SIGNALING_PORT 12212



void broadcast_published_channels();

void register_host(const char* url);
bool has_host(const char* url);


class SignalingServer : public TCPServer {
public:
	SignalingServer() : TCPServer(SIGNALING_PORT) {
		// printf("[signaling] Initialize signaling server on port %u\n", SIGNALING_PORT);
	}
	virtual ~SignalingServer() {}

	virtual void on_receive(TCPSocket* connection, char* buf, size_t len);
};



}

#endif /* SRC_HOSTS_H_ */
