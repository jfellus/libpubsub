#ifndef SRC_PROTOCOLS_SHM_H_
#define SRC_PROTOCOLS_SHM_H_


#include "protocols.h"
#include "../utils/shm.h"


// Implementation of SHM (shared memory) transport protocol

namespace pubsub {


class ServerSHM: public SHMServer, public Server {
public:
	ServerSHM(const char* name, bool isInput, size_t bufsize) : SHMServer(name, isInput, bufsize) {}
	virtual ~ServerSHM() {}

	virtual void send(const char* buf, size_t len) { write(buf, len); }
	virtual void on_receive(char* buf, size_t len) { if(cb) cb(buf, len); }

};

class ClientSHM : public Client, public SHMClient {
public:
	ClientSHM(const char* name, bool isInput) : SHMClient(name, isInput) {}
	virtual ~ClientSHM() {}

	virtual void send(const char* buf, size_t len) { write(buf, len); }
	virtual void on_receive(char* buf, size_t len) { if(cb) cb(buf, len); }

};


}




#endif /* SRC_PROTOCOLS_TCP_H_ */
