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

	void copy(const TelemetrySettings &settings);

	bool setDataRange(const uint64_t &start, const uint64_t &stop);
	bool setDataRange(const std::string &start, const std::string &stop);

	bool setComputeRange(const uint64_t &start, const uint64_t &stop);
	bool setComputeRange(const std::string &start, const std::string &stop);

	bool setViewRange(const uint64_t &start, const uint64_t &stop);
	bool setViewRange(const std::string &start, const std::string &stop);

	const int64_t& telemetryOffset(void) const;
	void setTelemetryOffset(const int64_t &offset);

	const uint64_t& telemetryBegin(void) const;
	const uint64_t& telemetryEnd(void) const;

	const uint64_t& telemetryComputeFrom(void) const;
	const uint64_t& telemetryComputeTo(void) const;

	const uint64_t& telemetryViewFrom(void) const;
	const uint64_t& telemetryViewTo(void) const;

	const Format& telemetryFormat(void) const;

	const bool& telemetryCheck(void) const;
	void setTelemetryCheck(bool check);

	const bool& telemetryPauseDetection(void) const;
	void setTelemetryPauseDetection(bool pause_detection);

	const Filter& telemetryFilter(void) const;
	void setTelemetryFilter(enum Filter filter);

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

	// First & last data from gpx, csv...
	uint64_t telemetry_begin_;
	uint64_t telemetry_end_;

	// Compute telemetry data range [from:to] timestamp
	uint64_t telemetry_compute_start_;
	uint64_t telemetry_compute_stop_;

	// View telemetry data range [start:stop] timestamp
	uint64_t telemetry_view_start_;
	uint64_t telemetry_view_stop_;

	enum Format telemetry_format_;

	bool telemetry_check_;
	bool telemetry_pause_detection_;

	enum Filter telemetry_filter_;
	enum Method telemetry_method_;

	int telemetry_rate_;

	TelemetrySettings::Smooth telemetry_smooth_default_method_;
	TelemetrySettings::Smooth telemetry_smooth_position_method_;
	TelemetrySettings::Smooth telemetry_smooth_grade_method_;
	TelemetrySettings::Smooth telemetry_smooth_speed_method_;
	TelemetrySettings::Smooth telemetry_smooth_course_method_;
	TelemetrySettings::Smooth telemetry_smooth_heading_method_;
	TelemetrySettings::Smooth telemetry_smooth_elevation_method_;
	TelemetrySettings::Smooth telemetry_smooth_acceleration_method_;
	TelemetrySettings::Smooth telemetry_smooth_verticalspeed_method_;

	int telemetry_smooth_default_points_;
	int telemetry_smooth_position_points_;
	int telemetry_smooth_grade_points_;
	int telemetry_smooth_speed_points_;
	int telemetry_smooth_course_points_;
	int telemetry_smooth_heading_points_;
	int telemetry_smooth_elevation_points_;
	int telemetry_smooth_acceleration_points_;
	int telemetry_smooth_verticalspeed_points_;
};


#endif

