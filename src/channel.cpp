/*
 * channels.cpp
 *
 *  Created on: 18 août 2016
 *      Author: jfellus
 */


#include "channel.h"
#include "mesh.h"
#include "signaling.h"

namespace pubsub {

static vector<Channel*> channels;

int get_channel_index(Channel* c) {
	for(int i=0; i<channels.size(); i++) if(channels[i]==c) return i;
	return -1;
}


Channel::Channel(const string& name) : name(name) {
	offeredPort = 0;
	server = NULL;
	publisher = NULL;
	channels.push_back(this);
}

Channel::~Channel() {
	vector_remove(channels, this);
}


Channel* get_channel(const string& name) {
	for(Channel* c : channels) if(c->name == name) return c;
	return NULL;
}

bool has_channel(const string& name) {
	return get_channel(name)!=NULL;
}

Channel* get_or_create_channel(const string& name) {
	Channel* c = get_channel(name);
	if(!c) return new Channel(name);
	else return c;
}

void dump_channels() {
	DBG("CHANNELS\n------------");
	for(Channel* c : channels) {
		DBG(TOSTRING("  - " << c->tostring()));
	}
	DBG("------------");
}


void apply_channel_statement(Host* h, const string& statement) {
	DBG("Unimplemented !!!");
}

void apply_publish_statement(Host* h, const string& statement) {
	char type = statement[0];
	string name = str_before(statement.substr(1), "|");
	string opts = str_after(statement, "|");

	Channel* c = get_or_create_channel(name);
	if(type=='P') c->setPublisher(h);
	else if(type=='A' || type=='S') {
		if(c->publisher) {
			if(c->publisher != get_host(str_before(opts, ":"), atoi(str_after(opts, ":").c_str())))
				DBG("Channel " << name << " already has a publisher : " << c->publisher->tostring() << " (so what is " << opts << " ?)");
		}
		else c->setPublisher(get_host(str_before(opts, ":"), atoi(str_after(opts, ":").c_str())));
	}
	else if(type=='R' || type=='N') { }
	else DBG("Wrong channel type...");

//	dump_channels();
}

void apply_unpublish_statement(Host* h, const string& statement) {
	char type = statement[0];
	string name = str_before(statement.substr(1), "|");
	Channel* c = get_channel(name);
	if(c) c->setPublisher(NULL);

//	dump_channels();
}


void close_all_channels(Host* h) {
	for(Channel* c : channels) {
		if(c->publisher == h) c->setPublisher(NULL);
	}
//	dump_channels();
}

void broadcast_published_channels() {
	for(Channel* c : channels) {
		if(c->publisher == Host::me()) Host::broadcast(TOSTRING("PUBLISH=" << c->tostring()));
	}
}


void make_offer(Host* h, const string& channel) {
	Channel* c = get_channel(channel);
	if(c) c->offer(h);
	else DBG("No such channel : " << channel);
}

void answer_offer(Host* h, const string& offer) {
	string channel = str_before(offer, "|");
	string offered = str_after(offer, "|");
	Channel* c = get_channel(channel);
	if(c) c->on_offer(offered);
	else DBG("No such channel : " << channel);
}






Subscription::Subscription(Channel* channel) : channel(channel) {
	channel->subscriptions.push_back(this);
	init();
}

Subscription::Subscription(Channel* channel, TCPSocket* socket) : channel(channel), socket(socket) {
	port = 0;
	channel->subscriptors.push_back(this);
	socket->on_open = [&]() { on_open(); };
	socket->on_close = [&]() { on_close(); };
	socket->cbRecv = [&, channel](const char* msg, size_t len) { if(channel->on_message) channel->on_message(msg,len); };
	bConnected = true;
}

Subscription::~Subscription() {
	vector_remove(channel->subscriptions, this);
	vector_remove(channel->subscriptors, this);
}

void Subscription::init() {
	socket = new TCPSocket();
	socket->on_open = [&]() { on_open(); };
	socket->on_close = [&]() { on_close(); };
	socket->cbRecv = [&](const char* msg, size_t len) { if(on_message) on_message(msg, len); };
	bConnected = false;
}

void Subscription::connect(int port) {
	if(bConnected) return;
	this->port = port;
	bConnected = true;
	socket->connect(channel->publisher->ip.c_str(), port);
}

void Subscription::close() {
	socket->remove_listeners();
	socket->close();
	on_close();
}

void Subscription::on_open() {
	DBG_3("Subscription open to " << channel->name);
}

void Subscription::on_close() {
	bConnected = false;
	socket->remove_listeners();
	DBG_3("Subscription closed to " << channel->name);
	if(!port) delete this;
	else init();
}

void Subscription::write(const char* msg, size_t len) {
	if(!socket->bStop) socket->write(msg, len);
}

}
