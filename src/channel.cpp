/*
 * channels.cpp
 *
 *  Created on: 18 août 2016
 *      Author: jfellus
 */


#include "hosts.h"
#include "channel.h"
#include "common.h"


namespace pubsub {

vector<EndPoint*> endpoints; // The global list of all declared endpoints


// Public API

bool has_endpoint(const char* name) {
	for(EndPoint* c : endpoints) if(c->name == name) return true;
	return false;
}

EndPoint* get_endpoint(const char* name) {
	for(EndPoint* c : endpoints) if(c->name == name) return c;
	return NULL;
}

EndPoint* request_endpoint(const char* name) {
	EndPoint* ep = get_endpoint(name);
	if(ep) return ep;
	ep = new EndPoint(name);
	ep->bRequested = true;
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

EndPoint::EndPoint(const char* name, DataCallback cb) : bRequested(false) {
	init();
	if(has_endpoint(name)) fprintf(stderr, "[WARNING] Channel already published : %s\n", name);
	this->name = name;
	this->fd = endpoints.size();
	this->cb = cb;

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

	Server* s = td.create_server();
	s->cb = cb;
	servers.push_back(s);

	offeredTransports.push_back(td);
	DBG("[pubsub] Offer transport %s for channel %s\n", td.to_string().c_str(), name.c_str());

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
		vector_remove(clients, c);
		// delete c;
	};
	clients.push_back(c);
}

void EndPoint::send(const char* buf, size_t len) {
	for(auto c : servers) c->send(buf, len);
	for(auto c : clients) c->send(buf, len);
}


TransportDescription EndPoint::find_matching_transport(const char* transportDescription) {
	TransportDescription td(name, transportDescription);
	for(TransportDescription t : offeredTransports) {
		if(t.protocol == td.protocol) return t;
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
