/*
 * hosts.h
 *
 *  Created on: 18 août 2016
 *      Author: jfellus
 */

#include "hosts.h"
#include "common.h"
#include "channel.h"
#include "signaling.h"
#include <pthread.h>

using namespace std;

namespace pubsub {


// Channels <-> Hosts mapping

vector<string> hosts;
extern SignalingServer* signalingServer;
extern vector<EndPoint*> endpoints;


////////////////
// Public API //
////////////////

void add_host(const char* url) {
	init();
	if(!has_host(url)) {
		hosts.push_back(url);
		signalingServer->connect(url);
		signalingServer->broadcast(SSTR("W"));
	}
	broadcast_published_channels();

	commit();
}


void close_all_endpoints() {
	for(EndPoint* ep : endpoints) delete ep;
	endpoints.clear();
}


// Internals

void on_new_host(const char* ip) {
	if(has_host(ip)) return;
	DBG_2("[signaling] New host joined : %s\n", ip);
	hosts.push_back(ip);
	broadcast_published_channels();
}

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

	signalingServer->broadcast(SSTR("C" << signalingServer->localCommit));
}

bool has_host(const char* url) {
	for(auto h : hosts) if(h==url) return true;
	return false;
}


}
