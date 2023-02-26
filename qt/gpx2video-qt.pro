QT += core
QT += gui
QT += widgets
QT += quick
QT += qml
QT += sql
QT += network
QT += concurrent
QT += location

qtHaveModule(location-private){
    QT += location-private
}else{
    QT += location
}
qtHaveModule(positioning-private){
    QT += positioning-private
}else{
    QT += positioning
}

TEMPLATE = app

CONFIG += c++14
CONFIG += utf8_source
CONFIG += link_pkgconfig

PKGCONFIG += libevent
PKGCONFIG += libssl
PKGCONFIG += libcurl
PKGCONFIG += cairo
PKGCONFIG += OpenImageIO

# using static plugin at demo
DEFINES += GPX2VideoPlugin_Static
contains(DEFINES, GPX2VideoPlugin_Static){
    LOCATION_PLUGIN_DESTDIR = $${OUT_PWD}/
    LOCATION_PLUGIN_NAME    = GPX2VideoFactory                                                                                                                  
}

INCLUDEPATH += \
	$$PWD/../ \
	$$PWD/qtlocation-opensource-src/include/ \
	$$PWD/qtlocation-opensource-src/include/QtLocation/5.15.7/ \
	$$PWD/qtlocation-opensource-src/include/QtPositioning/5.15.7/

HEADERS += \
	$$PWD/GPX2VideoGeoTileFetcher.h \
	$$PWD/GPX2VideoGeoTileProvider.h \
	$$PWD/GPX2VideoGeoFileTileCache.h \
	$$PWD/GPX2VideoGeoTiledMapReply.h \
    $$PWD/GPX2VideoGeoServiceProviderFactory.h \
    $$PWD/GPX2VideoGeoTiledMappingManagerEngine.h

SOURCES += \
    $$PWD/log.c \
    $$PWD/map.cpp \
    $$PWD/track.cpp \
	$$PWD/GPX2VideoGeoTileFetcher.cpp \
	$$PWD/GPX2VideoGeoTileProvider.cpp \
	$$PWD/GPX2VideoGeoFileTileCache.cpp \
	$$PWD/GPX2VideoGeoTiledMapReply.cpp \
    $$PWD/GPX2VideoGeoServiceProviderFactory.cpp \
    $$PWD/GPX2VideoGeoTiledMappingManagerEngine.cpp \
    $$PWD/main.cpp

RESOURCES += \
	$$PWD/qml/qml.qrc

DISTFILES += \
    $$PWD/gpx2video_plugin.json
