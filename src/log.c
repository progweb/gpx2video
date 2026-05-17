#include "log.h"
#include <unistd.h>


int gpx2video_log_debug_enabled = 0;
int gpx2video_log_verbose_enabled = 0;


void gpx2video_log_quiet(int quiet) {
	if (quiet)
		fclose(stderr);
}


void gpx2video_log_setup(const char *ident)
{
    (void) ident;
}


void gpx2video_log_set_level(int verbose) {
	gpx2video_log_debug_enabled = (verbose > 1);
	gpx2video_log_verbose_enabled = (verbose > 0);
}

