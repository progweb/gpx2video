#
# CONFIGURATION
#
remove_definitions(-Wall)

#
# LIBS
# 
FIND_PACKAGE(EXPAT REQUIRED)
include_directories(. .. ${EXPAT_INCLUDE_DIRS})

#
# SOURCES
#
set(EXTRACT_GPX_SOURCES
#	test-decoder.c
	test-extract.c
)
set(OVERLAY_FF_SOURCES
#	test-decoder.c
	test-packet.c
)
set(OVERLAY_QT_SOURCES
	main5-qt.cpp
)
set(OVERLAY_OIIO_SOURCES
	main6-oiio.cpp
)

set(GPXTRK_SOURCES
	gpxtrk.cpp
)

set(TIME_SOURCES
	time.c
)

#
# BINARIES
# 
add_executable(extract-gpx ${EXTRACT_GPX_SOURCES})
target_link_libraries(extract-gpx ${LIBAVUTIL_LIBRARIES} ${LIBAVFORMAT_LIBRARIES} ${LIBAVCODEC_LIBRARIES} ${LIBAVFILTER_LIBRARIES} ${LIBSWSCALE_LIBRARIES})

#add_executable(overlay-ff ${OVERLAY_FF_SOURCES})
#target_link_libraries(overlay-ff ${LIBAVUTIL_LIBRARIES} ${LIBAVFORMAT_LIBRARIES} ${LIBAVCODEC_LIBRARIES} ${LIBAVFILTER_LIBRARIES} ${LIBSWSCALE_LIBRARIES})

#add_executable(overlay-qt ${OVERLAY_QT_SOURCES})
#target_link_libraries(overlay-qt ${LIBAVUTIL_LIBRARIES} ${LIBAVFORMAT_LIBRARIES} ${LIBAVCODEC_LIBRARIES} ${LIBAVFILTER_LIBRARIES} ${LIBSWSCALE_LIBRARIES} ${QT_LIBRARIES})

#add_executable(overlay-oiio ${OVERLAY_OIIO_SOURCES})
#target_link_libraries(overlay-oiio ${LIBAVUTIL_LIBRARIES} ${LIBAVFORMAT_LIBRARIES} ${LIBAVCODEC_LIBRARIES} ${LIBAVFILTER_LIBRARIES} ${LIBSWSCALE_LIBRARIES} ${OIIO_LIBRARIES})

#add_executable(gpxtrk ${GPXTRK_SOURCES})
#target_link_libraries(gpxtrk gpxlib ${EXPAT_LIBRARIES})

add_executable(time ${TIME_SOURCES})

#
# INSTALL
#
#install(TARGETS overlay-ff DESTINATION bin)
#install(TARGETS overlay-qt DESTINATION bin)
#install(TARGETS overlay-oiio DESTINATION bin)

