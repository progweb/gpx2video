#include <QtNetwork/QNetworkRequest>

#include "GPX2VideoGeoTiledMapReply.h"
#include "GPX2VideoGeoTileFetcher.h"
#include "log.h"


QT_BEGIN_NAMESPACE

GPX2VideoGeoTileFetcher::GPX2VideoGeoTileFetcher(
		const QVector<GPX2VideoGeoTileProvider *> &providers,
		const QVariantMap &parameters,
		QGeoMappingManagerEngine *parent) 
	: QGeoTileFetcher(parent) 
	, providers_(providers)
	, networkManager_(new QNetworkAccessManager(this)) {
	log_call();
}


QGeoTiledMapReply * GPX2VideoGeoTileFetcher::getTileImage(const QGeoTileSpec &spec) {
	QNetworkRequest request;

	log_call();

	int provider = spec.mapId() - 1;

	if ((provider < 0) || (provider >= providers_.size()))
		return NULL;

	log_info("Get tile image [map id: %d -> %d] %dx%d (z: %d)",
		   spec.mapId(), providers_[provider]->source(), spec.x(), spec.y(), spec.zoom());

	request.setRawHeader("Accept", "*/*");
	//request.setHeader(QNetworkRequest::UserAgentHeader, user_agent_);
	request.setHeader(QNetworkRequest::UserAgentHeader, "gpx2video");
	request.setUrl(getUrl(spec));

//	qDebug() << "GeoTileFetcherMyMap::getTileImage" << request.url();

	QNetworkReply *reply = networkManager_->get(request);

	return new GPX2VideoGeoTiledMapReply(reply, spec);
}


QString GPX2VideoGeoTileFetcher::getUrl(const QGeoTileSpec &spec) const {
	int provider = spec.mapId() - 1;

	QString url = providers_[provider]->getRepoURI();

	int max_zoom = providers_[provider]->maximumZoomLevel();

	const QString z = QString("%1").arg(spec.zoom());
	const QString y = QString("%1").arg(spec.y());
	const QString x = QString("%1").arg(spec.x());
	const QString s = QString("%1").arg(max_zoom - spec.zoom());
	const QString r = QString("%1").arg((int) (random() % 4));

	return url.replace("#X", x).replace("#Y", y).replace("#Z", z).replace("#S", s).replace("#R", r);
}


QT_END_NAMESPACE

