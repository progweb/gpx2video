#include <string>

#include "log.h"
#include "datetime.h"


std::string timestamp2iso(uint64_t value) {
	int u;
	time_t t;

	struct tm tm;

	char str[128];
	char time[64];

	t = value / 1000;
	u = value % 1000;
	::gmtime_r(&t, &tm);
	::strftime(time, sizeof(time), "%Y-%m-%dT%H:%M:%S", &tm);

	::snprintf(str, sizeof(str), "%s.%03dZ", time, u);

	return std::string(str);
}


std::string timestamp2string(uint64_t value, bool utc) {
	int u;
	time_t t;

	struct tm tm;

	char str[128];
	char time[64];

	t = value / 1000;
	u = value % 1000;

	// Convert to local or utc
	if (utc)
		::gmtime_r(&t, &tm);
	else
		::localtime_r(&t, &tm);

	::strftime(time, sizeof(time), "%Y-%m-%d %H:%M:%S", &tm);

	::snprintf(str, sizeof(str), "%s.%03d", time, u);

	return std::string(str);
}

