#include <libintl.h>
#include <locale.h>
#include <iostream>

#include "log_i.h"
#include "application.h"

const char *GETTEXT_PACKAGE = "gpx2video";

extern void gpx2video_ui_register_resource(void);


int main(int argc, char *argv[]) {
	std::string locale_path = GPX2VideoApplicationWindow::locale();

	// Intl
	bindtextdomain(GETTEXT_PACKAGE, locale_path.c_str());
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);

	// Trace
	GPX2VideoLog::setup("gpx2video-gtk");
	GPX2VideoLog::quiet(false);
	GPX2VideoLog::set_level(0);

	// gpx2video application
	auto app = GPX2VideoApplication::create();

	// Start the application, showing the initial window,
	// and opening extra views for any files that it is asked to open,
	// for instance as a command-line parameter.
	// run() will return when the last window has been closed.
	return app->run(argc, argv);
}

