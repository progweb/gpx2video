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

		FormatCount,
	};

	enum Filter {
		FilterNone = 0,

		FilterTest,

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
			TelemetrySettings::Method method=TelemetrySettings::MethodNone,
			int rate=0,
			TelemetrySettings::Format format=TelemetrySettings::FormatAuto
	);
	virtual ~TelemetrySettings();

	void setDataRange(const std::string &begin, const std::string &end);

	void setFilter(enum Filter filter);

	const std::string& telemetryBegin(void) const;
	const std::string& telemetryEnd(void) const;

	const Format& telemetryFormat(void) const;

	const Filter& telemetryFilter(void) const;
	const Method& telemetryMethod(void) const;

	const int& telemetryRate(void) const;

	static const std::string getFriendlyName(const Method &method);

	void dump(void) const;

private:
	std::string telemetry_begin_;
	std::string telemetry_end_;

	enum Format telemetry_format_;

	enum Filter telemetry_filter_;
	enum Method telemetry_method_;

	int telemetry_rate_;
};


#endif

