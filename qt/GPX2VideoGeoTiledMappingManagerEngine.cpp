#include <QtCore/QProcessEnvironment>
#include <QtLocation/private/qgeocameracapabilities_p.h>
#include <QtLocation/private/qgeotiledmap_p.h>
#include <QtLocation/private/qgeomaptype_p.h>


#include "GPX2VideoGeoTileFetcher.h"
#include "GPX2VideoGeoFileTileCache.h"
#include "GPX2VideoGeoTiledMappingManagerEngine.h"
#include "log.h"


QT_BEGIN_NAMESPACE

GPX2VideoGeoTiledMappingManagerEngine::GPX2VideoGeoTiledMappingManagerEngine(
	const QVariantMap &parameters,
	QGeoServiceProvider::Error *error,
	QString *errorString) {
	QString cache_dir;

	const QByteArray plugin_name = "GPX2Video";

	log_call();

	// Build cache directory used by gpx2video
	cache_dir = QProcessEnvironment::systemEnvironment().value("HOME", "~");
	cache_dir += QString("/.gpx2video/cache");

	// Map camera capabilitied
	QGeoCameraCapabilities camera_caps;
	camera_caps.setMinimumZoomLevel(0.0);
	camera_caps.setMaximumZoomLevel(20.0);
	camera_caps.setSupportsBearing(true);
	camera_caps.setSupportsTilting(true);
	camera_caps.setMinimumTilt(0);
	camera_caps.setMaximumTilt(80);
	camera_caps.setMinimumFieldOfView(20.0);
	camera_caps.setMaximumFieldOfView(120.0);
	camera_caps.setOverzoomEnabled(true);
	setCameraCapabilities(camera_caps);

	// Tile size
	setTileSize(QSize(256, 256));

	// Tile providers
	providers_.push_back(new GPX2VideoGeoTileProvider(
		MapSettings::SourceOpenStreetMap,
		QGeoMapType(QGeoMapType::StreetMap, tr("Street Map"), tr("Street map view in daylight mode"), false, false, 1, plugin_name, camera_caps),
		camera_caps
	));
	providers_.push_back(new GPX2VideoGeoTileProvider(
		MapSettings::SourceOpenStreetMapRenderer,
		QGeoMapType(QGeoMapType::StreetMap, tr("Street Map"), tr("Street map view in daylight mode"), false, false, 2, plugin_name, camera_caps),
		camera_caps
	));
	providers_.push_back(new GPX2VideoGeoTileProvider(
		MapSettings::SourceOpenAerialMap,
		QGeoMapType(QGeoMapType::StreetMap, tr("Street Map"), tr("Street map view in daylight mode"), false, false, 3, plugin_name, camera_caps),
		camera_caps
	));
	providers_.push_back(new GPX2VideoGeoTileProvider(
		MapSettings::SourceMapsForFree,
		QGeoMapType(QGeoMapType::StreetMap, tr("Street Map"), tr("Street map view in daylight mode"), false, false, 4, plugin_name, camera_caps),
		camera_caps
	));
	providers_.push_back(new GPX2VideoGeoTileProvider(
		MapSettings::SourceOpenCycleMap,
		QGeoMapType(QGeoMapType::StreetMap, tr("Street Map"), tr("Street map view in daylight mode"), false, false, 5, plugin_name, camera_caps),
		camera_caps
	));
	providers_.push_back(new GPX2VideoGeoTileProvider(
		MapSettings::SourceOpenTopoMap,
		QGeoMapType(QGeoMapType::StreetMap, tr("Street Map"), tr("Street map view in daylight mode"), false, false, 6, plugin_name, camera_caps),
		camera_caps
	));
	providers_.push_back(new GPX2VideoGeoTileProvider(
		MapSettings::SourceOSMPublicTransport,
		QGeoMapType(QGeoMapType::StreetMap, tr("Street Map"), tr("Street map view in daylight mode"), false, false, 7, plugin_name, camera_caps),
		camera_caps
	));
	providers_.push_back(new GPX2VideoGeoTileProvider(
		MapSettings::SourceGoogleStreet,
		QGeoMapType(QGeoMapType::StreetMap, tr("Street Map"), tr("Street map view in daylight mode"), false, false, 8, plugin_name, camera_caps),
		camera_caps
	));
	providers_.push_back(new GPX2VideoGeoTileProvider(
		MapSettings::SourceGoogleSatellite,
		QGeoMapType(QGeoMapType::SatelliteMapDay, tr("Street Map"), tr("Street map view in daylight mode"), false, false, 9, plugin_name, camera_caps),
		camera_caps
	));
	providers_.push_back(new GPX2VideoGeoTileProvider(
		MapSettings::SourceGoogleHybrid,
		QGeoMapType(QGeoMapType::SatelliteMapDay, tr("Street Map"), tr("Street map view in daylight mode"), false, false, 10, plugin_name, camera_caps),
		camera_caps
	));
	providers_.push_back(new GPX2VideoGeoTileProvider(
		MapSettings::SourceVirtualEarthStreet,
		QGeoMapType(QGeoMapType::StreetMap, tr("Street Map"), tr("Street map view in daylight mode"), false, false, 11, plugin_name, camera_caps),
		camera_caps
	));
	providers_.push_back(new GPX2VideoGeoTileProvider(
		MapSettings::SourceVirtualEarthSatellite,
		QGeoMapType(QGeoMapType::SatelliteMapDay, tr("Street Map"), tr("Street map view in daylight mode"), false, false, 12, plugin_name, camera_caps),
		camera_caps
	));
	providers_.push_back(new GPX2VideoGeoTileProvider(
		MapSettings::SourceVirtualEarthHybrid,
		QGeoMapType(QGeoMapType::SatelliteMapDay, tr("Street Map"), tr("Street map view in daylight mode"), false, false, 13, plugin_name, camera_caps),
		camera_caps
	));
	providers_.push_back(new GPX2VideoGeoTileProvider(
		MapSettings::SourceOSMCTrails,
		QGeoMapType(QGeoMapType::StreetMap, tr("Street Map"), tr("Street map view in daylight mode"), false, false, 14, plugin_name, camera_caps),
		camera_caps
	));
	providers_.push_back(new GPX2VideoGeoTileProvider(
		MapSettings::SourceIGNEssentielMap,
		QGeoMapType(QGeoMapType::StreetMap, tr("Street Map"), tr("Street map view in daylight mode"), false, false, 15, plugin_name, camera_caps),
		camera_caps
	));
	providers_.push_back(new GPX2VideoGeoTileProvider(
		MapSettings::SourceIGNEssentielPhoto,
		QGeoMapType(QGeoMapType::SatelliteMapDay, tr("Street Map"), tr("Street map view in daylight mode"), false, false, 16, plugin_name, camera_caps),
		camera_caps
	));
	updateMapTypes();

	// Tile cache
	const int cache_size = 512 * 1024 * 1024;
	GPX2VideoGeoFileTileCache *tile_cache = new GPX2VideoGeoFileTileCache(providers_, cache_dir, this);
	tile_cache->setCostStrategyDisk(QGeoFileTileCache::ByteSize);
	tile_cache->setMaxDiskUsage(cache_size);
	tile_cache->setCostStrategyMemory(QGeoFileTileCache::ByteSize);
	tile_cache->setMaxMemoryUsage(0); //cache_size);
	tile_cache->setCostStrategyTexture(QGeoFileTileCache::ByteSize);
	tile_cache->setExtraTextureUsage(0);//cache_size);
	setTileCache(tile_cache);

	// Tile fetcher
	GPX2VideoGeoTileFetcher *tile_fetcher = new GPX2VideoGeoTileFetcher(providers_, parameters, this);
	setTileFetcher(tile_fetcher);

	// Result
	*error = QGeoServiceProvider::NoError;
	errorString->clear();
}


GPX2VideoGeoTiledMappingManagerEngine::~GPX2VideoGeoTiledMappingManagerEngine() {
	log_call();
}


QGeoMap * GPX2VideoGeoTiledMappingManagerEngine::createMap() {
	log_call();

	QGeoTiledMap *map = new QGeoTiledMap(this, nullptr);

	map->setPrefetchStyle(QGeoTiledMap::NoPrefetching);

	return map;
}


void GPX2VideoGeoTiledMappingManagerEngine::updateMapTypes() {
	QList<QGeoMapType> map_types;

	foreach (GPX2VideoGeoTileProvider *provider, providers_) {
		map_types << provider->mapType();
	}

	setSupportedMapTypes(map_types);
}

QT_END_NAMESPACE

