#include <string>
#include <iomanip>

#include "log.h"
#include "utils.h"
#include "datetime.h"
#include "test.h"


// Test API
//----------

Test::Test(GPXApplication &app) 
	: Task(app) 
	, app_(app) {
}


Test::~Test() {
}


Test * Test::create(GPXApplication &app) {
	Test *test = new Test(app);

	test->init();

	return test;
}


void Test::init(void) {
	log_call();
}


bool Test::start(void) {
	bool result = true;

	log_notice("Test initialization");

	return result;
}


void Test::printTimestamp(uint64_t timestamp) {
//	char buf[92];
//	char datetime[128];
//
//	time_t t;
//
//	struct tm tm;

	log_call();

//	// Convert to seconds
//	t = timestamp / 1000;
//
//	// Convert to string in UTC
//	gmtime_r(&t, &tm);
//	strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", &tm);
//	snprintf(datetime, sizeof(datetime), "%s.%03dZ", buf, (int) (timestamp % 1000));

	printf("  Datetime ISO: %s\n", ::timestamp2iso(timestamp).c_str());

	printf("  Datetime UTC: %s\n", ::timestamp2string(timestamp, true).c_str());

	// Convert to string in local
//	localtime_r(&t, &tm);
//	strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S%z", &tm);

	printf("  Datetime local: %s\n", ::timestamp2string(timestamp, false).c_str());
}


void Test::testParseDatetime(const char *string) {
	uint64_t timestamp;

	log_call();

	printf("Parsing... '%s'\n", string);

	if ((timestamp = ::string2timestamp(string)) != 0) {
		printTimestamp(timestamp);
	}
	else
		printf("  Fail to parse %s\n", string);
}


bool Test::run(void) {
	log_call();

	std::cout << "## Time 'YYYY-MM-DD HH:MM:SS TZ' parsing test (without ms)" << std::endl;

	testParseDatetime("2020-12-13 08:55:48");
	testParseDatetime("2020-12-13 08:55:48Z");
	testParseDatetime("2020-12-13 08:55:48 +200");
	testParseDatetime("2020-12-13 08:55:48 +0200");
	testParseDatetime("2020-12-13 08:55:48 +02:00");
	testParseDatetime("2020-07-21 08:55:48");
	testParseDatetime("2020-07-21 08:55:48Z");
	testParseDatetime("2020-07-21 08:55:48 +200");
	testParseDatetime("2020-07-21 08:55:48 +0200");
	testParseDatetime("2020-07-21 08:55:48 +02:00");

	std::cout << std::endl;

	std::cout << "## Time 'YYYY-MM-DD HH:MM:SS.mmm TZ' parsing test (with ms)" << std::endl;

	testParseDatetime("2020-12-13 08:55:48.123");
	testParseDatetime("2020-12-13 08:55:48.123Z");
	testParseDatetime("2020-12-13 08:55:48.123456Z");
	testParseDatetime("2020-12-13 08:55:48.123 +200");
	testParseDatetime("2020-12-13 08:55:48.123 +0200");
	testParseDatetime("2020-12-13 08:55:48.123456 +0200");
	testParseDatetime("2020-12-13 08:55:48.123 +02:00");
	testParseDatetime("2020-07-21 08:55:48.123");
	testParseDatetime("2020-07-21 08:55:48.123Z");
	testParseDatetime("2020-07-21 08:55:48.123456Z");
	testParseDatetime("2020-07-21 08:55:48.123 +200");
	testParseDatetime("2020-07-21 08:55:48.123 +0200");
	testParseDatetime("2020-07-21 08:55:48.123456 +0200");
	testParseDatetime("2020-07-21 08:55:48.123 +02:00");

	std::cout << std::endl;

	std::cout << "## Time 'YYYY:MM:DD HH:MM:SS TZ' parsing test (without ms)" << std::endl;

	testParseDatetime("2020:12:13 08:55:48");
	testParseDatetime("2020:12:13 08:55:48Z");
	testParseDatetime("2020:12:13 08:55:48 +200");
	testParseDatetime("2020:12:13 08:55:48 +0200");
	testParseDatetime("2020:12:13 08:55:48 +02:00");
	testParseDatetime("2020:07:21 08:55:48");
	testParseDatetime("2020:07:21 08:55:48Z");
	testParseDatetime("2020:07:21 08:55:48 +200");
	testParseDatetime("2020:07:21 08:55:48 +0200");
	testParseDatetime("2020:07:21 08:55:48 +02:00");

	std::cout << std::endl;

	std::cout << "## Time 'YYYY:MM:DD HH:MM:SS.mmm TZ' parsing test (with ms)" << std::endl;

	testParseDatetime("2020:12:13 08:55:48.123");
	testParseDatetime("2020:12:13 08:55:48.123Z");
	testParseDatetime("2020:12:13 08:55:48.123456Z");
	testParseDatetime("2020:12:13 08:55:48.123 +200");
	testParseDatetime("2020:12:13 08:55:48.123 +0200");
	testParseDatetime("2020:12:13 08:55:48.123456 +0200");
	testParseDatetime("2020:12:13 08:55:48.123 +02:00");
	testParseDatetime("2020:07:21 08:55:48.123");
	testParseDatetime("2020:07:21 08:55:48.123Z");
	testParseDatetime("2020:07:21 08:55:48.123456Z");
	testParseDatetime("2020:07:21 08:55:48.123 +200");
	testParseDatetime("2020:07:21 08:55:48.123 +0200");
	testParseDatetime("2020:07:21 08:55:48.123456 +0200");
	testParseDatetime("2020:07:21 08:55:48.123 +02:00");

	complete();

	return true;
}


bool Test::stop(void) {
	log_notice("Test completed");

	return true;
}

