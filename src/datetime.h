#ifndef __GPX2VIDEO__DATETIME_H__
#define __GPX2VIDEO__DATETIME_H__

#include <iostream>
#include <cstdint>
#include <string>


class Datetime {
public:
	enum Format {
		FormatDate,
		FormatTime,
		FormatDatetime
	};

	static std::string timestamp2iso(uint64_t timestamp);
	static std::string timestamp2string(uint64_t timestamp, Format format=FormatDatetime, bool utc=false);

	static uint64_t string2timestamp(std::string str);
};

#endif

