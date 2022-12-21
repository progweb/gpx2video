#include <QtCore/QProcessEnvironment>
#include <QtLocation/private/qgeocameracapabilities_p.h>
#include <QtLocation/private/qgeotiledmap_p.h>


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

	// Tile cache
	const int cache_size = 512 * 1024 * 1024;
	GPX2VideoGeoFileTileCache *tile_cache = new GPX2VideoGeoFileTileCache(cache_dir, this);
	tile_cache->setCostStrategyDisk(QGeoFileTileCache::ByteSize);
	tile_cache->setMaxDiskUsage(cache_size);
	tile_cache->setCostStrategyMemory(QGeoFileTileCache::ByteSize);
	tile_cache->setMaxMemoryUsage(cache_size);
	tile_cache->setCostStrategyTexture(QGeoFileTileCache::ByteSize);
	tile_cache->setExtraTextureUsage(cache_size);
	setTileCache(tile_cache);

	// Tile fetcher
	GPX2VideoGeoTileFetcher *tile_fetcher = new GPX2VideoGeoTileFetcher(parameters, this);
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

QT_END_NAMESPACE

