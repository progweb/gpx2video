#ifndef __GPX2VIDEO__GTK__TELEMETRYSTACKPAGE_H__
#define __GPX2VIDEO__GTK__TELEMETRYSTACKPAGE_H__

#include <gtkmm/builder.h>
#include <gtkmm/box.h>

#include "../../src/telemetrymedia.h"


class GPX2VideoTelemetryStackPage : public Gtk::Box {
public:
	GPX2VideoTelemetryStackPage(BaseObjectType *cobject,
			const Glib::RefPtr<Gtk::Builder> &ref_builder, std::string resource_file);

	virtual ~GPX2VideoTelemetryStackPage() {
	}

	static GPX2VideoTelemetryStackPage * create(void);

	std::string name(void) {
		return "telemetry_page";
	}

	std::string title(void) {
		return "Telemetry";
	}

	std::string icon_name(void) {
		return "network-cellular-signal-excellent-symbolic";
	}

	void set_telemetry(TelemetrySource *source);

protected:
	Glib::RefPtr<Gtk::Builder> ref_builder_;

	const std::string resource_file_;
};

#endif

