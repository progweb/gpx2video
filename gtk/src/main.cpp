#include <iostream>

#include "log.h"
#include "application.h"

extern void gpx2video_ui_register_resource(void);

int main(int argc, char *argv[]) {
//	g_setenv ("GSETTINGS_SCHEMA_DIR", ".", FALSE);
//::gpx2video_ui_register_resource();
	gpx2video_log_setup("gpx2video-gtk");
	gpx2video_log_quiet(0);
	gpx2video_log_debug_enable(0);

	auto app = GPX2VideoApplication::create();

	// Start the application, showing the initial window,
	// and opening extra views for any files that it is asked to open,
	// for instance as a command-line parameter.
	// run() will return when the last window has been closed.
	return app->run(argc, argv);
}

