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
#include "channel.h"

namespace pubsub {

extern int DBG_LEVEL;
extern int SIGNALING_PORT;

extern void dump_channels();
extern void dump_hosts();

}

typedef pubsub::Channel Channel;
typedef pubsub::Subscription Subscription;


void add_host(const char* url);
void add_host(const std::string& ip, int port);

Channel* publish(const std::string& channel);
Subscription* subscribe(const std::string& subscription);



#endif /* SRC_LIBPUBSUB_H_ */
