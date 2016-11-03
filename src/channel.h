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
#include "shm.h"


using namespace std;

namespace pubsub {

#define TRANSPORT_TCP 0
#define TRANSPORT_SHM 1

class Subscription;
class Channel;

class Subscription {
public:
	Channel* channel;
	int port;
	bool bConnected;
	int transport;

	TCPSocket* socket;
	SHM* shm;

	Subscription(Channel* channel);
	Subscription(Channel* channel, TCPSocket* socket);
	Subscription(Channel* channel, const char* shm_path);
	virtual ~Subscription();

	void connect_tcp(int port);
	void connect_shm(const string& shm_path);
	void close();

	void on_open();
	void on_close();
	std::function<void(const char* msg, size_t len)> on_message;

	void write(const char* msg, size_t len);
	void write(const char* msg) { write(msg, strlen(msg)+1); }
	void write(const string& s) { write(s.c_str()); }

};


class Channel {
public:
	string name;
	Host* publisher;

	TCPServer* server;
	int offeredPort;
	string offeredShmPath;
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
		if(publisher->is_local()) connect_shm();
		else connect_tcp();
	}

	void connect_tcp() {
		if(!offeredPort) publisher->send(TOSTRING("CONNECT_TCP=" << name));
		else for(Subscription* s : subscriptions) s->connect_tcp(offeredPort);
	}

	void connect_shm() {
		if(!publisher || subscriptions.size()==0 || publisher == Host::me()) return;
		if(offeredShmPath.empty()) publisher->send(TOSTRING("CONNECT_SHM" << name));
		for(Subscription* s : subscriptions) s->connect_shm(offeredShmPath);
	}

	void disconnect() {
		for(Subscription* s : subscriptions) s->close();
	}

	void offer_tcp(Host* h) {
		if(!server) {
			Channel* c = this;
			server = new TCPServer(10000,20000);
			server->on_open = [&, c](TCPSocket* s) { new Subscription(c, s); };
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


Channel* get_or_create_channel(const string& name);
int get_channel_index(Channel* c);
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
