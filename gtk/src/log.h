#ifndef __GPX2VIDEO__GTK__LOG_H__
#define __GPX2VIDEO__GTK__LOG_H__

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int gpx2video_gtk_log_debug_enabled;
extern int gpx2video_gtk_log_verbose_enabled;


/*
 * Initialize the log module.
 */

class GPX2VideoLog {
public:
	static void quiet(bool quiet);
	static void setup(const char *ident);
	static void set_level(int verbose);
};

#ifdef __cplusplus
}
#endif

#endif 

