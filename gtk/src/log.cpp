#include "log.h"
#ifndef LOG_MODE_PRINTF
#include <syslog.h>
#else
#include <unistd.h>
#endif /* LOG_MODE_PRINTF */


int gpx2video_gtk_log_debug_enabled = 0;


void GPX2VideoLog::quiet(int quiet) {
#if defined(LOG_MODE_PRINTF)
	if (quiet)
		fclose(stderr);
#else
	int mask = LOG_UPTO(quiet ? LOG_WARNING : LOG_DEBUG);
	setlogmask(mask);
#endif
}


void GPX2VideoLog::setup(const char *ident)
{
#if defined(LOG_MODE_PRINTF)
    (void) ident;
#else
    openlog(ident, LOG_PID, LOG_DAEMON);
#endif
}


void GPX2VideoLog::debug_enable(int verbose) {
	gpx2video_gtk_log_debug_enabled = (verbose > 0);
}

