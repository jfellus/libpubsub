/*
 * protocols.h
 *
 *  Created on: 18 août 2016
 *      Author: jfellus
 */

#ifndef SRC_PROTOCOLS_PROTOCOLS_H_
#define SRC_PROTOCOLS_PROTOCOLS_H_

#include "../common.h"
#include "../utils/Socket.h"

using namespace std;

namespace pubsub {

#define TRANSPORT_TCP 0


class Client {
public:
	DataCallback cb;
	Client() : cb(0) {}
	virtual ~Client() {}

	virtual void send(const char* buf, size_t len) = 0;
};


class Server {
public:
	DataCallback cb;
	Server() : cb(0) {}
	virtual ~Server() {}

	virtual void send(const char* buf, size_t len) = 0;
};



class TransportDescription {
public:
	bool valid;
	string protocol;
	bool local;
	string ip;
	int port;

	TransportDescription(string desc) {
		protocol = str_to_lower(str_before(desc, "://"));
		string url = str_after(desc, "://");
		ip = str_before(url, ":");
		if(str_has(url, ":")) port = atoi(str_after(url, ":").c_str());
		else port = 0;

		local = (protocol == "shm");
		if(!local && ip=="") ip = "localhost";

		valid = true;
	}

	TransportDescription() { valid = false; port = 0; local = false; }

	operator bool() { return valid; }

	string to_string() {
		if(!valid) return "";
		if(port) return TOSTRING(protocol << "://" << ip << ":" << port);
		else return TOSTRING(protocol << "://" << ip);
	}

	Client* create_client();
	Server* create_server();
};

#define INVALID_TRANSPORT_DESCRIPTION TransportDescription()



// TCP transport

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
	ClientTCP(const char* ip, int port) : TCPSocket(ip, port) {}
	virtual ~ClientTCP() {}

	virtual void send(const char* buf, size_t len) { write(buf, len); }

	virtual void on_receive(char* buf, size_t len) { if(cb) cb(buf, len); }

};





}

#endif /* SRC_PROTOCOLS_PROTOCOLS_H_ */
