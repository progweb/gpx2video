#ifndef __GPX2VIDEO__DATETIME_H__
#define __GPX2VIDEO__DATETIME_H__

#include <iostream>
#include <cstdint>
#include <string>


std::string timestamp2iso(uint64_t value);
std::string timestamp2string(uint64_t value, bool utc=false);

#endif

