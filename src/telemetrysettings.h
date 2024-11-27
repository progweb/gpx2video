#ifndef __GPX2VIDEO__TELEMETRYSETTINGS_H__
#define __GPX2VIDEO__TELEMETRYSETTINGS_H__

#include <iostream>
#include <string>


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

	TelemetrySettings(
			int64_t offset=0,
			bool check=false,
			TelemetrySettings::Method method=TelemetrySettings::MethodNone,
			int rate=0,
			int smooth_points=0,
			TelemetrySettings::Format format=TelemetrySettings::FormatAuto
	);
	virtual ~TelemetrySettings();

	void setDataRange(const std::string &begin, const std::string &end);
	void setComputeRange(const std::string &from, const std::string &to);

	void setFilter(enum Filter filter);

	const int64_t& telemetryOffset(void) const;

	const std::string& telemetryBegin(void) const;
	const std::string& telemetryEnd(void) const;

	const std::string& telemetryFrom(void) const;
	const std::string& telemetryTo(void) const;

	const Format& telemetryFormat(void) const;

	const bool& telemetryCheck(void) const;
	const Filter& telemetryFilter(void) const;
	const Method& telemetryMethod(void) const;

	const int& telemetryRate(void) const;
	const int& telemetrySmoothPoints(void) const;

	static const std::string getFriendlyFilterName(const Filter &filter);
	static const std::string getFriendlyMethodName(const Method &method);

	void dump(void) const;

private:
	int64_t telemetry_offset_;

	std::string telemetry_begin_;
	std::string telemetry_end_;

	std::string telemetry_from_;
	std::string telemetry_to_;

	enum Format telemetry_format_;

	bool telemetry_check_;
	enum Filter telemetry_filter_;
	enum Method telemetry_method_;

	int telemetry_rate_;
	int telemetry_smooth_points_;
};


#endif

