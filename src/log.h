#ifndef __GPX2VIDEO__LOG_H__
#define __GPX2VIDEO__LOG_H__

#include "macros.h"


#ifdef __cplusplus
extern "C" {
#endif

extern int gpx2video_log_debug_enabled;


#define LOG_MODE_PRINTF

#define GPX2VIDEO_LOG_LEVEL LOG_DEBUG


#if defined(LOG_MODE_PRINTF)

#include <stdio.h>

#define LOG_PREFIX ""

/*
 * Only log to stderr so that all messages are correctly ordered on the same
 * stream.
 */

#define log_debug_raw(format, ...) \
    fprintf(stderr, LOG_PREFIX "DEBUG: %s:%d: " format "\n", \
            __func__, __LINE__, ## __VA_ARGS__)

#define log_info(format, ...) \
    fprintf(stderr, LOG_PREFIX format "\n", ## __VA_ARGS__)

#define log_notice(format, ...) \
    fprintf(stderr, LOG_PREFIX format "\n", ## __VA_ARGS__)

#define log_warn(format, ...) \
    fprintf(stderr, LOG_PREFIX "WARNING: " format "\n", ## __VA_ARGS__)

#define log_error(format, ...) \
    fprintf(stderr, LOG_PREFIX "ERROR: " format "\n", ## __VA_ARGS__)

#else // LOG_MODE_PRINTF

#include <syslog.h>

#define log_debug_raw(format, ...) \
    syslog(LOG_DEBUG, "DEBUG: %s:%d: " format "\n", \
           __func__, __LINE__, ## __VA_ARGS__)

#if (GPX2VIDEO_LOG_LEVEL >= LOG_INFO)
#define log_info(format, ...) \
    syslog(LOG_INFO, format "\n", ## __VA_ARGS__)
#else
#define log_info(format, ...)    do { } while (0)
#endif

#if (GPX2VIDEO_LOG_LEVEL >= LOG_NOTICE)
#define log_notice(format, ...) \
    syslog(LOG_NOTICE, format "\n", ## __VA_ARGS__)
#else
#define log_notice(format, ...)    do { } while (0)
#endif

#if (GPX2VIDEO_LOG_LEVEL >= LOG_WARNING)
#define log_warn(format, ...) \
    syslog(LOG_WARNING, "WARNING: " format "\n", ## __VA_ARGS__)
#else
#define log_warn(format, ...)    do { } while (0)
#endif

#define log_error(format, ...) \
    syslog(LOG_ERR, "ERROR: " format "\n", ## __VA_ARGS__)

#endif // LOG_MODE_PRINTF


#if (GPX2VIDEO_LOG_LEVEL >= LOG_DEBUG)
#define log_debug(format, ...)                  \
MACRO_BEGIN                                     \
    if (gpx2video_log_debug_enabled)            \
        log_debug_raw(format, ## __VA_ARGS__);  \
MACRO_END
#else
#define log_debug(format, ...)    do { } while (0)
#endif

#define log_call() log_debug("called")


/*
 * Initialize the log module.
 */

void gpx2video_log_quiet(int quiet);
void gpx2video_log_setup(const char *ident);
void gpx2video_log_debug_enable(int enabled);

#ifdef __cplusplus
}
#endif

#endif 

