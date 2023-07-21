#ifndef __GPX2VIDEO__TELEMETRYSETTINGS_H__
#define __GPX2VIDEO__TELEMETRYSETTINGS_H__

#include <iostream>
#include <string>


class TelemetrySettings {
public:
	enum Filter {
		FilterNone = 0,

		FilterSample,
		FilterLinear,
		FilterInterpolate,
		FilterKalman,

		FilterCount
	};

	TelemetrySettings();
	virtual ~TelemetrySettings();

	const Filter& filter(void) const;
	void setFilter(const Filter &filter);

	static const std::string getFriendlyName(const Filter &filter);

private:
	enum Filter filter_;
};


#endif

