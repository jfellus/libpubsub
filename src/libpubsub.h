/*
 * libpubsub.h
 *
 *  Created on: 18 août 2016
 *      Author: jfellus
 */

#ifndef SRC_LIBPUBSUB_H_
#define SRC_LIBPUBSUB_H_

#include <stdlib.h>
#include <string.h>
#include <string>
#include <functional>

namespace pubsub {

extern int DBG_LEVEL;
extern int SIGNALING_PORT;

extern void dump_channels();
extern void dump_hosts();

/** Parse standard commandline arguments for pubsub-specific options (e.g., -h) */
extern void parse_args(int argc, char** argv);

}


/** A Channel handles a uniquely named channel available to the local host (Host::me())
 *  A Channel can have multiple
 *  - subscriptions (i.e., the local host subscribe to the channel)
 *  - subscriptors (i.e., a remote host subcribe to the channel)
 *  A Channel encapsulate a memory segment (ptr, size) that carry the channel's OUTGOING data.
 *  The Channel API provide facilities (cast operators and random access operators) to
 *  access the internal memory pointer
 *
 *  NOTE : Please don't manipulate ptr directly, as it can be reallocated (e.g., as a SHM segment)
 *         depending on subscriptors' and allocation requests.
 *         In the future, double/triple buffering will occur and ptr will regularly change its address
 *
 *	A Channel provides an 'on_message' callback that is called when a message is received on the channel
 *
 *	There are two ways of writing to the channel :
 *	- By copy : write(s, len) takes an existing memory segment and send it through the channel
 *	- Direct memory access : simply modify the channel's internal memory via the helper operators and
 *	                         call write to tell that data is ready to be sent. In some cases, DMA is much faster
 *	                         because there is no memory copy at all (e.g. SHM)
 *
 *  When you write to a Channel, data is directly forwarded to all subscriptors (pubsub MIMO model)
 *
 */
class Channel {
protected:
	char* ptr = NULL;
	size_t _size = 0;
public:

	// Callback
	std::function<void(const char* msg, size_t len)> on_message;

	// Allocation
	void alloc(size_t size);


	// DMA
	inline operator char*() {return (char*) ptr; }
	inline operator void*() {return (void*) ptr; }
	inline operator unsigned char*() { return (unsigned char*) ptr; }

	inline operator const char*() const { return (const char*) ptr; }
	inline operator const void*() const { return (const void*) ptr; }
	inline operator const unsigned char*() const { return (const unsigned char*) ptr; }

	inline char& operator[](int i) { return ptr[i]; }

	inline size_t size() { return _size; }


	// Write
	bool write(const char* s, size_t len);
	inline bool write(const std::string& s) { return write(s.c_str()); }
	inline bool write(const char* s) { return write(s, strlen(s)); }

	bool write();
};


/**
 * A Subscription handles one of the actual "connections" attached to a Channel
 * (Channels can have multiple connections, @see{Channel})
 *
 * A Subscription can represent either :
 * - a subscription (the local host has subscribed to some (possibly remote) channel)
 * - a subscriptor (a remote host has subscribed to a locally offered channel)
 *
 * As for Channels, Subscriptions provide facilities to access the channel's internal memory segment
 * They also offer an 'on_message' callback, that is called when data is received from the channel
 * Subscriptions provide the same 2 ways of writing data to the channel (copy and DMA, @see{Channel})
 */
class Subscription {
protected:
	char* ptr = NULL;
	size_t _size = 0;
public:
	// Callback
	std::function<void(const char* msg, size_t len)> on_message;

	// Allocation
	void alloc(size_t size);

	// DMA
	inline operator char*() {return (char*) ptr; }
	inline operator void*() {return (void*) ptr; }
	inline operator unsigned char*() { return (unsigned char*) ptr; }

	inline operator const char*() const { return (const char*) ptr; }
	inline operator const void*() const { return (const void*) ptr; }
	inline operator const unsigned char*() const { return (const unsigned char*) ptr; }

	inline char& operator[](int i) { return ptr[i]; }

	inline size_t size() { return _size; }


	// Write
	bool write(const char* s, size_t len);
	inline bool write(const std::string& s) { return write(s.c_str()); }
	inline bool write(const char* s) { return write(s, strlen(s)); }

	bool write();
};


/** Adds a networking peer to the pusbub mesh network */
void add_host(const char* url);
void add_host(const std::string& ip, int port);



//////////////
// MAIN API //
//////////////

/** Publish a new channel */
Channel* publish(const std::string& channel);

/** Subscribe to an existing channel */
Subscription* subscribe(const std::string& subscription);



#endif /* SRC_LIBPUBSUB_H_ */
