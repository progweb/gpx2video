# 
# GTK resources
#

include(GlibCompileResourcesSupport)

# Set the resources to bundle.
set(RESOURCE_UI_LIST
	window.ui
)

# Compile the resources.
compile_gresources(RESOURCE_UI_FILE
	XML_OUT
	TYPE EMBED_C
	TARGET "ui-resources.c"
	PREFIX "/com/progweb/gpx2video/ui"
	SOURCE_DIR ui
	RESOURCES ${RESOURCE_UI_LIST}
)

set(GPX2VIDEO_UI_RESOURCES
	${RESOURCE_UI_FILE}
)

add_library(uires ${GPX2VIDEO_UI_RESOURCES})

