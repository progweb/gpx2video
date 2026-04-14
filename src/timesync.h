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

	virtual bool start(void);
	virtual bool run(void);
	virtual bool stop(void);

protected:
	TimeSync(GPXApplication &app, const ExtractorSettings &settings);

private:
	bool ok_;

	int n_;
	int offset_;

    std::ofstream out_;
};

#endif

