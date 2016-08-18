#include "libpubsub.h"
#include "channel.h"
#include "hosts.h"
#include "common.h"


namespace pubsub {

extern vector<EndPoint*> endpoints;



int publish_in(const char* channel, DataCallback cb) { return (new EndPoint(channel, cb))->fd; }
int publish_out(const char* channel) { return (new EndPoint(channel))->fd; }

void offer_transport(const char* channel, const char* transportDescription) {
	EndPoint* ep = get_endpoint(channel);
	if(!ep) throw "Channel not found";
	ep->offer_transport(transportDescription);
}

int subscribe_in(const char* channel, const char* transportDescription, DataCallback cb) {
	EndPoint* ep = get_endpoint(channel);
	if(!ep) throw "Channel not found";
	ep->subscribe(transportDescription, cb);
	return ep->fd;
}

int subscribe_out(const char* channel, const char* transportDescription) {
	EndPoint* ep = get_endpoint(channel);
	if(!ep) throw "Channel not found";
	ep->subscribe(transportDescription);
	return ep->fd;
}



void send(int fd, const char* buf, size_t len) {
	if(fd >= endpoints.size()) throw "OutOfBound";
	endpoints[fd]->send(buf, len);
}







}