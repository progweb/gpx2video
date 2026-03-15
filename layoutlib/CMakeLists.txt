# Configuration
add_definitions(-Wno-unused-parameter)

# Libs
FIND_PACKAGE(EXPAT REQUIRED)
include_directories(. .. ${EXPAT_INCLUDE_DIRS})

# Include
include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Headers
set(LAYOUTLIB_HEADERS
)

# Sources
set(LAYOUTLIB_SOURCES
	Node.cpp
	Boolean.cpp
	Decimal.cpp
	String.cpp
	Unsigned.cpp
	Map.cpp
	Track.cpp
	Widget.cpp
	Layout.cpp
	Parser.cpp
	Report.cpp
	ReportCerr.cpp
)

# Libraries
add_library(layoutlib STATIC ${LAYOUTLIB_SOURCES})
target_link_libraries(layoutlib ${EXPAT_LIBRARIES})

