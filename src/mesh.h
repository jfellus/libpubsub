/*
 * mesh.h
 *
 *  Created on: 6 oct. 2016
 *      Author: jfellus
 */

#ifndef SRC_MESH_H_
#define SRC_MESH_H_

#include "utils/Socket.h"
#include <string>
#include <vector>

using namespace std;

namespace pubsub {

#define SIGNALING_DEFAULT_PORT 12212

class Host;

extern vector<Host*> hosts;


// Interface
void digest_message(Host* h, string& s);
////////////



class Host {
public:
	string ip;
	int port;
	int id;
	bool bReady;
	bool bConnecting;
	bool bServer;
	TCPSocket* socket;
	thread th;

	int commit_id;
	bool bUpToDate;

public:
	Host(int port);
	Host(const string& ip, int port);
	Host(const string& ip, int port, TCPSocket* socket);
	virtual ~Host();

	void on_open();
	void on_close();
	void on_receive(string& s);


	void send(const string& s);

	static Host* me();
	static void broadcast(const string& s);

	void setSocket(TCPSocket* socket);

protected:
	void welcome();
};


void add_host(const string& ip, int port);
bool has_host(const string& ip, int port);
Host* get_host(const string& ip, int port);

void dump_hosts();
void broadcast_hosts();




void init(); // TODO hide

}

#endif /* SRC_MESH_H_ */
