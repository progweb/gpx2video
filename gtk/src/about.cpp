#include <iostream>
#include <exception>

#include <gdkmm/general.h>

#include "log.h"
#include "about.h"



GPX2VideoAbout::GPX2VideoAbout()
	: Glib::ObjectBase("GPX2VideoAbout") {
	log_call();
}


GPX2VideoAbout::GPX2VideoAbout(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder>& ref_builder) 
	: Glib::ObjectBase("GPX2VideoAbout")
	, Gtk::AboutDialog(cobject)
	, ref_builder_(ref_builder) {
	log_call();
}


GPX2VideoAbout::~GPX2VideoAbout() {
	log_call();
}


GPX2VideoAbout * GPX2VideoAbout::create(Gtk::Window &parent) {
#define GPX2VIDEO_COPYRIGHT \
	"Copyright \xC2\xA9 2024 - Progweb Team"

	log_call();

//	// Load the Gtk::Builder file and instantiate its widgets.
//	auto ref_builder = Gtk::Builder::create_from_resource("/com/progweb/gpx2video/ui/about.ui");
//
//	auto dialog = Gtk::Builder::get_widget_derived<GPX2VideoAbout>(ref_builder, "about_dialog");
//	if (!dialog)
//		throw std::runtime_error("No \"about_dialog\" object in about.ui");

	const std::vector<Glib::ustring> authors = {
          "The Progweb Team",
	};

	const std::vector<Glib::ustring> documenters = {
          "The Progweb Team",
	};

	auto dialog = new GPX2VideoAbout();

	dialog->set_transient_for(parent);

	dialog->set_program_name("GPX2Video");
	dialog->set_version("v0.0.0");
	dialog->set_copyright(GPX2VIDEO_COPYRIGHT);
	dialog->set_comments("Video Telemetry Overlay Editor");
	dialog->set_license_type(Gtk::License::GPL_3_0);
	dialog->set_website("https://github.com/progweb/gpx2video");
	dialog->set_website_label("Github Project Page");
	dialog->set_authors(authors);
	dialog->set_documenters(documenters);
	dialog->set_logo_icon_name("com.progweb.gpx2video");

	return dialog;
}

