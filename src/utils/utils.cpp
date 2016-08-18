/*
 * utils.cpp
 *
 *  Created on: 18 août 2016
 *      Author: jfellus
 */


#include "utils.h"
#include <algorithm>

std::string str_replace(std::string subject, const std::string& search, const std::string& replace) {
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos) {
         subject.replace(pos, search.length(), replace);
         pos += replace.length();
    }
    return subject;
}

bool str_ends_with(const std::string& s, const std::string& suffix) {
	if(s.length() < suffix.length()) return false;
	return s.compare (s.length() - suffix.length(), suffix.length(), suffix) == 0;
}

bool str_starts_with(const std::string& s, const std::string& prefix) {
	return s.compare(0, prefix.length(), prefix)==0;
}

std::string str_after(const std::string& s, const std::string& prefix) {
	uint i = s.find(prefix);
	if(i==std::string::npos) return s;
	return s.substr(i+prefix.length());
}

std::string str_before(const std::string& s, const std::string& suffix) {
	uint i = s.find(suffix);
	if(i==std::string::npos) return s;
	return s.substr(0,i);
}

std::string str_between(const std::string& s, const std::string& prefix, const std::string& suffix) {
	return str_before(str_after(s,prefix), suffix);
}

bool str_has(const std::string& s, const std::string& needle) {
	return s.find(needle)!=std::string::npos;
}

std::string str_to_lower(const std::string& s) {
	std::string a = s;
	std::transform(a.begin(), a.end(), a.begin(), ::tolower);
	return a;
}
