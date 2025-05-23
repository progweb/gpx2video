#ifndef __GPX2VIDEO__TELEMETRYSETTINGS_H__
#define __GPX2VIDEO__TELEMETRYSETTINGS_H__

#include <iostream>
#include <string>

#include "telemetrydata.h"


class TelemetrySettings {
public:
	enum Format {
		FormatNone = 0,

		FormatAuto,
		FormatCSV,
		FormatGPX,
		FormatDump,

		FormatCount,
	};

	enum Filter {
		FilterNone = 0,

		FilterOutlier,

		FilterCount
	};

	enum Method {
		MethodNone = 0,

		MethodSample,
		MethodLinear,
		MethodInterpolate,
		MethodKalman,

		MethodCount
	};

	enum Smooth {
		SmoothNone = 0,

		SmoothWindowedMovingAverage,
		SmoothButterworth,

		SmoothCount
	};

	TelemetrySettings(
			int64_t offset=0,
			bool check=false,
			bool pause_detection=false,
			TelemetrySettings::Method method=TelemetrySettings::MethodNone,
			int rate=1000,
			TelemetrySettings::Format format=TelemetrySettings::FormatAuto
	);
	virtual ~TelemetrySettings();

	void setDataRange(const std::string &begin, const std::string &end);
	void setComputeRange(const std::string &from, const std::string &to);

	void setFilter(enum Filter filter);

	const int64_t& telemetryOffset(void) const;
	void setTelemetryOffset(const int64_t &offset);

	const std::string& telemetryBegin(void) const;
	const std::string& telemetryEnd(void) const;

	const std::string& telemetryFrom(void) const;
	const std::string& telemetryTo(void) const;

	const Format& telemetryFormat(void) const;

	const bool& telemetryCheck(void) const;
	const bool& telemetryPauseDetection(void) const;

	const Filter& telemetryFilter(void) const;

	const Method& telemetryMethod(void) const;
	const int& telemetryRate(void) const;
	void setTelemetryMethod(const Method &method, int rate=1000);

	const TelemetrySettings::Smooth& telemetrySmoothMethod(TelemetryData::Data type) const;
	void setTelemetrySmoothMethod(TelemetryData::Data type, TelemetrySettings::Smooth method);

	const int& telemetrySmoothPoints(TelemetryData::Data type) const;
	void setTelemetrySmoothPoints(TelemetryData::Data type, int number);

	static const std::string getFriendlyFilterName(const Filter &filter);
	static const std::string getFriendlyMethodName(const Method &method);
	static const std::string getFriendlySmoothName(const Smooth &smooth);

	void dump(void) const;

private:
	int64_t telemetry_offset_;

	std::string telemetry_begin_;
	std::string telemetry_end_;

	std::string telemetry_from_;
	std::string telemetry_to_;

	enum Format telemetry_format_;

	bool telemetry_check_;
	bool telemetry_pause_detection_;

	enum Filter telemetry_filter_;
	enum Method telemetry_method_;

	int telemetry_rate_;

	TelemetrySettings::Smooth telemetry_smooth_default_method_;
	TelemetrySettings::Smooth telemetry_smooth_grade_method_;
	TelemetrySettings::Smooth telemetry_smooth_speed_method_;
	TelemetrySettings::Smooth telemetry_smooth_elevation_method_;
	TelemetrySettings::Smooth telemetry_smooth_acceleration_method_;
	TelemetrySettings::Smooth telemetry_smooth_verticalspeed_method_;

	int telemetry_smooth_default_points_;
	int telemetry_smooth_grade_points_;
	int telemetry_smooth_speed_points_;
	int telemetry_smooth_elevation_points_;
	int telemetry_smooth_acceleration_points_;
	int telemetry_smooth_verticalspeed_points_;
};


#endif

