/*
 * protocols.cpp
 *
 *  Created on: 18 août 2016
 *      Author: jfellus
 */

#include "protocols.h"
#include "tcp.h"

namespace pubsub {

/////////////////////////////////////////////////////////////////////////
//                                                                     //
// This file contains the factory for Clients and Server               //
// for each transport protocol (tcp, shm, rtc, ...)                    //
//                                                                     //
/////////////////////////////////////////////////////////////////////////


Client* TransportDescription::create_client() {
	if(protocol == "tcp") return new ClientTCP(ip.c_str(), port);
	return NULL;
}

Server* TransportDescription::create_server() {
	if(protocol == "tcp") return new ServerTCP(port);
	return NULL;
}


}
