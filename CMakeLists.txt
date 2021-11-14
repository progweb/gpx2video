project(GPX2VIDEO)

cmake_minimum_required(VERSION 2.9)

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
add_definitions(-g -Wall -Wextra -D_GNU_SOURCE -fPIC -fpermissive -rdynamic)
#add_definitions(-g -Wall -Wextra -D_GNU_SOURCE -fPIC -rdynamic)

#
# OPTIONS
#

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

FIND_PACKAGE(Qt5 COMPONENTS Core Gui Widgets REQUIRED)

#
# INCLUDE
#
include_directories(
	${CMAKE_CURRENT_BINARY_DIR}/src
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/src
	${CMAKE_CURRENT_BINARY_DIR}
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
	src/gpx.cpp
	src/oiioutils.cpp
	src/ffmpegutils.cpp
	src/decoder.cpp
	src/encoder.cpp
	src/frame.cpp
	src/media.cpp
	src/stream.cpp
	src/audioparams.cpp
	src/videoparams.cpp
	src/renderer.cpp
	src/main.cpp

 	utmconvert/utmconvert.cpp
)

# 
# QT
#
SET(QT_LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets)
SET(OIIO_LIBRARIES OpenImageIO) ## OpenImageIO_Util)
SET(CMAKE_AUTOMOC ON)

#
# LINK
#
#add_library(core STATIC ${OVERLAY_CORE_SOURCES})

#
# PROPRERTIES
#

#
# BINARIES
# 
add_executable(gpx2video ${GPX2VIDEO_SOURCES})
target_link_libraries(gpx2video gpxlib ${LIBAVUTIL_LIBRARIES} ${LIBAVFORMAT_LIBRARIES} ${LIBAVCODEC_LIBRARIES} ${LIBAVFILTER_LIBRARIES} ${LIBSWRESAMPLE_LIBRARIES} ${LIBSWSCALE_LIBRARIES} ${OIIO_LIBRARIES} ${LIBGEOGRAPHIC_LIBRARIES})

#
# INSTALL
#
install(TARGETS gpx2video DESTINATION bin)

#
# SUB DIRECTORIES
#
add_subdirectory(gpxlib)
add_subdirectory(tests)
