#ifndef __GPX2VIDEO__GTK__VIDEOFRAME_H__
#define __GPX2VIDEO__GTK__VIDEOFRAME_H__

#include <gtkmm/builder.h>
#include <glibmm/dispatcher.h>
#include <gtkmm/frame.h>

#include "../../src/media.h"
#include "../../src/telemetrymedia.h"


class GPX2VideoVideoFrame : public Gtk::Frame {
public:
	GPX2VideoVideoFrame();
	GPX2VideoVideoFrame(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &ref_builder);
	virtual ~GPX2VideoVideoFrame();

	void refresh(void) {
		update_content();
	}

	void set_visible(bool visible);

	void set_video_selected(MediaContainer *media);
	void set_telemetry(TelemetrySource *source);

	bool set_starttime_from_gpx_data(void);
	bool set_starttime_from_creation_time(void);

	void on_action_use_creation_time(void);
	void on_action_use_gpmf_stream(void);
	void on_action_use_gpx_data(void);

	Glib::Dispatcher& signal_video_changed(void) {
		return video_dispatcher_;
	}

	Glib::Dispatcher& signal_timesync_requested(void) {
		return timesync_dispatcher_;
	}

protected:
	Glib::RefPtr<Gtk::Builder> ref_builder_;

	Glib::Dispatcher video_dispatcher_;
	Glib::Dispatcher timesync_dispatcher_;

	void update_content(void);

	void on_datetimestart_shown(void);
	void on_datetimestart_clicked(void);

private:
	MediaContainer *media_;

	TelemetrySource *source_;
};

#endif

