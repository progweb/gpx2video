cmake_minimum_required(VERSION 3.5)

project(GPX2VIDEO)

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
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

#
# VERSION
#
set(MAJOR 0)
set(MINOR 0)
set(PATCH 0)
set(VERSION ${MAJOR}.${MINOR}.${PATCH})

FILE(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
configure_file(src/version.h.in ${CMAKE_CURRENT_BINARY_DIR}/version.h)

#
# LIBS
# 
FIND_PACKAGE(PkgConfig)

PKG_CHECK_MODULES(LIBEVENT REQUIRED libevent>=2.0.0)
include_directories(${LIBEVENT_INCLUDE_DIRS})

PKG_CHECK_MODULES(LIBSSL REQUIRED libssl>=1.0.0)
include_directories(${LIBSSL_INCLUDE_DIRS})

PKG_CHECK_MODULES(LIBCURL REQUIRED libcurl>=7.38.0)
include_directories(${LIBCURL_INCLUDE_DIRS})

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

PKG_CHECK_MODULES(LIBGEOGRAPHIC REQUIRED geographiclib)
include_directories(${LIBGEOGRAPHIC_INCLUDE_DIRS})

PKG_CHECK_MODULES(LIBFREETYPE REQUIRED freetype2)
include_directories(${LIBFREETYPE_INCLUDE_DIRS})

PKG_CHECK_MODULES(LIBCAIRO REQUIRED cairo)
include_directories(${LIBCAIRO_INCLUDE_DIRS})

FIND_PACKAGE(OpenImageIO 2.1.12 REQUIRED)

#FIND_PACKAGE(Qt5 COMPONENTS Core Gui Widgets REQUIRED)

#
# INCLUDE
#
include_directories(
	${CMAKE_CURRENT_BINARY_DIR}/src
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/src
	${CMAKE_CURRENT_BINARY_DIR}
	${FFMPEG_INCLUDE_DIRS}
	${OIIO_INCLUDE_DIRS}
	${Qt5Core_INCLUDE_DIR}
	${Qt5Gui_INCLUDE_DIR}
	${Qt5Widgets_INCLUDE_DIR}
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
	src/log.c
	src/evcurl.c
	src/evcurl++.cpp
	src/datetime.cpp
	src/kalman.c
	src/oiio.cpp
	src/oiioutils.cpp
	src/ffmpegutils.cpp
	src/decoder.cpp
	src/encoder.cpp
	src/exportcodec.cpp
	src/frame.cpp
	src/gpmf.cpp
	src/extractor.cpp
	src/telemetry.cpp
	src/telemetrymedia.cpp
	src/application.cpp
	tools/gpx2video.cpp
	src/map.cpp
	src/track.cpp
	src/cache.cpp
	src/media.cpp
	src/stream.cpp
	src/audioparams.cpp
	src/videoparams.cpp
	src/videowidget.cpp
	src/renderer.cpp
	src/imagerenderer.cpp
	src/videorenderer.cpp
	src/timesync.cpp
	src/test.cpp
	src/utils.cpp

 	utmconvert/utmconvert.cpp
)

# 
# QT
#
#SET(QT_LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets)
#SET(CMAKE_AUTOMOC ON)

#
# LINK
#
#add_library(core STATIC ${OVERLAY_CORE_SOURCES})

#
# PROPRERTIES
#

#
# LIBRARIES
#
add_library(gpxcore ${GPX2VIDEO_SOURCES})
target_link_libraries(gpxcore gpxlib layoutlib ${LIBEVENT_LIBRARIES} ${LIBCURL_LIBRARIES} ${LIBAVUTIL_LIBRARIES} ${LIBAVFORMAT_LIBRARIES} ${LIBAVCODEC_LIBRARIES} ${LIBAVFILTER_LIBRARIES} ${LIBSWRESAMPLE_LIBRARIES} ${LIBSWSCALE_LIBRARIES} ${OIIO_LIBRARIES} ${LIBGEOGRAPHIC_LIBRARIES} ${LIBCAIRO_LIBRARIES} ${LIBFREETYPE_LIBRARIES} ssl crypto)

#
# SUB DIRECTORIES
#
add_subdirectory(gpxlib)
add_subdirectory(layoutlib)
add_subdirectory(tools)
#add_subdirectory(tests)

#
# ASSETS TOOL
#
file(COPY assets DESTINATION ${CMAKE_BINARY_DIR}/tools/)

