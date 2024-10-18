#ifndef __GPX2VIDEO__TEST_H__
#define __GPX2VIDEO__TEST_H__

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "application.h"


class Test : public GPXApplication::Task {
public:
	virtual ~Test();

	static Test * create(GPXApplication &app);

	bool start(void);
	bool run(void);
	bool stop(void);

protected:
	GPXApplication &app_;

	Test(GPXApplication &app);

	void init(void);

private:
	void printTimestamp(uint64_t timestamp);
	bool testParseDatetime(const char *string);
};

#endif

