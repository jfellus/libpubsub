/*
 * signaling.cpp
 *
 *  Created on: 19 août 2016
 *      Author: jfellus
 */

#include "signaling.h"
#include "utils/utils.h"
#include "hosts.h"
#include "channel.h"

using namespace std;

namespace pubsub {

SignalingServer* signalingServer = NULL;



// SignalingServer

SignalingServer::SignalingServer() : TCPServer(SIGNALING_PORT, SIGNALING_PORT+64) {
	DBG_2("[signaling] Initialize signaling server on port %u\n", port);

	localCommit = 0;
	sem_init(&semStates,0,0);
}

SignalingServer::~SignalingServer() {
	close();
}

void SignalingServer::close() {
	for(SignalingClient* c : signalingClients) c->close();
	signalingClients.clear();
	TCPServer::close();
}

void SignalingServer::connect(const char* url) {
	string ip = url;
	int port = SIGNALING_PORT;
	if(str_has(ip, ":")) {
		port = atoi(str_after(ip, ":").c_str());
		ip = str_before(ip, ":");
	}

	DBG_2("[signaling] Connect to %s:%u\n", ip.c_str(), port);
	SignalingClient* c = new SignalingClient(this, ip.c_str(), port);
	c->on_close = [&]() {
		vector_remove(signalingClients, c);
	};
	states[c] = 0;
	signalingClients.push_back(c);
	c->wait_connected();
}

void SignalingServer::broadcast(const char* msg) {
	for(TCPSocket* connection : connections) connection->write(msg);
	for(TCPSocket* c : signalingClients) c->write(msg);
}

void SignalingServer::on_receive(TCPSocket* connection, char* buf, size_t len) {
	if(!states[connection]) states[connection]=0;

	if(buf[0] == 'W') {
		// Welcome a new host
		on_new_host(connection->ip.c_str());
		broadcast_published_channels();
	}
	else if(buf[0] == 'H') {
		// Received a host declaration
		on_new_host(&buf[1]);
	}
	else if(buf[0] == 'E') {
		// Received an endpoint declaration
		if(!has_endpoint(&buf[1])) {
			new EndPoint(&buf[1]);
			DBG_2("[signaling] Channel \"%s\" offered by %s\n", &buf[1], connection->ip.c_str());
		}
	}
	else if(buf[0] == 'T') {
		// Received a transport declaration
		string s = &buf[1];
		string endpoint = str_before(s, "=");
		string transportDescription = str_after(s, "=");

		EndPoint* ep = get_endpoint(endpoint.c_str());
		if(!ep) ep = new EndPoint(endpoint.c_str());

		TransportDescription td(transportDescription);
		td.ip = connection->ip;

		if(ep->on_remote_offered_transport(td)) {
			DBG_2("[signaling] Transport \"%s\" offered for channel \"%s\"\n", td.to_string().c_str(), ep->name.c_str());
			broadcast_published_channels();
		}

		ep->realize();
	}
	else if(buf[0] == 'C') {
		// Remote host committed a change
		connection->write(SSTR("A" << &buf[1])); // Acknowledge the commit
	}
	else if(buf[0] == 'A') {
		// Remote host acknowledged a commit
		states[connection] = atoi(&buf[1]);
		sem_post(&semStates);
	}
	else fprintf(stderr, "Received bad message : %s\n", buf);
}


void SignalingServer::sync() {
	localCommit++;
	broadcast(SSTR("C" << localCommit));

	for(;;) {
		bool ok = true;
		for(auto p : states) if(p.second != localCommit) ok = false;
		if(ok) break;
		sem_wait(&semStates);
	}
}

void SignalingServer::dump() {
	DBG_3("[signaling] Dump connections [%lu]:\n", connections.size()+signalingClients.size());
	for(TCPSocket* connection : connections) DBG_3(" - %s:%u\n", connection->ip.c_str(), connection->port);
	for(TCPSocket* c : signalingClients) DBG_3(" - %s:%u (cl)\n", c->ip.c_str(), c->port);
}




// Public API

void commit() {
	signalingServer->sync();
}

}
