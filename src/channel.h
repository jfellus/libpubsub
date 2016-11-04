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

#include "mesh.h"
#include "signaling.h"
#include "shm.h"
#include "libpubsub.h"


using namespace std;

namespace pubsub {

#define TRANSPORT_TCP 0
#define TRANSPORT_SHM 1

class SubscriptionImpl;
class ChannelImpl;

class SubscriptionImpl : public Subscription {
public:
	ChannelImpl* channel;
	int port;
	bool bConnected;
	int transport;

	TCPSocket* socket;
	SHM* shm;

	SubscriptionImpl(ChannelImpl* channel);
	SubscriptionImpl(ChannelImpl* channel, TCPSocket* socket);
	SubscriptionImpl(ChannelImpl* channel, const char* shm_path);
	virtual ~SubscriptionImpl();

	void connect_tcp(int port);
	void connect_shm(const string& shm_path);
	void close();

	void on_open();
	void on_close();

	void write(const char* msg, size_t len);
	void write(const char* msg) { write(msg, strlen(msg)+1); }
	void write(const string& s) { write(s.c_str()); }

};


class ChannelImpl : public Channel {
public:
	string name;
	Host* publisher;

	TCPServer* server;
	int offeredPort;
	string offeredShmPath;
	vector<SubscriptionImpl*> subscriptions;
	vector<SubscriptionImpl*> subscriptors;


	ChannelImpl(const string& name);
	virtual ~ChannelImpl();

	void publish() {
		setPublisher(Host::me());
		Host::broadcast(TOSTRING("PUBLISH=" << tostring()));
	}

	void unpublish() {
		setPublisher(NULL);
		Host::broadcast(TOSTRING("UNPUBLISH=" << tostring()));
	}

	SubscriptionImpl* subscribe() {
		SubscriptionImpl* s = new SubscriptionImpl(this);
		connect();
		return s;
	}

	void write(const char* msg, size_t len) {
		for(SubscriptionImpl* s : subscriptors) s->write(msg, len);
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
		//if(publisher->is_local()) connect_shm();  // TODO PLUG SHM TRANSPORT HERE
		// else
			connect_tcp();
	}

	void connect_tcp() {
		if(!offeredPort) publisher->send(TOSTRING("CONNECT_TCP=" << name));
		else for(SubscriptionImpl* s : subscriptions) s->connect_tcp(offeredPort);
	}

	void connect_shm() {
		if(!publisher || subscriptions.size()==0 || publisher == Host::me()) return;
		if(offeredShmPath.empty()) publisher->send(TOSTRING("CONNECT_SHM" << name));
		for(SubscriptionImpl* s : subscriptions) s->connect_shm(offeredShmPath);
	}

	void disconnect() {
		for(SubscriptionImpl* s : subscriptions) s->close();
	}

	void offer_tcp(Host* h) {
		if(!server) {
			ChannelImpl* c = this;
			server = new TCPServer(10000,20000);
			server->on_open = [&, c](TCPSocket* s) { new SubscriptionImpl(c, s); };
		}
		h->send(TOSTRING("OFFER_TCP=" << name << "|" << server->port));
	}

	void offer_shm(Host* h) {
		// TODO Create shm path
		offeredShmPath = name;
		h->send(TOSTRING("OFFER_SHM" << name << "|" << offeredShmPath));
	}

	void on_offer_tcp(const string& offer) {
		offeredPort = atoi(offer.c_str());
		connect_tcp();
	}

	void on_offer_shm(const string& offer) {
		offeredShmPath = offer;
		connect_shm();
	}

	string tostring() {
		return TOSTRING( getType() << name << "|" << (publisher ? publisher->tostring() : ""));
	}
};


////


ChannelImpl* get_or_create_channel(const string& name);
int get_channel_index(ChannelImpl* c);
void close_all_channels(Host* h);

void apply_channel_statement(Host* h, const string& statement);
void apply_publish_statement(Host* h, const string& statement);
void apply_unpublish_statement(Host* h, const string& statement);

void make_offer_tcp(Host* h, const string& channel);
void make_offer_shm(Host* h, const string& channel);
void answer_offer_tcp(Host* h, const string& offer);
void answer_offer_shm(Host* h, const string& offer);


void broadcast_published_channels();




// UTILS
static std::string ENC(const char* str) { return str_replace(str, "/", "@@@"); }


}


#endif /* SRC_CHANNEL_H_ */
