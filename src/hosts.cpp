/*
 * hosts.h
 *
 *  Created on: 18 août 2016
 *      Author: jfellus
 */

#include "hosts.h"
#include "common.h"
#include "channel.h"

using namespace std;

namespace pubsub {


// Channels <-> Hosts mapping

static vector<string> hosts;
extern SignalingServer* signalingServer;
extern vector<EndPoint*> endpoints;

void add_host(const char* url) {
	init();
	if(!has_host(url)) hosts.push_back(url);
	broadcast_published_channels();
}



// Internals

void broadcast_published_channels() {
	for(string h : hosts) {
		signalingServer->broadcast(SSTR( "H" << h ));
	}

	for(EndPoint* ep : endpoints) {
		signalingServer->broadcast(SSTR( "E" << ep->name));
		for(TransportDescription td : ep->offeredTransports) {
			signalingServer->broadcast(SSTR( "T" << ep->name << "=" << td.to_string()));
		}
	}
}

bool has_host(const char* url) {
	for(auto h : hosts) if(h==url) return true;
	return false;
}


void SignalingServer::on_receive(TCPSocket* connection, char* buf, size_t len) {
	printf("[signaling] Recv \"%s\"\n", buf);
	switch(buf[0]) {
		case 'H': // Received a host declaration
			hosts.push_back(&buf[1]);
			broadcast_published_channels();
			break;
		case 'E': // Received an endpoint declaration
			new EndPoint(&buf[1]);
			break;
		case 'T': // Received a transport declaration
		{
			string s = &buf[1];
			string endpoint = str_before(s, "=");
			string transportDescription = str_after(s, "=");

			EndPoint* ep = get_endpoint(endpoint.c_str());
			if(!ep) ep = new EndPoint(endpoint.c_str());

			TransportDescription td(transportDescription);
			td.ip = connection->ip;

			ep->on_remote_offered_transport(td);
			broadcast_published_channels();
			break;
		}
		default:
			fprintf(stderr, "Received bad message : %s\n", buf);
		break;
	}
}



}
