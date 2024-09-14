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

	const Format& telemetryFormat(void) const;

	const Method& telemetryMethod(void) const;

	const int& telemetryRate(void) const;

	static const std::string getFriendlyName(const Method &method);

private:
	enum Format telemetry_format_;

	enum Method telemetry_method_;

	int telemetry_rate_;
};


#endif

