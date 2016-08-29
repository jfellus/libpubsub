/*
 * protocols.cpp
 *
 *  Created on: 18 août 2016
 *      Author: jfellus
 */

#include "protocols.h"
#include "tcp.h"
#include "shm.h"
#include "pipe.h"

namespace pubsub {

/////////////////////////////////////////////////////////////////////////
//                                                                     //
// This file contains the factory for Clients and Server               //
// for each transport protocol (tcp, shm, rtc, ...)                    //
//                                                                     //
/////////////////////////////////////////////////////////////////////////


Client* TransportDescription::create_client() {
	if(protocol == "tcp") return new ClientTCP(ip.c_str(), port);
	else if(protocol == "shm") return new ClientSHM(channel.c_str(), type==OUTPUT);
	else if(protocol == "pipe") return new ClientPipe(channel.c_str(), type==OUTPUT, port);
	return NULL;
}

Server* TransportDescription::create_server() {
	if(protocol == "tcp") return new ServerTCP(port);
	else if(protocol == "shm") return new ServerSHM(channel.c_str(), type==INPUT, port);
	else if(protocol == "pipe") return new ServerPipe(channel.c_str(), type==INPUT, port);
	return NULL;
}


}
