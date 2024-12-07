#include <iostream>
#include <exception>

#include <gtkmm/window.h>
#include <gtkmm/application.h>
#include <gtkmm/applicationwindow.h>
#include <gtkmm/icontheme.h>

#include "log.h"
#include "window.h"


GPX2VideoApplicationWindow::GPX2VideoApplicationWindow(BaseObjectType *cobject,
	const Glib::RefPtr<Gtk::Builder> &ref_builder) 
	: Gtk::ApplicationWindow(cobject)
	, ref_builder_(ref_builder)
	, media_file_("") {
	log_call();


//    // BEGIN TEST
//	std::cout << "APP WIN" << std::endl;
//	std::cout << get_icon_name() << std::endl;
//	std::cout << get_default_icon_name() << std::endl;

	// Set the window icon.
	Gtk::IconTheme::get_for_display(get_display())->add_resource_path("/com/progweb/gpx2video/icons");
//	set_default_icon_name("com.progweb.gpx2video");
	set_icon_name("com.progweb.gpx2video");
//	set_theme_name("gpx2video");

//	std::cout << 
//	Gtk::IconTheme::get_for_display(get_display())->get_theme_name() << std::endl;
//
//	std::cout << get_icon_name() << std::endl;
//	std::cout << get_default_icon_name() << std::endl;
	// END TEST

	// Get widgets from the Gtk::Builder file
//    stack_ = ref_builder_->get_widget<Gtk::Stack>("stack");
//    if (!stack_)
//        throw std::runtime_error("No \"stack\" object in window.ui");

	gears_ = ref_builder_->get_widget<Gtk::MenuButton>("gears");
	if (!gears_)
		throw std::runtime_error("No \"gears\" object in window.ui");

	video_area_ = Gtk::Builder::get_widget_derived<GPX2VideoArea>(ref_builder_, "video_area");
	if (!video_area_)
		throw std::runtime_error("No \"video_area\" object in window.ui");

	// Connect the menu to the MenuButton gears_.
	// (The connection between action and menu item is specified in gears_menu.ui)
	auto menu_builder = Gtk::Builder::create_from_resource("/com/progweb/gpx2video/ui/gears_menu.ui");
	auto menu = menu_builder->get_object<Gio::MenuModel>("menu");
	if (!menu)
		throw std::runtime_error("No \"menu\" object in gears_menu.ui");

	gears_->set_menu_model(menu);
}


GPX2VideoApplicationWindow::~GPX2VideoApplicationWindow() {
	log_call();
}


GPX2VideoApplicationWindow * GPX2VideoApplicationWindow::create(void) {
	log_call();

	// Create a dummy instance before the call to Gtk::Builder::create_from_resource
	// This creation registers GPX2VideoArea's class in the GType system.
	static_cast<void>(GPX2VideoArea());

	// Load the Builder file and instantiate its widgets.
	auto ref_builder = Gtk::Builder::create_from_resource("/com/progweb/gpx2video/ui/window.ui");

	auto window = Gtk::Builder::get_widget_derived<GPX2VideoApplicationWindow>(ref_builder, "application_window");

	if (!window)
		throw std::runtime_error("No \"application_window\" object in window.ui");

	return window;
}


void GPX2VideoApplicationWindow::open_file_view(const Glib::RefPtr<Gio::File> &file) {
	log_call();

	media_file_ = file->get_parse_name();

	video_area_->open_stream(media_file_);
	video_area_->play();
}

