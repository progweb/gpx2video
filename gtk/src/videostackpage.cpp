#include <glibmm/datetime.h>
#include <glibmm/miscutils.h>

#include <giomm/file.h>
#include <giomm/fileinfo.h>

#include <gtkmm/label.h>

#include "log.h"
#include "compat.h"
#include "videostackpage.h"


GPX2VideoVideoStackPage::GPX2VideoVideoStackPage(BaseObjectType *cobject,
		const Glib::RefPtr<Gtk::Builder> &ref_builder, std::string resource_file)
	: Glib::ObjectBase("GPX2VideoVideoStackPage")
	, Gtk::Box(cobject)
	, ref_builder_(ref_builder) 
	, resource_file_(resource_file) {
}


GPX2VideoVideoStackPage * GPX2VideoVideoStackPage::create(void) {
	log_call();

	const std::string resource_file = "video_stackpage.ui";

	// Load the Builder file and instantiate its widgets.
	auto ref_builder = Gtk::Builder::create_from_resource("/com/progweb/gpx2video/ui/" + resource_file);

	auto stackpage = Gtk::Builder::get_widget_derived<GPX2VideoVideoStackPage>(ref_builder, "video_stackpage", resource_file);

	if (!stackpage)
		throw std::runtime_error("No \"video_stackpage\" object in " + resource_file);

	return stackpage;
}


void GPX2VideoVideoStackPage::set_media(MediaContainer *media) {
	log_call();

	Gtk::Label *label;

	VideoStreamPtr stream;
	Glib::DateTime creationtime;

	auto info = Gio::File::create_for_path(media->filename())->query_info();

	// Video resolution
	stream = media->getVideoStream();
	
	// Video creation time
//#if GLIBMM_CHECK_VERSION(2, 80, 0)
//	creationtime = Glib::DateTime::create_from_utc_usec(media_->creationTime() * 1000);
//#else
//	creationtime = Glib::DateTime::create_from_iso8601(Datetime::timestamp2iso(media_->creationTime()));
//#endif
	creationtime = Glib::DateTime::create_now_local(media->creationTime() / 1000);

	// Populate date label
	label = ref_builder_->get_widget<Gtk::Label>("date_label");
#if GLIBMM_CHECK_VERSION(2, 80, 0)
	label->set_label(media->creationTime() ? creationtime.format("%Ex").c_str() : "-");
#else
	label->set_label(media->creationTime() ? creationtime.format("%x").c_str() : "-");
#endif
	
	// Populate time label
	label = ref_builder_->get_widget<Gtk::Label>("time_label");
#if GLIBMM_CHECK_VERSION(2, 80, 0)
	label->set_label(media->creationTime() ? creationtime.format("%EX").c_str() : "-");
#else
	label->set_label(media->creationTime() ? creationtime.format("%X").c_str() : "-");
#endif
	
	// Populate size label
	label = ref_builder_->get_widget<Gtk::Label>("size_label");
	label->set_label(Glib::ustring::sprintf("%d × %d pixels", stream->width(), stream->height()));
	
	// Populate filesize label
	label = ref_builder_->get_widget<Gtk::Label>("filesize_label");
	label->set_label(Glib::format_size(info->get_size()));
}

