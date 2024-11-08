#include <string>
#include <cstring>
#include <cmath>
#include <cctype>

#include "log.h"
#include "datetime.h"


std::string timestamp2iso(uint64_t timestamp) {
	int u;
	time_t t;

	struct tm tm;

	char str[128];
	char time[64];

	t = timestamp / 1000;
	u = timestamp % 1000;
	::gmtime_r(&t, &tm);
	::strftime(time, sizeof(time), "%Y-%m-%dT%H:%M:%S", &tm);

	::snprintf(str, sizeof(str), "%s.%03dZ", time, u);

	return std::string(str);
}


std::string timestamp2string(uint64_t timestamp, bool utc) {
	int u;
	time_t t;

	struct tm tm;

	char str[128];
	char time[64];

	t = timestamp / 1000;
	u = timestamp % 1000;

	// Convert to local or utc
	if (utc)
		::gmtime_r(&t, &tm);
	else
		::localtime_r(&t, &tm);

	::strftime(time, sizeof(time), "%Y-%m-%d %H:%M:%S", &tm);

	::snprintf(str, sizeof(str), "%s.%03d", time, u);

	return std::string(str);
}


uint64_t string2timestamp(std::string str) {
	uint64_t timestamp = 0;

	struct tm time;

	const char *s;

	bool utc = false;

	// reset
	memset(&time, 0, sizeof(time));

	// Try format: "2020:12:13 08:55:48"
	// Try format: "2020:12:13 08:55:48.123"
	// Try format: "2020:12:13 08:55:48.123456"
	// Try format: "2020:12:13 08:55:48.123456+0200"
	if ((s = strptime(str.c_str(), "%Y:%m:%d %H:%M:%S", &time)) != NULL)
		;
	// Try format: "2020-07-28T07:04:43"
	// Try format: "2020-07-28T07:04:43Z"
	// Try format: "2020-07-28T07:04:43.123"
	// Try format: "2020-07-28T07:04:43.123456"
	// Try format: "2020-07-28T07:04:43.123456+0200"
	else if ((s = strptime(str.c_str(), "%Y-%m-%dT%H:%M:%S", &time)) != NULL)
		;
	else
		return 0;

	// What else ?
	// Datetime expected format:
	// "Z"
	// "+0200"
	// "+02:00"
	// ".123Z"
	// ".123456Z"
	// ".123+0200"
	// ".123456+0200"
	// ".123+02:00"
	// ".123456+02:00"

	// Parse precision
	if (*s == '.') {
		char *endptr = NULL;

		long val = ::strtol(++s, &endptr, 10);

		// ms: only 3 digits
		timestamp += (int) (val / pow(10, (endptr-s) - 3));

		// Update str pointer
		s = endptr;
	}

	// Parse time zone
	while (std::isspace(*s))
		++s;

	if ((*s == '+') || (*s == '-')) {
		int n = 0;
		int offset = 0;

		bool neg = *s++ == '-';

		utc = true;

		// Parse 4 digits & ignors ':'
		while ((n < 4) && (((*s >= '0') && (*s <= '9')) || (*s == ':'))) {
			if (*s != ':') {
				offset = offset * 10 + *s - '0';
				n++;
			}

			s++;
		}

		// Only hours ?
		if (n == 2)
			offset *= 100;
		else if (n != 4)
			return 0;

		// Convert the minutes to decimal
		if (offset % 100 >= 60)
			return 0;

		offset = (offset / 100) * 100 + ((offset % 100) * 50) / 30;

		if (offset > 1200)
			return 0;

		// Convert offset to ms
		offset = 1000 * (offset * 3600) / 100;

		if (!neg)
			offset = -offset;

		// Return result
		timestamp += offset;
	}
	else if (*s == 'Z')
		utc = true;

	// Convert time
	if (utc) 
		timestamp += timegm(&time) * 1000;
	else {
		time.tm_isdst = -1;
		timestamp += timelocal(&time) * 1000;
	}

	return timestamp;
}

