/*
 * hosts.h
 *
 *  Created on: 18 août 2016
 *      Author: jfellus
 */

#ifndef SRC_HOSTS_H_
#define SRC_HOSTS_H_

namespace pubsub {

void dump_published_channels() {}
static void register_host(const char* url) {}

static void on_new_host(const char* ip) {}

}

#endif /* SRC_HOSTS_H_ */
