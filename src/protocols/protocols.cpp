/*
 * protocols.cpp
 *
 *  Created on: 18 août 2016
 *      Author: jfellus
 */

#include "protocols.h"


namespace pubsub {

Client* TransportDescription::create_client() {
	if(protocol == "tcp") return new ClientTCP(ip.c_str(), port);
	return NULL;
}

Server* TransportDescription::create_server() {
	if(protocol == "tcp") return new ServerTCP(port);
	return NULL;
}


}
