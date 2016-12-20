#include "libpubsub.h"
#include "channel.h"
#include <string.h>

using namespace pubsub;

// Main API

Channel* publish(const string& name) {
	DBG_2("Publish " << name);
	ChannelImpl* c = pubsub::get_or_create_channel(name);
	c->publish();
	return (Channel*)c;
}

Subscription* subscribe(const string& name) {
	DBG_2("Subscribe " << name);
	ChannelImpl* c = pubsub::get_or_create_channel(name);
	pubsub::SubscriptionImpl* s = c->subscribe();
	return (Subscription*)s;
}



// Write

bool Channel::write(const char* s, size_t len) {
	if(s == ptr) ((ChannelImpl*)this)->write();
	else ((ChannelImpl*)this)->write(s, len); return true;
}
bool Channel::write() { ((ChannelImpl*)this)->write(); return true; }

bool Subscription::write(const char* s, size_t len) {
	if(s == ptr) ((SubscriptionImpl*)this)->write();
	else ((SubscriptionImpl*)this)->write(s, len); return true;
}
bool Subscription::write() { ((SubscriptionImpl*)this)->write(); return true; }


// Allocation

void Channel::alloc(size_t size) { ((ChannelImpl*)this)->alloc(size); }
void Subscription::alloc(size_t size) { ((SubscriptionImpl*)this)->alloc(size); }


// Mesh network

void add_host(const char* url) {
	pubsub::add_host(url);
}

void add_host(const std::string& ip, int port) {
	pubsub::add_host(ip, port);
}


namespace pubsub {

/** Parse standard commandline arguments for pubsub-specific options (e.g., -h) */
void parse_args(int argc, char** argv) {

	// Look for << -h host1,host2,... >> for adding hosts to the mesh network
	if(argc >= 2 && !strcmp(argv[1], "-h")) {
		char* hosts = argv[2];
		for(char* h = strtok(hosts, ","); h; h = strtok(NULL, ",")) {
			add_host(h);
		}
	}

}
}

