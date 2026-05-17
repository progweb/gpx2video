#include "../../src/log.h"
#include "log_i.h"
#include <unistd.h>


int gpx2video_gtk_log_debug_enabled = 0;
int gpx2video_gtk_log_verbose_enabled = 0;


void GPX2VideoLog::quiet(bool quiet) {
	if (quiet)
		fclose(stderr);
}


void GPX2VideoLog::setup(const char *ident)
{
    (void) ident;
}


void GPX2VideoLog::set_level(int verbose) {
	// 0: gtk & core notice
	// 1: gtk info
	// 2: gtk info / core info
	// 3: gtk info + debug / core info
	// 4: gtk info + debug / core info debug

	gpx2video_gtk_log_debug_enabled = (verbose > 2);
	gpx2video_gtk_log_verbose_enabled = (verbose > 0);

	gpx2video_log_set_level((verbose >> 1));
}

