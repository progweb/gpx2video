#
# CONFIGURATION
#
add_definitions(-Wno-unused-parameter)

#
# LIBS
# 
FIND_PACKAGE(EXPAT REQUIRED)
include_directories(. .. ${EXPAT_INCLUDE_DIRS})

#
# INCLUDE
#
include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/src
)

#
# HEADERS
#
set(LAYOUTLIB_HEADERS
)

#
# SOURCES
#
set(LAYOUTLIB_SOURCES
	Node.cpp
#	Decimal.cpp
#	DateTime.cpp
	String.cpp
	Unsigned.cpp
#	DGPSStation.cpp
#	Fix.cpp
#	Degrees.cpp
#	Longitude.cpp
#	Latitude.cpp
#	Bounds.cpp
#	PTSeg.cpp
#	PT.cpp
#	EMail.cpp
#	Link.cpp
#	Person.cpp
#	URI.cpp
#	Copyright.cpp
#	TRKSeg.cpp
#	Extensions.cpp
	Widget.cpp
#	WPT.cpp
#	RTE.cpp
#	Metadata.cpp
	Layout.cpp
	Parser.cpp
#	Writer.cpp
	Report.cpp
	ReportCerr.cpp
#	Algorithm.cpp
)

#
# LIBRARIES
#
add_library(layoutlib STATIC ${LAYOUTLIB_SOURCES})
target_link_libraries(layoutlib ${EXPAT_LIBRARIES})

