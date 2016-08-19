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

namespace pubsub {

extern int DBG_LEVEL;


typedef void (*DataCallback) (const char* buf, size_t len);


int publish_in(const char* channel, DataCallback cb);
int publish_out(const char* channel);

void offer_transport(const char* channel, const char* transportDescription);

int subscribe_in(const char* channel, const char* transportDescription, DataCallback cb);
int subscribe_out(const char* channel, const char* transportDescription);


void send(int fd, const char* buf, size_t len);
inline void send(int fd, const char* buf) { send(fd, buf, strlen(buf)); }


void add_host(const char* url);


}


#endif /* SRC_LIBPUBSUB_H_ */
