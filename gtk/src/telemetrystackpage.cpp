#include <glibmm/datetime.h>
#include <glibmm/miscutils.h>

#include <giomm/file.h>
#include <giomm/fileinfo.h>

#include <gtkmm/label.h>

#include "log.h"
#include "compat.h"
#include "telemetrystackpage.h"


GPX2VideoTelemetryStackPage::GPX2VideoTelemetryStackPage(BaseObjectType *cobject,
		const Glib::RefPtr<Gtk::Builder> &ref_builder, std::string resource_file)
	: Glib::ObjectBase("GPX2VideoTelemetryStackPage")
	, Gtk::Box(cobject)
	, ref_builder_(ref_builder) 
	, resource_file_(resource_file) {
}


GPX2VideoTelemetryStackPage * GPX2VideoTelemetryStackPage::create(void) {
	log_call();

	const std::string resource_file = "telemetry_stackpage.ui";

	// Load the Builder file and instantiate its widgets.
	auto ref_builder = Gtk::Builder::create_from_resource("/com/progweb/gpx2video/ui/" + resource_file);

	auto stackpage = Gtk::Builder::get_widget_derived<GPX2VideoTelemetryStackPage>(ref_builder, "telemetry_stackpage", resource_file);

	if (!stackpage)
		throw std::runtime_error("No \"telemetry_stackpage\" object in " + resource_file);

	return stackpage;
}


void GPX2VideoTelemetryStackPage::set_telemetry(TelemetrySource *source) {
	log_call();

	Gtk::Label *label;
	Glib::DateTime datetime;

	TelemetryData data;

	auto info = Gio::File::create_for_path(source->filename())->query_info();

	// Populate filename label
	std::string filename = Glib::path_get_basename(Glib::StdStringView(source->filename()));
	label = ref_builder_->get_widget<Gtk::Label>("filename_label");
	label->set_label(filename);

	// Get first gpx point
	source->retrieveFirst(data);
	datetime = Glib::DateTime::create_now_local(data.timestamp() / 1000);

	// Populate firstpoint label
	label = ref_builder_->get_widget<Gtk::Label>("firstpoint_label");
#if GLIBMM_CHECK_VERSION(2, 80, 0)
	label->set_label(datetime.format("%Ex %EX").c_str());
#else
	label->set_label(datetime.format("%x %X").c_str());
#endif

	// Get last gpx point
	source->retrieveLast(data);
	datetime = Glib::DateTime::create_now_local(data.timestamp() / 1000);

	// Populate lastpoint label
	label = ref_builder_->get_widget<Gtk::Label>("lastpoint_label");
#if GLIBMM_CHECK_VERSION(2, 80, 0)
	label->set_label(datetime.format("%Ex %EX").c_str());
#else
	label->set_label(datetime.format("%x %X").c_str());
#endif

	// Populate number of points
	label = ref_builder_->get_widget<Gtk::Label>("numberofpoint_label");
	label->set_label(std::to_string(source->numberOfPoints()));

	// Populate filesize label
	label = ref_builder_->get_widget<Gtk::Label>("filesize_label");
	label->set_label(Glib::format_size(info->get_size()));
}


