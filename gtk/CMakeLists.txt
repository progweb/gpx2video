cmake_minimum_required(VERSION 3.10)

project(GPX2VIDEO-GTK)

include(GNUInstallDirs)
include(CheckFunctionExists)

#
# DEBUG DEFAULT MODE
#
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "DEBUG")
endif()

#
# CONFIGURATION
#
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -g -ggdb -fPIC -rdynamic")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -O2 -g -ggdb -fPIC -fpermissive -rdynamic")

add_definitions(-Wall -Wextra -D_GNU_SOURCE)

#
# OPTIONS
#
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

#
# LIBS
# 
FIND_PACKAGE(PkgConfig)

PKG_CHECK_MODULES(LIBGLM REQUIRED glm)
include_directories(${LIBGLM_INCLUDE_DIRS})

PKG_CHECK_MODULES(LIBEPOXY REQUIRED epoxy)
include_directories(${LIBEPOXY_INCLUDE_DIRS})

PKG_CHECK_MODULES(LIBGTKMM4 REQUIRED IMPORTED_TARGET gtkmm-4.0 glibmm-2.68)
include_directories(${LIBGTKMM4_INCLUDE_DIRS})

PKG_CHECK_MODULES(LIBAVUTIL REQUIRED libavutil>=3.0.0)
include_directories(${LIBAVUTIL_INCLUDE_DIRS})

PKG_CHECK_MODULES(LIBAVFORMAT REQUIRED libavformat>=3.0.0)
include_directories(${LIBAVFORMAT_INCLUDE_DIRS})

PKG_CHECK_MODULES(LIBAVCODEC REQUIRED libavcodec>=3.0.0)
include_directories(${LIBAVCODEC_INCLUDE_DIRS})

PKG_CHECK_MODULES(LIBAVFILTER REQUIRED libavfilter>=3.0.0)
include_directories(${LIBAVFILTER_INCLUDE_DIRS})

PKG_CHECK_MODULES(LIBSWRESAMPLE REQUIRED libswresample>=2.0.0)
include_directories(${LIBSWRESAMPLE_INCLUDE_DIRS})

PKG_CHECK_MODULES(LIBSWSCALE REQUIRED libswscale>=3.0.0)
include_directories(${LIBSWSCALE_INCLUDE_DIRS})

PKG_CHECK_MODULES(LIBFREETYPE REQUIRED freetype2)
include_directories(${LIBFREETYPE_INCLUDE_DIRS})

PKG_CHECK_MODULES(LIBPULSE REQUIRED libpulse-mainloop-glib)
include_directories(${LIBPULSE_INCLUDE_DIRS})

FIND_PACKAGE(OpenImageIO 2.1.12 REQUIRED)

# 
# GTK resources
#

include(GlibCompileResourcesSupport)

# Set the resources to bundle.
set(RESOURCE_ICONS_LIST
	48x48/apps/com.progweb.gpx2video.png
	scalable/apps/gpx2video.svg
	scalable/apps/com.progweb.gpx2video.svg
)

set(RESOURCE_UI_LIST
    gears_menu.ui
	prefs.ui
	window.ui
	video_frame.ui
	widget_frame.ui
	telemetry_frame.ui
)

set(RESOURCE_GL_LIST
	glarea-gles.fs.glsl
	glarea-gles.vs.glsl
	glarea-gl.fs.glsl
	glarea-gl.vs.glsl
)

# Compile the resources.
compile_gresources(RESOURCE_ICONS_FILE
	XML_ICONS_OUT 
	XML_PREFIX "icons_res"
	TYPE EMBED_C
	TARGET "icon-resources.c"
	PREFIX "/com/progweb/gpx2video/icons"
	SOURCE_DIR icons
	RESOURCES ${RESOURCE_ICONS_LIST}
)

compile_gresources(RESOURCE_UI_FILE
	XML_UI_OUT 
	XML_PREFIX "ui_res"
	TYPE EMBED_C
	TARGET "ui-resources.c"
	PREFIX "/com/progweb/gpx2video/ui"
	SOURCE_DIR src/ui
	RESOURCES ${RESOURCE_UI_LIST}
)

compile_gresources(RESOURCE_GL_FILE
	XML_GL_OUT 
	XML_PREFIX "gl_res"
	TYPE EMBED_C
	TARGET "gl-resources.c"
	PREFIX "/com/progweb/gpx2video/gl"
	SOURCE_DIR src/gl
	RESOURCES ${RESOURCE_GL_LIST}
)

set(GPX2VIDEO_RESOURCES
	${RESOURCE_ICONS_FILE}
	${RESOURCE_UI_FILE}
	${RESOURCE_GL_FILE}
)

#
# INCLUDE
#
include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${OIIO_INCLUDE_DIRS}
)

#
# HEADERS
#
set(GPX2VIDEO_HEADERS
)

#
# SOURCES
#
set(GPX2VIDEO_SOURCES
	src/about.cpp
	src/application.cpp
	src/area.cpp
	src/audiodevice.cpp
	src/log.c
	src/main.cpp
	src/prefs.cpp
	src/renderer.cpp
	src/shader.cpp
	src/stream.cpp
	src/window.cpp
	src/videoframe.cpp
	src/widgetframe.cpp
	src/telemetryframe.cpp
	src/videowidget.cpp
)

#
# SUB DIRECTORIES
#

#
# BINARIES
#
add_executable(gpx2video-gtk ${GPX2VIDEO_SOURCES} ${GPX2VIDEO_RESOURCES})
target_link_libraries(gpx2video-gtk gpxcore gpxlib layoutlib ${LIBPULSE_LIBRARIES} ${LIBGLM_LIBRARIES} ${LIBEPOXY_LIBRARIES} ${LIBGTKMM4_LIBRARIES} ${LIBAVUTIL_LIBRARIES} ${LIBAVFORMAT_LIBRARIES} ${LIBAVCODEC_LIBRARIES} ${LIBAVFILTER_LIBRARIES} ${LIBSWRESAMPLE_LIBRARIES} ${LIBSWSCALE_LIBRARIES} ${OIIO_LIBRARIES} ${LIBFREETYPE_LIBRARIES})

