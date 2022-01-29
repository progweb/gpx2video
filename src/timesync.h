#ifndef __GPX2VIDEO__TIMESYNC_H__
#define __GPX2VIDEO__TIMESYNC_H__

#include <string>
#include <vector>

#include "media.h"
#include "decoder.h"
#include "gpx2video.h"
#include "extractorsettings.h"
#include "extractor.h"


class TimeSync : public Extractor {
public:
	static TimeSync * create(GPX2Video &app);

	virtual ~TimeSync();

	void run(void);

private:
	TimeSync(GPX2Video &app, const ExtractorSettings &settings);

	int offset_;
};

#endif

