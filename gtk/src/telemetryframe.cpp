#include "log.h"
#include "telemetryframe.h"



GPX2VideoTelemetryFrame::GPX2VideoTelemetryFrame()
	: Glib::ObjectBase("GPX2VideoTelemetryFrame") {
	log_call();
}


GPX2VideoTelemetryFrame::GPX2VideoTelemetryFrame(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &ref_builder) 
	: Glib::ObjectBase("GPX2VideoTelemetryFrame")
	, Gtk::Frame(cobject)
	, ref_builder_(ref_builder) {
	log_call();
}


GPX2VideoTelemetryFrame::~GPX2VideoTelemetryFrame() {
	log_call();
}

