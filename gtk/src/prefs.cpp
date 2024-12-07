#include <iostream>
#include <exception>

#include <gdkmm/general.h>

#include "log.h"
#include "prefs.h"



GPX2VideoPrefs::GPX2VideoPrefs()
	: Glib::ObjectBase("GPX2VideoPrefs") {
	log_call();
}


GPX2VideoPrefs::GPX2VideoPrefs(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder>& ref_builder) 
	: Glib::ObjectBase("GPX2VideoPrefs")
	, Gtk::Dialog(cobject)
	, ref_builder_(ref_builder) {
	log_call();
}


GPX2VideoPrefs::~GPX2VideoPrefs() {
	log_call();
}


GPX2VideoPrefs * GPX2VideoPrefs::create(Gtk::Window &parent) {
	log_call();

	// Create a dummy instance before the call to Gtk::Builder::create_from_resource
	// This creation registers GPX2VideoPrefs's class in the GType system.
	static_cast<void>(GPX2VideoPrefs());

	// Load the Gtk::Builder file and instantiate its widgets.
	auto ref_builder = Gtk::Builder::create_from_resource("/com/progweb/gpx2video/ui/prefs.ui");

	auto dialog = Gtk::Builder::get_widget_derived<GPX2VideoPrefs>(ref_builder, "prefs_dialog");
	if (!dialog)
		throw std::runtime_error("No \"prefs_dialog\" object in prefs.ui");

	dialog->set_transient_for(parent);

	return dialog;
}

