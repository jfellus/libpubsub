/*
 * channel.h
 *
 *  Created on: 18 août 2016
 *      Author: jfellus
 */

#ifndef SRC_CHANNEL_H_
#define SRC_CHANNEL_H_

#include <vector>
#include <string>

#include "protocols/protocols.h"


using namespace std;

namespace pubsub {



class EndPoint;


bool has_endpoint(const char* name);
EndPoint* get_endpoint(const char* name);
bool is_transport_offered(const char* channel, const char* transportDescription);
TransportDescription find_matching_transport(const char* channel, const char* transportDescription);



class EndPoint {
public:
	string name;
	int fd;

	DataCallback cb;

	vector<TransportDescription> offeredTransports;
	vector<Server*> servers;
	vector<Client*> clients;

public:
	EndPoint(const char* name, DataCallback cb = 0);
	virtual ~EndPoint() {}

	bool is_input() { return cb; }
	bool is_output() { return !cb; }

	void offer_transport(const char* transportDescription);
	bool is_transport_offered(const char* transportDescription);
	TransportDescription find_matching_transport(const char* transportDescription);
	void on_remote_offered_transport(TransportDescription td);

	void subscribe(const char* transportDescription, DataCallback cb = 0);

	void send(const char* buf) { send(buf, strlen(buf)); }
	void send(const char* buf, size_t len);
	void on_receive(const char* buf, size_t len) { cb(buf, len); }
};


}


#endif /* SRC_CHANNEL_H_ */
