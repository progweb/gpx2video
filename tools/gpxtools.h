#ifndef __GPX2VIDEO__GPXTOOLS_H__
#define __GPX2VIDEO__GPXTOOLS_H__

#include <iostream>
#include <cstdlib>
#include <string>
#include <list>

#include <unistd.h>

#include "log.h"
#include "application.h"
#include "telemetrysettings.h"


class GPXTools : public GPXApplication {
public:
	class Settings : public GPXApplication::Settings, public TelemetrySettings {
	public:
		Settings(
			std::string gpx_file="", 
			std::string output_file="",
			std::string from="",
			std::string to="",
			int offset=0,
			int max_duration_ms=0,
			TelemetrySettings::Method telemetry_method=TelemetrySettings::MethodNone,
			int telemetry_rate=0)
			: GPXApplication::Settings(
					gpx_file, output_file,
					from, to, 
					offset,
					max_duration_ms)
			, TelemetrySettings(
					telemetry_method, 
					telemetry_rate) {
		}
	};

	GPXTools(struct event_base *evbase);
	~GPXTools();

	Settings& settings(void);
	void setSettings(const Settings &settings);

	int parseCommandLine(int argc, char *argv[]);

private:
	Settings settings_;
};

#endif
