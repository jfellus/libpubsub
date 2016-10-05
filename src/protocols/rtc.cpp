/*
 * rtc.cpp
 *
 *  Created on: 30 août 2016
 *      Author: jfellus
 */

#include "rtc.h"

#ifdef USE_WEBRTC
#include <webrtcpp.h>
#endif

namespace pubsub {


ServerRTC::ServerRTC(const char* channel, bool isInput, int port) {
	// printf("open rtc server for %s on %u\n", channel, port);
	// fd = webrtcpp_create(channel);
	//
	// if(isInput) {
	// 	webrtcpp_add_callback(fd, [&](const char* buf, size_t size){
	// 		on_receive(buf, size);
	// 	});
	// }
}


void ServerRTC::send(const char* buf, size_t len) {
	// webrtcpp_write(fd, (void*)buf, len);
}

ServerRTC::~ServerRTC() {
	// webrtcpp_close(fd);
}



ClientRTC::ClientRTC(const char* channel, bool isInput, int port) {
	throw "Not Implemented";
}

void ClientRTC::send(const char* buf, size_t len) {
	throw "Not Implemented";
}


}
