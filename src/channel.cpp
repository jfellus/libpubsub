/*
 * channels.cpp
 *
 *  Created on: 18 août 2016
 *      Author: jfellus
 */


#include "hosts.h"
#include "channel.h"
#include "mesh.h"
#include "signaling.h"

namespace pubsub {

static vector<Channel*> channels;


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


Subscription* subscribe_channel(const string& name) {
	DBG_2("Subscribe " << name);
	Channel* c = get_or_create_channel(name);
	Subscription* s = c->subscribe();
//	dump_channels();
	return s;
}

Channel* publish_channel(const string& name) {
	DBG_2("Publish " << name);
	Channel* c = get_or_create_channel(name);
	c->publish();
//	dump_channels();
	return c;
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
	socket->cbRecv = [&](const char* msg, size_t len) { if(channel->on_message) channel->on_message(msg,len); };
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
	socket->write(msg, len);
}



/////////////////////////////








vector<EndPoint*> endpoints; // The global list of all declared endpoints


// Public API

bool has_endpoint(const char* name, bool bCountRequested) {
	for(EndPoint* c : endpoints) if(c->name == name && (bCountRequested || !c->bRequested)) return true;
	return false;
}

EndPoint* get_endpoint(const char* name) {
	for(EndPoint* c : endpoints) if(c->name == name) return c;
	return NULL;
}

EndPoint* request_endpoint(const char* name) {
	EndPoint* ep = get_endpoint(name);
	if(ep) return ep;
	ep = new EndPoint(name, BOTH, 0, true);
	return ep;
}

bool is_transport_offered(const char* channel, const char* transportDescription) {
	EndPoint* ep = get_endpoint(channel);
	if(!ep) return false;
	return ep->is_transport_offered(transportDescription);
}

TransportDescription find_matching_transport(const char* channel, const char* transportDescription) {
	EndPoint* ep = get_endpoint(channel);
	if(!ep) return INVALID_TRANSPORT_DESCRIPTION("?");
	return ep->find_matching_transport(transportDescription);
}



// EndPoint

EndPoint::EndPoint(const char* name, EndPointType type, DataCallback cb, bool bRequested) : bRequested(bRequested) {
	init();
	if(has_endpoint(name)) fprintf(stderr, "[WARNING] Channel already published : %s\n", name);
	this->name = name;
	this->fd = endpoints.size();
	this->cb = cb;
	this->type = type;

	printf("++ Endpoint : %s\n", name);

	endpoints.push_back(this);
	broadcast_published_channels();

	// dump_endpoints();
}

EndPoint::~EndPoint() {
	for(Client* c : clients) c->close();
	for(Server* s : servers) s->close();
}

void EndPoint::realize() {
	if(!bRequested) return;
	bRequested = false;

	for(SubscriptionRequest r : requested_subscriptions) {
		subscribe(r.first.c_str(), r.second);
	}
}


void EndPoint::offer_transport(const char* transportDescription) {
	TransportDescription td(name, transportDescription);
	if(!td) throw "Can't parse transport description";
	td.type = type;

	Server* s = td.create_server();
	s->cb = cb;
	servers.push_back(s);

	offeredTransports.push_back(td);
	DBG("[pubsub] Offer transport " << td << " for channel " << name);

	broadcast_published_channels();

//	dump_endpoints();
}

void EndPoint::subscribe(const char* transportDescription, DataCallback cb) {
	if(bRequested) {
		requested_subscriptions.push_back(SubscriptionRequest(transportDescription, cb));
		return;
	}


	TransportDescription td = find_matching_transport(transportDescription);
	if(!td) throw "Transport not offered";

	Client* c = td.create_client();
	c->cb = cb;
	c->on_close = [&]()->void {
		printf("CLOSED\n");
		bRequested = true;
		vector_remove(clients, c);
	};
	clients.push_back(c);
}

void EndPoint::send(const char* buf, size_t len) {
	for(auto c : servers) c->send(buf, len);
	for(auto c : clients) c->send(buf, len);
}


static bool are_endpoint_type_comptabible(EndPointType t1, EndPointType t2) {
	return t1 == BOTH || (t1 == INPUT && t2 == OUTPUT) || (t1 == OUTPUT && t2 == INPUT);
}

TransportDescription EndPoint::find_matching_transport(const char* transportDescription) {
	TransportDescription td(name, transportDescription);
	for(TransportDescription t : offeredTransports) {
		if(t.protocol == td.protocol && are_endpoint_type_comptabible(t.type, td.type)) return t;
	}
	return INVALID_TRANSPORT_DESCRIPTION(name);
}

bool EndPoint::is_transport_offered(const char* transportDescription) {
	return find_matching_transport(transportDescription).valid;
}

bool EndPoint::on_remote_offered_transport(TransportDescription td) {
	for(TransportDescription t : offeredTransports) if(t == td) return false;
	offeredTransports.push_back(td);
	return true;
}






// Debug

void dump_endpoints() {
	printf("\n\nDump endpoints\n------------------\n");
	for(EndPoint* ep : endpoints) {
		printf(" - %d : %s (%lu transports offered, %lu c, %lu s)\n", ep->fd, ep->name.c_str(), ep->offeredTransports.size(), ep->clients.size(), ep->servers.size());
	}
	printf("----------------\n\n");
}

}
