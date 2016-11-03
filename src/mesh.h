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
	int known_commit_id;
	bool bUpToDate;
	bool bFastforwarding;

public:
	Host(int port);
	Host(const string& ip, int port);
	Host(const string& ip, int port, TCPSocket* socket);
	virtual ~Host();

	bool is_local();

	void on_open();
	void on_close();
	void on_receive(string& s);

	void send(const string& s);

	static Host* me();
	static void broadcast(const string& s);

	void setSocket(TCPSocket* socket);

	string tostring();

protected:
	void welcome();
};


bool has_host(const string& ip, int port);
Host* get_host(const string& ip, int port);

void dump_hosts();
void broadcast_hosts();

extern void add_host(const char* url);
extern void add_host(const string& ip, int port);



extern vector<Host*> hosts;


}

#endif /* SRC_MESH_H_ */
