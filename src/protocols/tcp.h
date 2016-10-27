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
		bReconnect = false;
		TCPSocket::on_close = [&]() { Client::on_close(); };
	}
	virtual ~ClientTCP() {}

	virtual void send(const char* buf, size_t len) { write(buf, len); }

	virtual void on_receive(char* buf, size_t len) { if(cb) cb(buf, len); }

};



// UTILS

static int get_free_port(int firstPort, const char* channel) {
	char path[512];
	char _channel[512];
	system("mkdir -p /tmp/.libpubsub/ports; chmod a+wxr /tmp/.libpubsub/ports");
	int i;
	for(i = firstPort; ; i++) {
		sprintf(path, "/tmp/.libpubsub/ports/%u", i);
		FILE* f = fopen(path, "r");
		if(!f) break;
		fgets(_channel, 512, f);
		if(!strcmp(_channel, channel)) { fclose(f); break; }
		fclose(f);
	}
	FILE *f = fopen(path, "w");
	fprintf(f, "%s", channel);
	fclose(f);
	sprintf(_channel, "chmod a+wr %s", path);
	system(_channel);
	return i;
}



}




#endif /* SRC_PROTOCOLS_TCP_H_ */
