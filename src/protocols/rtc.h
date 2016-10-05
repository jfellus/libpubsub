/*
 * rtc.h
 *
 *  Created on: 30 août 2016
 *      Author: jfellus
 */

#ifndef SRC_PROTOCOLS_RTC_H_
#define SRC_PROTOCOLS_RTC_H_

#include "protocols.h"

namespace pubsub {

class ServerRTC : public Server {
public:
	int fd;
public:
	ServerRTC(const char* channel, bool isInput, int port);
	virtual ~ServerRTC();

	void on_receive(const char* buf, size_t len) { if(cb) cb(buf, len); }
	virtual void send(const char* buf, size_t len);
};

class ClientRTC: public Client {
public:
	ClientRTC(const char* channel, bool isInput, int port);

	void on_receive(const char* buf, size_t len) { if(cb) cb(buf, len); }
	virtual void send(const char* buf, size_t len);
};

}

#endif /* SRC_PROTOCOLS_RTC_H_ */
