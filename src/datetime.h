#ifndef __GPX2VIDEO__DATETIME_H__
#define __GPX2VIDEO__DATETIME_H__

#include <iostream>
#include <cstdint>
#include <string>


std::string timestamp2iso(uint64_t timestamp);
std::string timestamp2string(uint64_t timestamp, bool withdate=true, bool utc=false);

uint64_t string2timestamp(std::string str);

#endif

