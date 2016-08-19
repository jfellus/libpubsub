/*
 * utils.h
 *
 *  Created on: 18 août 2016
 *      Author: jfellus
 */

#ifndef SRC_UTILS_UTILS_H_
#define SRC_UTILS_UTILS_H_

#include <iostream>
#include <sstream>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <vector>
#include <algorithm>

#define TOSTRING(x) ((std::ostringstream&)(std::ostringstream().flush() << x)).str()
#define SSTR(x) TOSTRING(x).c_str()


bool str_ends_with(const std::string& s, const std::string& suffix);
bool str_starts_with(const std::string& s, const std::string& prefix);
bool str_starts_with(const std::string& s, const std::string& prefix);
std::string str_after(const std::string& s, const std::string& prefix);
std::string str_before(const std::string& s, const std::string& suffix);
std::string str_between(const std::string& s, const std::string& prefix, const std::string& suffix);
std::string str_to_lower(const std::string& s);
std::string str_replace(std::string subject, const std::string& search, const std::string& replace);
bool str_has(const std::string& s, const std::string& needle);


template <typename T> void vector_remove(std::vector<T>& v, const T& a) {
	v.erase(std::remove(v.begin(), v.end(), a), v.end());
}


#define DBG(x...) do { if(DBG_LEVEL > 0) printf(x); } while(0)
#define DBG_2(x...) do { if(DBG_LEVEL > 1) printf(x); } while(0)
#define DBG_3(x...) do { if(DBG_LEVEL > 2) printf(x); } while(0)
#define DBG_4(x...) do { if(DBG_LEVEL > 3) printf(x); } while(0)

extern int DBG_LEVEL;

#endif /* SRC_UTILS_UTILS_H_ */
