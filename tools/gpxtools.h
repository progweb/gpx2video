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
			int max_duration_ms=0,
			std::string begin="",
			std::string end="",
			std::string from="",
			std::string to="",
			int telemetry_offset=0,
			bool telemetry_check=false,
			TelemetrySettings::Filter telemetry_filter=TelemetrySettings::FilterNone,
			TelemetrySettings::Method telemetry_method=TelemetrySettings::MethodNone,
			int telemetry_rate=0,
			TelemetrySettings::Smooth telemetry_smooth_grade_method=TelemetrySettings::SmoothNone,
			int telemetry_smooth_grade_points=0,
			TelemetrySettings::Smooth telemetry_smooth_speed_method=TelemetrySettings::SmoothNone,
			int telemetry_smooth_speed_points=0,
			TelemetrySettings::Smooth telemetry_smooth_elevation_method=TelemetrySettings::SmoothNone,
			int telemetry_smooth_elevation_points=0,
			TelemetrySettings::Smooth telemetry_smooth_acceleration_method=TelemetrySettings::SmoothNone,
			int telemetry_smooth_acceleration_points=0,
			TelemetrySettings::Format telemetry_format=TelemetrySettings::FormatAuto)
			: GPXApplication::Settings(
					gpx_file, output_file,
					max_duration_ms)
			, TelemetrySettings(
					telemetry_offset,
					telemetry_check,
					telemetry_method, 
					telemetry_rate,
					telemetry_format) {
			TelemetrySettings::setFilter(telemetry_filter);
			TelemetrySettings::setDataRange(begin, end);
			TelemetrySettings::setComputeRange(from, to);

			TelemetrySettings::setTelemetrySmoothMethod(TelemetryData::DataGrade, telemetry_smooth_grade_method);
			TelemetrySettings::setTelemetrySmoothPoints(TelemetryData::DataGrade, telemetry_smooth_grade_points);

			TelemetrySettings::setTelemetrySmoothMethod(TelemetryData::DataSpeed, telemetry_smooth_speed_method);
			TelemetrySettings::setTelemetrySmoothPoints(TelemetryData::DataSpeed, telemetry_smooth_speed_points);

			TelemetrySettings::setTelemetrySmoothMethod(TelemetryData::DataElevation, telemetry_smooth_elevation_method);
			TelemetrySettings::setTelemetrySmoothPoints(TelemetryData::DataElevation, telemetry_smooth_elevation_points);

			TelemetrySettings::setTelemetrySmoothMethod(TelemetryData::DataAcceleration, telemetry_smooth_acceleration_method);
			TelemetrySettings::setTelemetrySmoothPoints(TelemetryData::DataAcceleration, telemetry_smooth_acceleration_points);
		}

		void dump(void) const {
			std::cout << "GPXTools settings: " << std::endl;

			GPXApplication::Settings::dump();
			TelemetrySettings::dump();
		}
	};

	GPXTools(struct event_base *evbase);
	~GPXTools();

	Settings& settings(void);
	void setSettings(const Settings &settings);

	int parseTelemetrySmoothArg(char *arg,
		TelemetryData::Data &type, TelemetrySettings::Smooth &method, int &number);
	int parseCommandLine(int argc, char *argv[]);

private:
	Settings settings_;
};

#endif
