#include <QtCore/QFile>

#include "GPX2VideoGeoTiledMapReply.h"
#include "log.h"


QT_BEGIN_NAMESPACE

GPX2VideoGeoTiledMapReply::GPX2VideoGeoTiledMapReply(QNetworkReply *reply, const QGeoTileSpec &spec, QObject *parent) 
	: QGeoTiledMapReply(spec, parent) {
	log_call();

	if (!reply) {
		setError(UnknownError, QStringLiteral("Null reply"));
		return;
	}

	connect(reply, SIGNAL(finished()), this, SLOT(networkReplyFinished()));
	connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
			this, SLOT(networkReplyError(QNetworkReply::NetworkError)));
	connect(this, &QGeoTiledMapReply::aborted, reply, &QNetworkReply::abort);
	connect(this, &QObject::destroyed, reply, &QObject::deleteLater);

//	QFile file("/home/eok065/Projects/qt-map/cache/15/16/tile_23705_33106.png");
//	file.open(QIODevice::ReadOnly);
//
//	setMapImageFormat("png");
////	setMapImageData(reply->readAll());
//	setMapImageData(file.readAll());
//	setFinished(true);
}


void GPX2VideoGeoTiledMapReply::networkReplyFinished() {
	QNetworkReply *reply = static_cast<QNetworkReply *>(sender());

	log_call();

	reply->deleteLater();

	if (reply->error() != QNetworkReply::NoError)
		return;

	setMapImageFormat("png");
	setMapImageData(reply->readAll());
	setFinished(true);
}


void GPX2VideoGeoTiledMapReply::networkReplyError(QNetworkReply::NetworkError error) {       
	QNetworkReply *reply = static_cast<QNetworkReply *>(sender());

	log_call();

	reply->deleteLater();

	if (error == QNetworkReply::OperationCanceledError)
		setFinished(true);
	else
		setError(QGeoTiledMapReply::CommunicationError, reply->errorString());
}

QT_END_NAMESPACE

