#include "libpubsub.h"
#include "channel.h"
#include "hosts.h"
#include "common.h"
#include "signaling.h"

namespace pubsub {

extern vector<EndPoint*> endpoints;

static std::string ENC(const char* str) {
	return str_replace(str, "/", "@@@");
}


int publish_in(const char* channel, DataCallback cb) {
	DBG("[pubsub] Publish input : %s\n", channel);
	EndPoint* ep = new EndPoint(ENC(channel).c_str(), INPUT, cb);
	commit();
	return ep->fd;
}

int publish_out(const char* channel) {
	DBG("[pubsub] Publish output : %s\n", channel);
	EndPoint* ep = new EndPoint(ENC(channel).c_str(), OUTPUT);
	commit();
	return ep->fd;
}

void offer_transport(const char* channel, const char* transportDescription) {
	EndPoint* ep = get_endpoint(ENC(channel).c_str());
	if(!ep) throw "Channel not found";
	ep->offer_transport(transportDescription);
	commit();
}

int subscribe_in(const char* channel, const char* transportDescription, DataCallback cb) {
	EndPoint* ep = request_endpoint(ENC(channel).c_str());
	if(!ep) throw "Channel not found";
	ep->subscribe(SSTR(transportDescription << "/input"), cb);
	return ep->fd;
}

int subscribe_out(const char* channel, const char* transportDescription) {
	EndPoint* ep = request_endpoint(ENC(channel).c_str());
	if(!ep) throw "Channel not found";
	ep->subscribe(SSTR(transportDescription << "/output"));
	return ep->fd;
}



void send(int fd, const char* buf, size_t len) {
	if(fd >= endpoints.size()) throw "OutOfBound";
	endpoints[fd]->send(buf, len);
}







}
