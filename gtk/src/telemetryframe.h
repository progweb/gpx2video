#ifndef __GPX2VIDEO__GTK__TELEMETRYFRAME_H__
#define __GPX2VIDEO__GTK__TELEMETRYFRAME_H__

#include <gtkmm/builder.h>
#include <gtkmm/frame.h>

class GPX2VideoTelemetryFrame : public Gtk::Frame {
public:
	GPX2VideoTelemetryFrame();
	GPX2VideoTelemetryFrame(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &ref_builder);
	virtual ~GPX2VideoTelemetryFrame();

protected:
	Glib::RefPtr<Gtk::Builder> ref_builder_;
};

#endif
