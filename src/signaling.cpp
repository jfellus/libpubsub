/*
 * signaling.cpp

 *
 *  Created on: 27 oct. 2016
 *      Author: jfellus
 */

#include "signaling.h"
#include "mesh.h"
#include "utils/utils.h"

namespace pubsub {

void broadcast(const string& s) { Host::broadcast(s); }

void digest_message(Host* h, string& s) {
	// Handle commits
	if(str_starts_with(s, "YOUR_COMMIT=")) {
		int commit_id = atoi(str_after(s, "=").c_str());
		if(Host::me()->commit_id == commit_id) h->bUpToDate = true;
		dump_states();
	} else if(str_starts_with(s, "COMMIT=")) {
		h->commit_id = atoi(str_after(s, "=").c_str());
		h->send(TOSTRING("YOUR_COMMIT=" << h->commit_id));
		dump_states();
	} else if(s == "STATE") {
		DBG("Received state from " << h->ip << ":" << h->port);
	}

	//

}

void commit() {
	for(auto h : hosts) h->bUpToDate = false;
	Host::me()->commit_id++;
	Host::me()->bUpToDate = true;

	DBG("COMMIT " << Host::me()->commit_id);
	dump_states();
	broadcast_state();
	broadcast(TOSTRING("COMMIT=" << Host::me()->commit_id));
}


void broadcast_state() {
	broadcast("STATE");
}


void dump_states() {
	DBG("STATES\n--------------");
	for(auto h : hosts) {
		DBG(h->ip << ":" << h->port << "   [c" << h->commit_id << "] (" << (h->bUpToDate ? "up to date with me" : "outdated with me"));
	}
	DBG("--------------");
}
}
