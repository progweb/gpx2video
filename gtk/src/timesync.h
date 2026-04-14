#ifndef __GPX2VIDEO__GTK__TIMESYNC_H__
#define __GPX2VIDEO__GTK__TIMESYNC_H__

#include <glibmm/dispatcher.h>

#include "../../src/timesync.h"


class GPX2VideoTimeSync : public TimeSync {
public:
	virtual ~GPX2VideoTimeSync();

	static GPX2VideoTimeSync * create(GPXApplication &app, MediaContainer *container);

	void exec(void);

	virtual bool start(void);
	virtual bool stop(void);

	Glib::Dispatcher& signal_timesync_changed(void) {
		return dispatcher_;
	}

protected:
	GPX2VideoTimeSync(GPXApplication &app, const ExtractorSettings &settings);

private:
	Glib::Dispatcher dispatcher_;
};

#endif

