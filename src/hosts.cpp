/*
 * hosts.h
 *
 *  Created on: 18 août 2016
 *      Author: jfellus
 */

#include "hosts.h"
#include "common.h"
#include "channel.h"
#include <pthread.h>

using namespace std;

namespace pubsub {


// Channels <-> Hosts mapping

static vector<string> hosts;
extern SignalingServer* signalingServer;
extern vector<EndPoint*> endpoints;

/////////////////////////////
// LOCK (disabled for know //
/////////////////////////////

static pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
static pthread_t locker = 0;

static void LOCK() {
	//	if(locker == pthread_self()) return;
	//	pthread_mutex_lock(&mut);
	//	locker = pthread_self();
}

static void UNLOCK() {
	//	pthread_mutex_unlock(&mut);
}



////////////////
// Public API //
////////////////

void add_host(const char* url) {
	LOCK();
	init();
	if(!has_host(url)) {
		hosts.push_back(url);
		signalingServer->connect(url);
		signalingServer->broadcast(SSTR("W"));
	}
	broadcast_published_channels();
	UNLOCK();

	commit();
}

void commit() {
	signalingServer->sync();
}



void close_all_endpoints() {
	LOCK();
	for(EndPoint* ep : endpoints) delete ep;
	endpoints.clear();
	UNLOCK();
}


// Internals

void on_new_host(const char* ip) {
	if(has_host(ip)) return;
	DBG_2("[signaling] New host joined : %s\n", ip);
	hosts.push_back(ip);
	broadcast_published_channels();
}

void broadcast_published_channels() {
	LOCK();

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

	UNLOCK();
}

bool has_host(const char* url) {
	for(auto h : hosts) if(h==url) return true;
	return false;
}



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
	c->on_close = [&]() { vector_remove(signalingClients, c); };
	states[c] = 0;
	signalingClients.push_back(c);
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
		}

		broadcast_published_channels();
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
	LOCK();
	localCommit++;
	broadcast(SSTR("C" << localCommit));
	UNLOCK();

	for(;;) {
		bool ok = true;
		for(auto p : states) if(p.second != localCommit) ok = false;
		if(ok) break;
		sem_wait(&semStates);
	}
}



void SignalingServer::dump() {
	DBG_3("[signaling] Dump connections [%u]:\n", connections.size()+signalingClients.size());
	for(TCPSocket* connection : connections) DBG_3(" - %s:%u\n", connection->ip.c_str(), connection->port);
	for(TCPSocket* c : signalingClients) DBG_3(" - %s:%u (cl)\n", c->ip.c_str(), c->port);
}


}
