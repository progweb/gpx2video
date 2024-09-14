#ifndef __GPX2VIDEO__TIMESYNC_H__
#define __GPX2VIDEO__TIMESYNC_H__

#include <string>
#include <vector>

#include "media.h"
#include "decoder.h"
#include "application.h"
#include "extractorsettings.h"
#include "extractor.h"


class TimeSync : public Extractor {
public:
	static TimeSync * create(GPXApplication &app, MediaContainer *container);

	virtual ~TimeSync();

	bool start(void);
	bool run(void);
	bool stop(void);

private:
	TimeSync(GPXApplication &app, const ExtractorSettings &settings);

	bool ok_;

	int n_;
	int offset_;

    std::ofstream out_;
};

#endif

