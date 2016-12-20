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
typedef SubscriptionImpl SubscriptorImpl; // This is to make the distinct role of Subscriptions and Subscriptors clear


/** Actual implementation of a Subscription */
class SubscriptionImpl : public Subscription {
public:
	ChannelImpl* channel;
	Host* host;

	bool bAsync = false;
	bool bConnected;
	int transport;

	TCPSocket* socket;
	SHM* shm;

	//////////////////
	// Construction

	/** Creates a new subscription to the given Channel (i.e., the local Host subscribes to the Channel) */
	SubscriptionImpl(ChannelImpl* channel, bool bAsync);

	/** Creates a new subscriptor to the given Channel (the remote Host 'host' subscribes to the Channel through the given socket) */
	SubscriptionImpl(ChannelImpl* channel, TCPSocket* socket, Host* host);

	/** Creates a new subscriptor to the given Channel (the remote Host 'host' subscribes to the Channel using the given SHM segment) */
	SubscriptionImpl(ChannelImpl* channel, const char* shm_path, Host* host);

	virtual ~SubscriptionImpl();


	//////////////////
	// Connection

	/** Connects this Subscription to the Channel's publisher via TCP */
	void connect_tcp();

	/** Connects this Subscription to the Channel's publisher via a SHM segment */
	void connect_shm();

	/** Disconnect this Subscription */
	void close();

	/** Called upon successful connection to the Channel's publisher */
	void on_open();

	/** Called upon successful disconnection from the Channel's publisher */
	void on_close();


	////////////////
	// Write

	/** Send data through this Subscription's connection using Channel's DMA (@see{Channel}) */
	void write();

	/** Send data through this Subscription's connection (by copy, @see{Channel}) */
	void write(const char* msg, size_t len);
	void write(const char* msg) { write(msg, strlen(msg)+1); }
	void write(const string& s) { write(s.c_str()); }

};


/** Actual implementation of a Channel */
class ChannelImpl : public Channel {
public:
	string name;
	Host* publisher;

	TCPServer* server;
	int offeredPort;
	string offeredShmPath;

	vector<SubscriptionImpl*> subscriptions;
	vector<SubscriptorImpl*> subscriptors;

	////////////////

	ChannelImpl(const string& name);
	virtual ~ChannelImpl();

	//////////////////
	// Main API

	/** Tells the network that this host (Host::me()) is going to publish this Channel */
	void publish() {
		if(publisher == Host::me()) return;
		setPublisher(Host::me());
		Host::broadcast(TOSTRING("PUBLISH=" << tostring()));
	}

	/** Tells the network that this host (Host::me()) doesn't publish this Channel anymore */
	void unpublish() {
		setPublisher(NULL);
		Host::broadcast(TOSTRING("UNPUBLISH=" << tostring()));
	}

	/** Request a Subscription to this Channel */
	SubscriptionImpl* subscribe() {
		SubscriptionImpl* s = new SubscriptionImpl(this);
		connect();
		return s;
	}


	/////////////////
	// Allocation

	/** Allocates the Channel's memory segment (for now, every segment is allocated in shared memory) */
	void alloc(size_t size) {
		if(ptr) { fprintf(stderr, "WARNING : channel %s already allocated\n", name.c_str()); return; }
		ptr = SHM::create(name, size);
		_size = size;
		Host::broadcast(TOSTRING("ALLOCATE=" << name << "=" << size));
	}


	////////////////
	// Write

	/** Send this Channel's internal memory segment to all subscriptors */
	void write() {
		for(SubscriptorImpl* s : subscriptors) s->write();
	}

	/** Send existing data buffers to all subscriptors */
	void write(const char* msg, size_t len) {
		for(SubscriptorImpl* s : subscriptors) s->write(msg, len);
	}
	void write(const char* msg) { write(msg, strlen(msg)+1); }
	void write(const string& s) { write(s.c_str()); }


	////////////////////
	// Accessors

	/** Sets this Channel publisher (Host), eventually triggering connection of requested subscriptions */
	void setPublisher(Host* p) {
		if(publisher && publisher != p) disconnect();
		publisher = p;
		offeredPort = 0;
		if(subscriptions.size() && publisher) connect();
	}

	/** @return this Channel's state */
	char getType() {
		if(publisher == Host::me()) return 'P'; // Published
		else if(subscriptions.size()) return 'S'; // Subscribed
		else if(publisher) return 'A'; // Available
		else return 'N'; // Not available
	}


	//////////////////
	// Connection

	// Connects all local subscriptions for this channel to its publisher if it is known */
	void connect() {
		if(!publisher || subscriptions.size()==0 || publisher == Host::me()) return;
		if(publisher->is_local()) connect_shm();
		else connect_tcp();
	}

	// Connects all local subscriptions to a remote publisher (which must be defined !)
	void connect_tcp() {
		if(!offeredPort) publisher->send(TOSTRING("CONNECT_TCP=" << name));
		else for(SubscriptionImpl* s : subscriptions) s->connect_tcp(offeredPort);
	}

	// Connects all local subscriptions to a local publisher (which must be defined !)
	void connect_shm() {
		if(offeredShmPath.empty()) publisher->send(TOSTRING("CONNECT_SHM" << name));
		else for(SubscriptionImpl* s : subscriptions) s->connect_shm(offeredShmPath);
	}

	// Disconnect all local subscriptions for this Channel */
	void disconnect() {
		for(SubscriptionImpl* s : subscriptions) s->close();
	}


	///////////////////
	// Signaling

	/** Send a TCP transport offer for this Channel to the given Host */
	void offer_tcp(Host* h) {
		// Lazy-create the TCP server
		if(!server) {
			ChannelImpl* c = this;
			server = new TCPServer(10000,20000);
			server->on_open = [&, c, h](TCPSocket* s) { new SubscriptionImpl(c, s, h); };
		}
		h->send(TOSTRING("OFFER_TCP=" << name << "|" << server->port));
	}

	/** Send a SHM transport offer for this Channel to the given Host */
	void offer_shm(Host* h) {
		offeredShmPath = name;
		h->send(TOSTRING("OFFER_SHM" << name << "|" << offeredShmPath << "|" << _size));
	}

	/** Called when receiving a TCP transport offer -> effectively connects to the offered TCPServer */
	void on_offer_tcp(const string& offer) {
		offeredPort = atoi(offer.c_str());
		connect_tcp();
	}

	/** Called when receiving a SHM transport offer -> effectively maps the offered SHM segment */
	void on_offer_shm(const string& offer) {
		offeredShmPath = str_before(offer, "|");
		_size = atoi(str_after(offer, "|").c_str());
		connect_shm();
	}


	//////////////////
	// Utils

	string tostring() {
		return TOSTRING( getType() << name << "|" << (publisher ? publisher->tostring() : ""));
	}
};


////


/** Close all channels published by the given Host */
void close_all_channels(Host* h);


//////////////////////
// CHANNEL REGISTRY //
//////////////////////

/** @return a Channel or creates it if it doesn't exist given its name */
ChannelImpl* get_or_create_channel(const string& name);

/** @return a unique integer identifier for the given channel */
int get_channel_index(ChannelImpl* c);



///////////////
// SIGNALING //
///////////////

/** Called when receiving a "PUBLISH=..." statement */
void apply_publish_statement(Host* h, const string& statement);

/** Called when receiving a "UNPUBLISH=..." statement */
void apply_unpublish_statement(Host* h, const string& statement);

/** Called when receiving a "CONNECT_TCP=" statement */
void make_offer_tcp(Host* h, const string& channel);

/** Called when receiving a "CONNECT_SHM=" statement */
void make_offer_shm(Host* h, const string& channel);

/** Called when receiving a "OFFER_TCP=" statement */
void answer_offer_tcp(Host* h, const string& offer);

/** Called when receiving a "OFFER_SHM=" statement */
void answer_offer_shm(Host* h, const string& offer);

void broadcast_published_channels();




// UTILS
static std::string ENC(const char* str) { return str_replace(str, "/", "@@@"); }


}


#endif /* SRC_CHANNEL_H_ */
