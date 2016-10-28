#include "libpubsub.h"
#include "channel.h"

using namespace pubsub;

Channel* publish(const string& name) {
	DBG_2("Publish " << name);
	Channel* c = get_or_create_channel(name);
	c->publish();
	return c;
}

Subscription* subscribe(const string& name) {
	DBG_2("Subscribe " << name);
	Channel* c = get_or_create_channel(name);
	Subscription* s = c->subscribe();
	return s;
}




