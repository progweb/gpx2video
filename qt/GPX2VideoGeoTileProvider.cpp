#include "GPX2VideoGeoTileProvider.h"
#include "log.h"


QT_BEGIN_NAMESPACE

GPX2VideoGeoTileProvider::GPX2VideoGeoTileProvider(
	MapSettings::Source source,
	const QGeoMapType &map_type,
	const QGeoCameraCapabilities &camera_capabilities)
	: source_(source) 
	, map_type_(map_type)
	, camera_capabilities_(camera_capabilities) {

	log_call();
}


GPX2VideoGeoTileProvider::~GPX2VideoGeoTileProvider() {
	log_call();
}


const MapSettings::Source &GPX2VideoGeoTileProvider::source() const {
	log_call();

	return source_;
}


int GPX2VideoGeoTileProvider::minimumZoomLevel() const {
	log_call();

	return MapSettings::getMinZoom(source_);
}


int GPX2VideoGeoTileProvider::maximumZoomLevel() const {
	log_call();

	return MapSettings::getMaxZoom(source_);
}


QString GPX2VideoGeoTileProvider::getRepoURI() const {
	log_call();

	return QString::fromStdString(MapSettings::getRepoURI(source_));
}


const QGeoMapType &GPX2VideoGeoTileProvider::mapType() const {
	log_call();

	return map_type_;
}


QGeoCameraCapabilities GPX2VideoGeoTileProvider::cameraCapabilities() const {
	log_call();

	return camera_capabilities_;
}

QT_END_NAMESPACE

