#ifndef __GPX2VIDEO__TELEMETRY_H__
#define __GPX2VIDEO__TELEMETRY_H__

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "gpx.h"
#include "gpx2video.h"
#include "telemetrysettings.h"


class Telemetry : public GPX2Video::Task {
public:
	virtual ~Telemetry();

	static Telemetry * create(GPX2Video &app);

	bool start(void);
	bool run(void);
	bool stop(void);

private:
	GPX *gpx_;

	GPXData data_;

	int timecode_ms_;

	std::ofstream out_;

protected:
	GPX2Video &app_;

	Telemetry(GPX2Video &app);

	void init(void);
};

#endif

