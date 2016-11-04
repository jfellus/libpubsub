#include "libpubsub.h"
#include "channel.h"

using namespace pubsub;

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

bool Channel::write(const char* s, size_t len) { ((ChannelImpl*)this)->write(s, len); return true; }
bool Subscription::write(const char* s, size_t len) { ((SubscriptionImpl*)this)->write(s, len); return true; }


