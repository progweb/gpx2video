# Configuration
remove_definitions(-Wall)
add_definitions(-Wno-unused-parameter)

# Libs
FIND_PACKAGE(EXPAT REQUIRED)
include_directories(. .. ${EXPAT_INCLUDE_DIRS})

# Include
include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Headers
set(TCXLIB_HEADERS
)

# Sources
set(TCXLIB_SOURCES
	Activities.cpp
	Activity.cpp
	Node.cpp
	Decimal.cpp
	DateTime.cpp
	String.cpp
	Unsigned.cpp
	Degrees.cpp
	Longitude.cpp
	Latitude.cpp
	Extensions.cpp
	Lap.cpp
	HeartRate.cpp
	Position.cpp
	Track.cpp
	TrackPoint.cpp
	TCX.cpp
	Parser.cpp
	Report.cpp
	ReportCerr.cpp
)

# Libraries
add_library(tcxlib STATIC ${TCXLIB_SOURCES})
target_link_libraries(tcxlib ${EXPAT_LIBRARIES})

