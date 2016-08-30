/*
 * rtc.cpp
 *
 *  Created on: 30 août 2016
 *      Author: jfellus
 */

#include "rtc.h"

namespace pubsub {


ServerRTC::ServerRTC(const char* channel, bool isInput, int port) {
	printf("open rtc server for %s on %u\n", channel, port);
}


void ServerRTC::send(const char* buf, size_t len) {

}



ClientRTC::ClientRTC(const char* channel, bool isInput, int port) {

}

void ClientRTC::send(const char* buf, size_t len) {

}


}
