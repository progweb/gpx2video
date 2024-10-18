#include <string>
#include <iomanip>

#include "log.h"
#include "utils.h"
#include "telemetry/gpx.h"
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
	char buf[92];
	char datetime[128];

	time_t t;

	struct tm tm;

	log_call();

	// Convert to seconds
	t = timestamp / 1000;

	// Convert to string in UTC
	gmtime_r(&t, &tm);
	strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", &tm);
	snprintf(datetime, sizeof(datetime), "%s.%03dZ", buf, (int) (timestamp % 1000));

	printf("  Datetime UTC: %s\n", datetime);

	// Convert to string in local
	localtime_r(&t, &tm);
	strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S%z", &tm);

	printf("  Datetime local: %s\n", buf);
}


bool Test::testParseDatetime(const char *string) {
	bool ok;

	uint64_t timestamp;

	log_call();

	printf("Parsing... '%s'\n", string);

	if ((ok = GPX::parseDatetime(string, &timestamp)) == true) {
		printTimestamp(timestamp);
	}
	else
		printf("  Fail to parse %s\n", string);

	return ok;
}


bool Test::run(void) {
	log_call();

	testParseDatetime("2020:12:13 08:55:48Z");
	testParseDatetime("2020:12:13 08:55:48 +0200");
	testParseDatetime("2020:07:21 08:55:48Z");
	testParseDatetime("2020:07:21 08:55:48 +0200");

	complete();

	return true;
}


bool Test::stop(void) {
	log_notice("Test completed");

	return true;
}

