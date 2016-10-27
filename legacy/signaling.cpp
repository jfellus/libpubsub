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
extern vector<string> hosts;
extern vector<EndPoint*> endpoints;


static pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;


// SignalingServer

SignalingServer::SignalingServer() : TCPServer(SIGNALING_DEFAULT_PORT, SIGNALING_DEFAULT_PORT+64) {
	DBG_2("[signaling] Initialize signaling server on port %u\n", port);

	websocketServer = 0; // TODO
//	websocketServer = new SignalingWebsocketServer();
//	websocketServer->start();

	localCommit = 0;
	sem_init(&semStates,0,0);
}

SignalingServer::~SignalingServer() {
	websocketServer->close();
	close();
}

void SignalingServer::close() {
	for(SignalingClient* c : signalingClients) c->close();
	signalingClients.clear();
	TCPServer::close();
}

void SignalingServer::connect(const char* url) {
	string ip = url;
	int port = SIGNALING_DEFAULT_PORT;
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

	printf("__ %s\n", buf);

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
		if(!has_endpoint(&buf[1], true)) {
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

		TransportDescription td(ep->name, transportDescription);
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
	printf("SYNC!!!\n");
	pthread_mutex_lock(&mut);
	localCommit++;
	broadcast(SSTR("C" << localCommit));

	for(;;) {
		bool ok = true;
		for(auto p : states) printf("%s %u\n", p.first->ip.c_str(), p.second);
		for(auto p : states) if(p.second != localCommit) ok = false;
		if(ok) break;
		sem_wait(&semStates);
	}
	pthread_mutex_unlock(&mut);
	printf("SYNC DONE!!!\n");
}

void SignalingServer::dump() {
	DBG_3("[signaling] Dump connections [%lu]:\n", connections.size()+signalingClients.size());
	for(TCPSocket* connection : connections) DBG_3(" - %s:%u\n", connection->ip.c_str(), connection->port);
	for(TCPSocket* c : signalingClients) DBG_3(" - %s:%u (cl)\n", c->ip.c_str(), c->port);
}



SignalingWebsocketPeer::SignalingWebsocketPeer(struct libwebsocket *ws) : IWebSocketPeer(ws) {
	for(string h : hosts) {
		send(SSTR( "H" << h ));
	}

	for(EndPoint* ep : endpoints) {
		send(SSTR( "E" << ep->name));
		for(TransportDescription td : ep->offeredTransports) {
			send(SSTR( "T" << ep->name << "=" << td.to_string()));
		}
	}

	send(SSTR("C" << signalingServer->localCommit));
}



// Public API

void commit() {
	signalingServer->sync();
}

}
