/*
 * channel.h
 *
 *  Created on: 18 août 2016
 *      Author: jfellus
 */

#ifndef SRC_CHANNEL_H_
#define SRC_CHANNEL_H_

#include <vector>
#include <string>
#include <list>

#include "libpubsub.h"
#include "protocols/protocols.h"

using namespace std;

namespace pubsub {


typedef std::pair<string, DataCallback> SubscriptionRequest;

/**
 * An EndPoint holds the declaration of a channel (local or remote) with a given name
 * An EndPoint can offer several transport protocols (represented as TransportDescriptions)
 * An EndPoint maintains a set of transport servers and clients for transmitting actual data
 */
class EndPoint {
public:
	string name;
	EndPointType type;

	int fd;

	DataCallback cb;

	vector<TransportDescription> offeredTransports;
	vector<Server*> servers;
	vector<Client*> clients;

	list<SubscriptionRequest> requested_subscriptions;

	bool bRequested;

public:
	EndPoint(const char* name, EndPointType type = BOTH, DataCallback cb = 0);
	virtual ~EndPoint();

	bool is_input() { return type == INPUT; }
	bool is_output() { return type == OUTPUT; }
	bool is_duplex() { return type == BOTH; }

	void offer_transport(const char* transportDescription);
	bool is_transport_offered(const char* transportDescription);
	TransportDescription find_matching_transport(const char* transportDescription);
	bool on_remote_offered_transport(TransportDescription td);
	void realize();

	void subscribe(const char* transportDescription, DataCallback cb = 0);

	void send(const char* buf) { send(buf, strlen(buf)+1); }
	void send(const char* buf, size_t len);
	void on_receive(const char* buf, size_t len) { cb(buf, len); }
};


// Utilities

bool has_endpoint(const char* name);
EndPoint* get_endpoint(const char* name);
EndPoint* request_endpoint(const char* name);
bool is_transport_offered(const char* channel, const char* transportDescription);
TransportDescription find_matching_transport(const char* channel, const char* transportDescription);
void close_all_endpoints();



// Debug
void dump_endpoints();

}


#endif /* SRC_CHANNEL_H_ */
