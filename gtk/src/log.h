#ifndef __GPX2VIDEO__GTK__LOG_H__
#define __GPX2VIDEO__GTK__LOG_H__

#include <stdio.h>

#include "macros.h"


#ifdef __cplusplus
extern "C" {
#endif

extern int gpx2video_gtk_log_debug_enabled;


#define GPX2VIDEO_GTK_LOG_LEVEL LOG_DEBUG


#define LOG_PREFIX "[gtk] "

/*
 * Only log to stderr so that all messages are correctly ordered on the same
 * stream.
 */

#define log_debug_raw(format, ...) \
    fprintf(stderr, LOG_PREFIX "DEBUG: %s:%d: " format "\n", \
            __PRETTY_FUNCTION__, __LINE__, ## __VA_ARGS__)

#define log_info(format, ...) \
    fprintf(stderr, LOG_PREFIX format "\n", ## __VA_ARGS__)

#define log_notice(format, ...) \
    fprintf(stderr, LOG_PREFIX format "\n", ## __VA_ARGS__)

#define log_warn(format, ...) \
    fprintf(stderr, LOG_PREFIX "WARNING: " format "\n", ## __VA_ARGS__)

#define log_error(format, ...) \
    fprintf(stderr, LOG_PREFIX "ERROR: " format "\n", ## __VA_ARGS__)


#if (GPX2VIDEO_GTK_LOG_LEVEL >= LOG_DEBUG)
#define log_debug(format, ...)                  \
MACRO_BEGIN                                     \
    if (gpx2video_gtk_log_debug_enabled)            \
        log_debug_raw(format, ## __VA_ARGS__);  \
MACRO_END
#else
#define log_debug(format, ...)    do { } while (0)
#endif

#define log_call() log_debug("called")


/*
 * Initialize the log module.
 */

class GPX2VideoLog {
public:
	static void quiet(int quiet);
	static void setup(const char *ident);
	static void debug_enable(int enabled);
};

#ifdef __cplusplus
}
#endif

#endif 

