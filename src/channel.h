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
#include "mesh.h"
#include "signaling.h"


using namespace std;

namespace pubsub {

class Subscription;
class Channel;

Subscription* subscribe_channel(const string& name);
Channel* publish_channel(const string& name);

void apply_channel_statement(Host* h, const string& statement);
void apply_publish_statement(Host* h, const string& statement);
void apply_unpublish_statement(Host* h, const string& statement);
void close_all_channels(Host* h);

void broadcast_published_channels();

void make_offer(Host* h, const string& channel);
void answer_offer(Host* h, const string& offer);




class Subscription {
public:
	Channel* channel;
	int port;
	TCPSocket* socket;
	bool bConnected;

	Subscription(Channel* channel);
	Subscription(Channel* channel, TCPSocket* socket);
	virtual ~Subscription();

	void connect(int port);
	void close();

	void on_open();
	void on_close();
	std::function<void(const char* msg, size_t len)> on_message;

	void write(const char* msg, size_t len);
	void write(const char* msg) { write(msg, strlen(msg)+1); }
	void write(const string& s) { write(s.c_str()); }


protected:
	void init();

};


class Channel {
public:
	string name;
	Host* publisher;

	TCPServer* server;
	int offeredPort;
	vector<Subscription*> subscriptions;
	vector<Subscription*> subscriptors;

	std::function<void(const char* msg, size_t len)> on_message;


	Channel(const string& name);
	virtual ~Channel();

	void publish() {
		setPublisher(Host::me());
		Host::broadcast(TOSTRING("PUBLISH=" << tostring()));
	}

	void unpublish() {
		setPublisher(NULL);
		Host::broadcast(TOSTRING("UNPUBLISH=" << tostring()));
	}

	Subscription* subscribe() {
		Subscription* s = new Subscription(this);
		connect();
		return s;
	}

	void write(const char* msg, size_t len) {
		for(Subscription* s : subscriptors) s->write(msg, len);
	}
	void write(const char* msg) { write(msg, strlen(msg)+1); }
	void write(const string& s) { write(s.c_str()); }


	void setPublisher(Host* p) {
		if(publisher && publisher != p) disconnect();
		publisher = p;
		offeredPort = 0;
		if(subscriptions.size() && publisher) connect();
	}

	char getType() {
		if(publisher == Host::me()) return 'P'; // Published
		else if(subscriptions.size()) return 'S'; // Subscribed
		else if(publisher) return 'A'; // Available
		else return 'N'; // Not available
	}

	void connect() {
		if(!publisher || subscriptions.size()==0 || publisher == Host::me()) return;
		if(!offeredPort) publisher->send(TOSTRING("CONNECT=" << name));
		else {
			for(Subscription* s : subscriptions) s->connect(offeredPort);
		}
	}

	void disconnect() {
		for(Subscription* s : subscriptions) s->close();
	}

	void offer(Host* h) {
		if(!server) {
			Channel* c = this;
			server = new TCPServer(10000,20000);
			server->on_open = [&, c](TCPSocket* s) { new Subscription(c, s); };
		}
		h->send(TOSTRING("OFFER=" << name << "|" << server->port));
	}

	void on_offer(const string& offer) {
		offeredPort = atoi(offer.c_str());
		connect();
	}

	string tostring() {
		return TOSTRING( getType() << name << "|" << (publisher ? publisher->tostring() : ""));
	}
};















///////////////////////////

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

	Host* provider;

	vector<TransportDescription> offeredTransports;
	vector<Server*> servers;
	vector<Client*> clients;

	list<SubscriptionRequest> requested_subscriptions;

	bool bRequested;

public:
	EndPoint(const char* name, EndPointType type = BOTH, DataCallback cb = 0, bool bRequested = false);
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

bool has_endpoint(const char* name, bool bCountRequested = false);
EndPoint* get_endpoint(const char* name);
EndPoint* request_endpoint(const char* name);
bool is_transport_offered(const char* channel, const char* transportDescription);
TransportDescription find_matching_transport(const char* channel, const char* transportDescription);
void close_all_endpoints();



// Debug
void dump_endpoints();

}


#endif /* SRC_CHANNEL_H_ */
