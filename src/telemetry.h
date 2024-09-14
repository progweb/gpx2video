#ifndef __GPX2VIDEO__TELEMETRY_H__
#define __GPX2VIDEO__TELEMETRY_H__

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "application.h"
#include "telemetrysettings.h"
#include "telemetrymedia.h"


class Telemetry : public GPXApplication::Task {
public:
	virtual ~Telemetry();

	static Telemetry * create(GPXApplication &app, TelemetrySettings &settings);

	const TelemetrySettings& settings(void) {
		return settings_;
	}

	bool start(void);
	bool run(void);
	bool stop(void);

	enum TelemetrySource::Data get(TelemetryData &data, int64_t timecode_ms);

protected:
	GPXApplication &app_;

	TelemetrySettings &settings_;

	TelemetrySettings::Format output_format_;

	TelemetrySource *source_;

	TelemetryData data_;

	int64_t timecode_ms_;

	std::ofstream out_;

	Telemetry(GPXApplication &app, TelemetrySettings &settings);

	void init(void);
};

#endif

