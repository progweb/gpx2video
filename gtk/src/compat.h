#ifndef __GPX2VIDEO__COMPAT_H__
#define __GPX2VIDEO__COMPAT_H__

#include <giommconfig.h>
#include <glibmmconfig.h>
#include <gtkmmconfig.h>
#include <gtkmm/version.h>


#ifndef GLIBMM_CHECK_VERSION
#define GLIBMM_CHECK_VERSION(major, minor, micro) \
  (GLIBMM_MAJOR_VERSION > (major) || \
  (GLIBMM_MAJOR_VERSION == (major) && GLIBMM_MINOR_VERSION > (minor)) || \
  (GLIBMM_MAJOR_VERSION == (major) && GLIBMM_MINOR_VERSION == (minor) && \
   GLIBMM_MICRO_VERSION >= (micro)))
#endif


#ifndef GIOMM_CHECK_VERSION
#define GIOMM_CHECK_VERSION(major, minor, micro) \
  (GIOMM_MAJOR_VERSION > (major) || \
  (GIOMM_MAJOR_VERSION == (major) && GIOMM_MINOR_VERSION > (minor)) || \
  (GIOMM_MAJOR_VERSION == (major) && GIOMM_MINOR_VERSION == (minor) && \
   GIOMM_MICRO_VERSION >= (micro)))
#endif


#ifndef GTKMM_CHECK_VERSION
#define GTKMM_CHECK_VERSION(major, minor, micro) \
  (GTKMM_MAJOR_VERSION > (major) || \
  (GTKMM_MAJOR_VERSION == (major) && GTKMM_MINOR_VERSION > (minor)) || \
  (GTKMM_MAJOR_VERSION == (major) && GTKMM_MINOR_VERSION == (minor) && \
   GTKMM_MICRO_VERSION >= (micro)))
#endif

#endif

