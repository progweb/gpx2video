#ifndef __GPX2VIDEO__LOG_H__
#define __GPX2VIDEO__LOG_H__

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int gpx2video_log_debug_enabled;
extern int gpx2video_log_verbose_enabled;


/*
 * Initialize the log module.
 */

void gpx2video_log_quiet(int quiet);
void gpx2video_log_setup(const char *ident);
void gpx2video_log_set_level(int verbose);

#ifdef __cplusplus
}
#endif

#endif 

