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
	Decimal.cpp
	String.cpp
	Unsigned.cpp
	Map.cpp
	Widget.cpp
	Layout.cpp
	Parser.cpp
	Report.cpp
	ReportCerr.cpp
)

#
# LIBRARIES
#
add_library(layoutlib STATIC ${LAYOUTLIB_SOURCES})
target_link_libraries(layoutlib ${EXPAT_LIBRARIES})

