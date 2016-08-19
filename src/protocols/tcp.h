/*
 * tcp.h
 *
 *  Created on: 19 août 2016
 *      Author: jfellus
 */

#ifndef SRC_PROTOCOLS_TCP_H_
#define SRC_PROTOCOLS_TCP_H_


#include "protocols.h"


// Implementation of TCP transport protocol

namespace pubsub {


class ServerTCP : public Server, public TCPServer {
public:
	ServerTCP(int port) : TCPServer(port) {}
	virtual ~ServerTCP() {}

	virtual void send(const char* buf, size_t len) {
		broadcast(buf, len);
	}

	virtual void on_receive(TCPSocket* connection, char* buf, size_t len) { if(cb) cb(buf, len); }

};

class ClientTCP : public Client, public TCPSocket {
public:
	ClientTCP(const char* ip, int port) : TCPSocket(ip, port) {
		TCPSocket::on_close = [&]() { Client::on_close(); };
	}
	virtual ~ClientTCP() {}

	virtual void send(const char* buf, size_t len) { write(buf, len); }

	virtual void on_receive(char* buf, size_t len) { if(cb) cb(buf, len); }

};


}




#endif /* SRC_PROTOCOLS_TCP_H_ */
