#include <iostream>
#include <exception>

//#include <giomm/file.h>
#include <glibmm/fileutils.h>
#include <glibmm/markup.h>
//#include <glibmm/version.h>
#include <gtkmm/application.h>
#include <gtkmm/applicationwindow.h>

#include "log.h"
#include "compat.h"
#include "application.h"
#include "about.h"
#include "prefs.h"
#include "window.h"


GPX2VideoApplication::GPX2VideoApplication() 
	: Gtk::Application("com.progweb.gpx2video",
			Gio::Application::Flags::NON_UNIQUE | Gio::Application::Flags::HANDLES_OPEN) {
	log_call();

	log_info("gpx2video built with gtkmm v%d.%d.%d", 
			GTKMM_MAJOR_VERSION, GTKMM_MINOR_VERSION, GTKMM_MICRO_VERSION);
	log_info("  glibmm v%d.%d.%d",
			GLIBMM_MAJOR_VERSION, GLIBMM_MINOR_VERSION, GLIBMM_MICRO_VERSION);

#if GIOMM_CHECK_VERSION(2, 80, 0)
	set_version("v0.0.0");
#endif

	add_main_option_entry(Gio::Application::OptionType::BOOL, "debug", 'v', "Verbose mode", "boolean");
}


Glib::RefPtr<GPX2VideoApplication> GPX2VideoApplication::create(void) {
	log_call();

	return Glib::make_refptr_for_instance<GPX2VideoApplication>(new GPX2VideoApplication());
}


GPX2VideoApplicationWindow * GPX2VideoApplication::create_application_window(void) {
	log_call();

	auto appwindow = GPX2VideoApplicationWindow::create();

	// Make sure that the application runs for as long this window is still open.
	add_window(*appwindow);

	// A window can be added to an application with Gtk::Application::add_window()
	// or Gtk::Window::set_application(). When all added windows have been hidden
	// or removed, the application stops running (Gtk::Application::run() returns()),
	// unless Gio::Application::hold() has been called.

	// Delete the window when it is hidden.
	appwindow->signal_hide().connect([appwindow](){ delete appwindow; });

	return appwindow;
}


int GPX2VideoApplication::on_handle_local_options(const Glib::RefPtr<Glib::VariantDict>& options) {
	log_call();

	bool debug = false;

	if (options->lookup_value("debug", debug))
		gpx2video_log_debug_enable(debug ? 1 : 0);

	return -1;
}


void GPX2VideoApplication::on_startup(void) {
	log_call();

	// Call the base class's implementation.
	Gtk::Application::on_startup();

	// Add actions and keyboard accelerators for the menu.
	add_action("preferences", sigc::mem_fun(*this, &GPX2VideoApplication::on_action_preferences));
	add_action("about", sigc::mem_fun(*this, &GPX2VideoApplication::on_action_about));
	add_action("quit", sigc::mem_fun(*this, &GPX2VideoApplication::on_action_quit));
	set_accel_for_action("app.quit", "<Ctrl>Q");
}


void GPX2VideoApplication::on_activate(void) {
	log_call();

	try {
		// The application has been started, so let's show a window.
		auto appwindow = create_application_window();

		appwindow->present();
	}
	// If create_application_window() throws an exception (perhaps from Gtk::Builder),
	// no window has been created, no window has been added to the application,
	// and therefore the application will stop running.
	catch(const Glib::FileError& ex) {
		std::cerr << "FileError: " << ex.what() << std::endl;
	}
	catch(const Glib::MarkupError& ex) {
		std::cerr << "MarkupError: " << ex.what() << std::endl;
	}
	catch(const Gtk::BuilderError& ex) {
		std::cerr << "BuilderError: " << ex.what() << std::endl;
	}
	catch (const Glib::Error& ex) {
		std::cerr << "GPX2VideoApplication::on_activate(): " << ex.what() << std::endl;
	}
	catch (const std::exception& ex) {
		std::cerr << "GPX2VideoApplication::on_activate(): " << ex.what() << std::endl;
	}
}


void GPX2VideoApplication::on_open(const Gio::Application::type_vec_files& files,
		const Glib::ustring &hint)
{
	log_call();

	(void) hint;

	// The application has been asked to open some files,
	// so let's open a new view for each one.
	GPX2VideoApplicationWindow *appwindow = nullptr;

	auto windows = get_windows();

	if (windows.size() > 0)
		appwindow = dynamic_cast<GPX2VideoApplicationWindow*>(windows[0]);

	try {
		if (!appwindow)
			appwindow = create_application_window();

		for (const auto& file : files) {
			appwindow->open_media_file(file);
			break;
		}

		appwindow->present();
	}
	catch (const Glib::Error& ex) {
		std::cerr << "GPX2VideoApplication::on_open(): " << ex.what() << std::endl;
	}
	catch (const std::exception& ex) {
		std::cerr << "GPX2VideoApplication::on_open(): " << ex.what() << std::endl;
	}
}


void GPX2VideoApplication::on_action_about(void) {
    log_call();

	try {
		auto about_dialog = GPX2VideoAbout::create(*get_active_window());

		about_dialog->present();

		// Delete the dialog when it is hidden.
		about_dialog->signal_hide().connect([about_dialog]() { 
			delete about_dialog; 
		});
	}
	catch (const Glib::Error& ex) {
		std::cerr << "GPX2VideoApplication::on_action_about(): " << ex.what() << std::endl;
	}
	catch (const std::exception& ex) {
		std::cerr << "GPX2VideoApplication::on_action_about(): " << ex.what() << std::endl;
	}
}


void GPX2VideoApplication::on_action_preferences(void) {
    log_call();

	try {
		auto prefs_dialog = GPX2VideoPrefs::create(*get_active_window());

		prefs_dialog->present();

		// Delete the dialog when it is hidden.
		prefs_dialog->signal_hide().connect([prefs_dialog]() { 
			delete prefs_dialog; 
		});
	}
	catch (const Glib::Error& ex) {
		std::cerr << "GPX2VideoApplication::on_action_preferences(): " << ex.what() << std::endl;
	}
	catch (const std::exception& ex) {
		std::cerr << "GPX2VideoApplication::on_action_preferences(): " << ex.what() << std::endl;
	}
}


void GPX2VideoApplication::on_action_quit(void) {
	log_call();

	// Gio::Application::quit() will make Gio::Application::run() return,
	// but it's a crude way of ending the program. The window is not removed
	// from the application. Neither the window's nor the application's
	// destructors will be called, because there will be remaining reference
	// counts in both of them. If we want the destructors to be called, we
	// must remove the window from the application. One way of doing this
	// is to hide the window. See comment in create_appwindow().
	auto windows = get_windows();

	for (auto window : windows)
		window->set_visible(false);

	// Not really necessary, when Gtk::Widget::set_visible(false) is called,
	// unless Gio::Application::hold() has been called without a corresponding
	// call to Gio::Application::release().
	quit();
}

