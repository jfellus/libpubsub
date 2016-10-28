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

#include "protocols/protocols.h"
#include "mesh.h"
#include "signaling.h"


using namespace std;

namespace pubsub {

class Subscription;
class Channel;

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


////


Channel* get_or_create_channel(const string& name);
int get_channel_index(Channel* c);
void close_all_channels(Host* h);

void apply_channel_statement(Host* h, const string& statement);
void apply_publish_statement(Host* h, const string& statement);
void apply_unpublish_statement(Host* h, const string& statement);

void make_offer(Host* h, const string& channel);
void answer_offer(Host* h, const string& offer);


void broadcast_published_channels();




// UTILS
static std::string ENC(const char* str) { return str_replace(str, "/", "@@@"); }


}


#endif /* SRC_CHANNEL_H_ */
