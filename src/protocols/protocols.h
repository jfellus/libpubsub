/*
 * protocols.h
 *
 *  Created on: 18 août 2016
 *      Author: jfellus
 */

#ifndef SRC_PROTOCOLS_PROTOCOLS_H_
#define SRC_PROTOCOLS_PROTOCOLS_H_

#include "../utils/utils.h"
#include "../utils/Socket.h"
#include <functional>
#include <utility>

using namespace std;

namespace pubsub {

#define TRANSPORT_TCP 0

typedef enum { INPUT, OUTPUT, BOTH } EndPointType;

class Client;
class Server;

typedef void (*DataCallback)(const char* msg, size_t len);


#define INVALID_TRANSPORT_DESCRIPTION(channel) TransportDescription(channel)


/** TransportDescriptions contains information on a specific transport protocol for a particular channel */
class TransportDescription {
public:
	bool valid;
	string protocol;
	bool local;
	string ip;
	int port;
	string channel;
	EndPointType type;

	/** Parse a TransportDescription string of the form "protocol://address[:port]"
	 *   (e.g., "tcp://0.0.0.0:1234") */
	TransportDescription(const std::string& channel, string desc) {
		this->channel = channel;
		protocol = str_to_lower(str_before(desc, "://"));
		string url = str_after(desc, "://");
		ip = str_before(str_before(url, ":"), "/");
		if(str_has(url, ":")) port = atoi(str_after(url, ":").c_str());
		else port = 0;

		string details = str_after(url, "/");
		if(str_has(details, "in")) type = INPUT;
		else if(str_has(details, "out")) type = OUTPUT;
		else type = BOTH;

		local = (protocol == "shm");
		if(!local && ip=="") ip = "localhost";

		valid = true;
	}

	TransportDescription(const std::string& channel) { this->channel = channel; valid = false; port = 0; local = false; type = BOTH; }

	operator bool() { return valid; }

	string to_string() {
		if(!valid) return "";
		ostringstream ss;
		ss << protocol << "://" << ip;
		if(port) ss << ":" << port;
		if(type!=BOTH) ss << (type == INPUT ? "/input" : "/output");
		return ss.str();
	}

	Client* create_client();
	Server* create_server();
};





/** Implements a channel Client */
class Client {
public:
	DataCallback cb;
	std::function<void()> on_close;

	Client() : cb(0), on_close(0) {}
	virtual ~Client() {}

	virtual void close() {}

	virtual void send(const char* buf, size_t len) = 0;
};


/** Implements a channel Server */
class Server {
public:
	DataCallback cb;

	Server() : cb(0) {}
	virtual ~Server() {}

	virtual void close() {}

	virtual void send(const char* buf, size_t len) = 0;
};




}

#endif /* SRC_PROTOCOLS_PROTOCOLS_H_ */
