project(GPX2VIDEO)

cmake_minimum_required(VERSION 2.8)

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

PKG_CHECK_MODULES(LIBSWSCALE REQUIRED libswscale>=3.0.0)
include_directories(${LIBSWSCALE_INCLUDE_DIRS})

FIND_PACKAGE(Qt5 COMPONENTS Core Gui Widgets REQUIRED)

#
# INCLUDE
#
include_directories(
	${CMAKE_CURRENT_BINARY_DIR}/src
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
set(OVERLAY_FF_SOURCES
#	tests/test-decoder.c
	tests/test-packet.c
)
set(OVERLAY_QT_SOURCES
	tests/main5-qt.cpp
)
set(OVERLAY_OIIO_SOURCES
	tests/main6-oiio.cpp
)

set(GPX2VIDEO_SOURCES
	src/ffmpegutils.cpp
	src/decoder.cpp
	src/encoder.cpp
	src/frame.cpp
	src/media.cpp
	src/stream.cpp
	src/audioparams.cpp
	src/videoparams.cpp
	src/main.cpp
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
add_executable(overlay-ff ${OVERLAY_FF_SOURCES})
target_link_libraries(overlay-ff ${LIBAVUTIL_LIBRARIES} ${LIBAVFORMAT_LIBRARIES} ${LIBAVCODEC_LIBRARIES} ${LIBAVFILTER_LIBRARIES} ${LIBSWSCALE_LIBRARIES})

#add_executable(overlay-qt ${OVERLAY_QT_SOURCES})
#target_link_libraries(overlay-qt ${LIBAVUTIL_LIBRARIES} ${LIBAVFORMAT_LIBRARIES} ${LIBAVCODEC_LIBRARIES} ${LIBAVFILTER_LIBRARIES} ${LIBSWSCALE_LIBRARIES} ${QT_LIBRARIES})

add_executable(overlay-oiio ${OVERLAY_OIIO_SOURCES})
target_link_libraries(overlay-oiio ${LIBAVUTIL_LIBRARIES} ${LIBAVFORMAT_LIBRARIES} ${LIBAVCODEC_LIBRARIES} ${LIBAVFILTER_LIBRARIES} ${LIBSWSCALE_LIBRARIES} ${OIIO_LIBRARIES})

add_executable(gpx2video ${GPX2VIDEO_SOURCES})
target_link_libraries(gpx2video ${LIBAVUTIL_LIBRARIES} ${LIBAVFORMAT_LIBRARIES} ${LIBAVCODEC_LIBRARIES} ${LIBAVFILTER_LIBRARIES} ${LIBSWSCALE_LIBRARIES} ${OIIO_LIBRARIES})

#
# INSTALL
#
install(TARGETS overlay-ff DESTINATION bin)
#install(TARGETS overlay-qt DESTINATION bin)
install(TARGETS overlay-oiio DESTINATION bin)
install(TARGETS gpx2video DESTINATION bin)

#
# SUB DIRECTORIES
#
#add_subdirectory(tools)

##
## TESTS
##
#if(CMAKE_BUILD_TYPE MATCHES "DEBUG")
#  add_subdirectory(tests)
#endif()

