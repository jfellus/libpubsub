/*
 * mesh.cpp
 *
 *  Created on: 13 oct. 2016
 *      Author: jfellus
 */

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include "mesh.h"
#include "utils/utils.h"
#include "utils/Socket.h"
#include "signaling.h"

namespace pubsub {



vector<Host*> hosts;
int SIGNALING_PORT = SIGNALING_DEFAULT_PORT;
static int ID;

static pthread_mutex_t mut = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
TCPServer* signalingServer;

static void LOCK() {
	pthread_mutex_lock(&mut);
}

static void UNLOCK() {
	pthread_mutex_unlock(&mut);
}


/** Dummy Host for loopback */
Host::Host(int port) {
	commit_id = 0; bUpToDate = true;
	id = 0;
	bConnecting = false;
	bReady = false;
	this->ip = "0.0.0.0";
	this->port = port;
	bServer = true;
	socket = NULL;
	hosts.push_back(this);
}

/** Connect to a remote Host */
Host::Host(const string& ip, int port) {
	commit_id = -1; bUpToDate = false;
	id = 0;
	bConnecting = false;
	bReady = false;
	this->ip = ip;
	this->port = port;
	socket = NULL;
	if(!has_host(ip, port)) hosts.push_back(this);
	bServer = false;

	setSocket(new TCPSocket());
	socket->connect(ip.c_str(), port);
}

/** Add Host from incoming connection */
Host::Host(const string& ip, int port, TCPSocket* socket) {
	commit_id = -1; bUpToDate = false;
	id = 0;
	bReady = false;
	this->ip = ip;
	this->port = port;
	this->socket = NULL;
	if(!has_host(ip, port)) hosts.push_back(this);
	bServer = true;
	setSocket(socket);
}

void Host::setSocket(TCPSocket* s) {
	socket = s;
	socket->on_open = [&]() { welcome(); };
	socket->on_close = [&]() { on_close(); };
	socket->cbRecv = [&](const char* msg, size_t len) { string s(msg,len); on_receive(s); };
	if(bServer) welcome();
}

Host::~Host() {
	vector_remove(hosts, this);
	socket->close();
}

void Host::welcome() {
	bConnecting = true;
	string ip = socket->ip;
	if(ip == "127.0.0.1") ip = "localhost";
	socket->write(TOSTRING("ID=" << ID << "|PORT=" << SIGNALING_PORT));
	socket->write(TOSTRING("YOU=" << ip));
}

void Host::on_open() {
	DBG_2("Connection established to " << ip << ":" << port);
	dump_hosts();
}

void Host::on_close() {
	bReady = false;
	socket->close();
	DBG_2("Connection lost to " << ip << ":" << port);
	dump_hosts();
}

void Host::on_receive(string& s) {
	if(str_starts_with(s, "ID=")) {
		LOCK();
		string sId = str_before(s, "|");
		string sPort = str_after(s, "|");
		id = atoi(str_after(sId, "=").c_str());
		int port = atoi(str_after(sPort, "=").c_str());
		this->port = port;
		UNLOCK();
		send("CONNECTED");
	}
	else if(str_starts_with(s, "YOU=")) {
		me()->ip = str_after(s, "=");
		me()->bReady = true;
	} else if(s=="CONNECTED") {
		bReady = true;
		if(!has_host(ip, port)) hosts.push_back(this);
		on_open();
		broadcast_hosts();
	} else if(!bReady) return;

	else if(str_starts_with(s, "Host=")) {
		string ss = str_after(s, "=");
		add_host(str_before(ss, ":"), atoi(str_after(ss, ":").c_str()));
	} else {
		digest_message(this, s);
	}
}

void Host::send(const string& s) {
	if(socket) socket->write(s);
};


Host* Host::me() { return hosts[0]; }

void Host::broadcast(const string& s) {
	LOCK();
	for(int i=1; i<hosts.size(); i++) {
		if(hosts[i]->bReady) hosts[i]->send(s);
	}
	UNLOCK();
}

void handle_incoming_connection(TCPSocket* socket) {
	socket->cbRecv =  [&, socket](const char* msg, size_t len) {
		string s(msg,len);
		if(str_starts_with(s, "ID=")) {
			LOCK();
			string sPort = str_after(s, "|");
			string sId = str_before(s, "|");
			int id = atoi(str_after(sId, "=").c_str());
			int port = atoi(str_after(sPort, "=").c_str());
			string ip = socket->ip;
			if(ip=="127.0.0.1") ip = "localhost";

			Host* h = get_host(ip, port);
			if(!h) {
				h = new Host(ip, port, socket);
				h->send("CONNECTED");
			}
			else if(id > ID || h->bServer) {
				if(h->socket) { h->socket->remove_listeners(); h->socket->close(true); }
				h->bServer = true;
				h->setSocket(socket);
				if(!h->bReady) h->send("CONNECTED");
			}
			else if(id < ID) socket->close(true);
			else DBG("ERROR : SAME UNIQUE ID FOR " << ip << ":" << port);

			UNLOCK();
		}
	};
}

void add_host(const string& ip, int port) {
	init();
	LOCK();
	if(!has_host(ip, port)) {
		DBG("Connect to " << ip << ":" << port);
		new Host(ip, port);
	}
	UNLOCK();
}

bool has_host(const string& ip, int port) {
	LOCK();
	bool res = false;
	for(auto h : hosts) {
		if(h->ip == ip && h->port == port) { res = true; break; }
	}
	UNLOCK();
	return res;
}

Host* get_host(const string& ip, int port) {
	LOCK();
	Host* res = NULL;
	for(auto h : hosts) {
		if(h->ip == ip && h->port == port) { res = h; break; }
	}
	UNLOCK();
	return res;
}


void broadcast_hosts() {
	LOCK();
	ostringstream s;
	for(auto h : hosts) {
		if(h->bReady) s << "Host=" << h->ip << ":" << h->port << "\n";
	}
	Host::broadcast(s.str());
	UNLOCK();
}

void dump_hosts() {
	LOCK();
	DBG("");
	DBG("Hosts (" << hosts.size() << ")");
	for(auto h : hosts) {
		DBG("- " << h->ip << ":" << h->port << (h->bReady ? "" : " (not ready) ") << (h->bServer ? "(server)" : ""));
	}
	DBG("");
	UNLOCK();
}



//////////
// INIT //
//////////

static void uninit();

static void _sigint_handler(int signo) {
	exit(0);
}

static void _at_exit() {
	uninit();
}


static bool bInited = false;
void init() {
	if(bInited) return;
	LOCK();
	if(!bInited) {
		bInited = true;

		atexit(_at_exit);
		signal(SIGINT, _sigint_handler);

		srand(time(0));
		ID = rand();

		signalingServer = new TCPServer(SIGNALING_DEFAULT_PORT, SIGNALING_DEFAULT_PORT + 64);
		SIGNALING_PORT = signalingServer->port;
		signalingServer->on_open = [&](TCPSocket* s) { handle_incoming_connection(s); };

		DBG_2("Initialized signaling server on port " << SIGNALING_PORT);

		new Host(SIGNALING_PORT); // Create loopback Host

		if(SIGNALING_PORT != SIGNALING_DEFAULT_PORT) add_host("localhost", SIGNALING_DEFAULT_PORT);
	}
	UNLOCK();
}


void uninit() {
	LOCK();
	signalingServer->close();
	// close_all_endpoints();
	UNLOCK();
}






}
