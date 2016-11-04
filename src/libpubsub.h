/*
 * libpubsub.h
 *
 *  Created on: 18 août 2016
 *      Author: jfellus
 */

#ifndef SRC_LIBPUBSUB_H_
#define SRC_LIBPUBSUB_H_

#include <stdlib.h>
#include <string.h>
#include <string>
#include <functional>

namespace pubsub {

extern int DBG_LEVEL;
extern int SIGNALING_PORT;

extern void dump_channels();
extern void dump_hosts();

}



class Channel {
public:
	std::function<void(const char* msg, size_t len)> on_message;

	bool write(const char* s, size_t len);
	inline bool write(const std::string& s) { return write(s.c_str()); }
	inline bool write(const char* s) { return write(s, strlen(s)); }
};

class Subscription {
public:
	std::function<void(const char* msg, size_t len)> on_message;

	bool write(const char* s, size_t len);
	inline bool write(const std::string& s) { return write(s.c_str()); }
	inline bool write(const char* s) { return write(s, strlen(s)); }
};


void add_host(const char* url);
void add_host(const std::string& ip, int port);

Channel* publish(const std::string& channel);
Subscription* subscribe(const std::string& subscription);



#endif /* SRC_LIBPUBSUB_H_ */
