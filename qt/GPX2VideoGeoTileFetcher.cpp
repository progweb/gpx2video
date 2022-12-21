#include <QtNetwork/QNetworkRequest>

#include "GPX2VideoGeoTiledMapReply.h"
#include "GPX2VideoGeoTileFetcher.h"
#include "log.h"


QT_BEGIN_NAMESPACE

GPX2VideoGeoTileFetcher::GPX2VideoGeoTileFetcher(
	const QVariantMap &parameters,
	QGeoMappingManagerEngine *parent) 
	: QGeoTileFetcher(parent) 
	, networkManager_(new QNetworkAccessManager(this)) {
	log_call();
}


QGeoTiledMapReply * GPX2VideoGeoTileFetcher::getTileImage(const QGeoTileSpec &spec) {
	QNetworkRequest request;

	log_call();

	log_debug("Get tile image %dx%d (z: %d)", spec.x(), spec.y(), spec.zoom());

	request.setRawHeader("Accept", "*/*");
	//request.setHeader(QNetworkRequest::UserAgentHeader, _userAgent);
	request.setUrl(getUrl(spec));

	qDebug() << "GeoTileFetcherMyMap::getTileImage" << request.url();

	QNetworkReply *reply = networkManager_->get(request);

	return new GPX2VideoGeoTiledMapReply(reply, spec);
}


QString GPX2VideoGeoTileFetcher::getUrl(const QGeoTileSpec &spec) const {
	QString url = QString("https://wxs.ign.fr/essentiels/geoportail/wmts?SERVICE=WMTS&REQUEST=GetTile&VERSION=1.0.0&LAYER=GEOGRAPHICALGRIDSYSTEMS.PLANIGNV2&TILEMATRIXSET=PM&TILEMATRIX=#Z&TILECOL=#X&TILEROW=#Y&STYLE=normal&FORMAT=image/png");

	const QString z = QString("%1").arg(spec.zoom()); //, 2, 10, QLatin1Char('0')).toUpper();
	const QString y = QString("%1").arg(spec.y()); //, 8, 16, QLatin1Char('0')).toUpper();
	const QString x = QString("%1").arg(spec. x()); //, 8, 16, QLatin1Char('0')).toUpper();

	return url.replace("#X", x).replace("#Y", y).replace("#Z", z);
}


QT_END_NAMESPACE

