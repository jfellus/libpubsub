/*
 * signaling.h
 *
 *  Created on: 27 oct. 2016
 *      Author: jfellus
 */

#ifndef SRC_SIGNALING_H_
#define SRC_SIGNALING_H_

#include "mesh.h"

namespace pubsub {


//void commit();

//void dump_states();
//
//
//class Delta {
//public:
//	int commit_id;
//
//	Delta() {commit_id = -1;}
//	virtual ~Delta() {}
//
//	virtual string serialize() = 0;
//};
//
//
///** A basic delta that directly wraps a delta string specification */
//class BasicDelta : public Delta {
//public:
//	string s;
//	BasicDelta(const string& s) : s(s) {}
//	virtual ~BasicDelta() {}
//	virtual string serialize() { return s; }
//};
//
//
//void add_delta(Delta* d);
//void add_delta(const string& delta);
//void apply_delta(Host* h, const string& s);
//void apply_state(Host* h, const string& s);

}

#endif /* SRC_SIGNALING_H_ */
