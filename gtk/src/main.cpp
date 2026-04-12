#include <iostream>

#include "log.h"
#include "application.h"

extern void gpx2video_ui_register_resource(void);


int main(int argc, char *argv[]) {
	// Trace
//	g_setenv ("GSETTINGS_SCHEMA_DIR", ".", FALSE);
//::gpx2video_ui_register_resource();
	GPX2VideoLog::setup("gpx2video-gtk");
	GPX2VideoLog::quiet(0);
	GPX2VideoLog::debug_enable(0);

	// gpx2video application
	auto app = GPX2VideoApplication::create();

	// Start the application, showing the initial window,
	// and opening extra views for any files that it is asked to open,
	// for instance as a command-line parameter.
	// run() will return when the last window has been closed.
	return app->run(argc, argv);
}

