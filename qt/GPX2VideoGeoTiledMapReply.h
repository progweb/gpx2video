#include <QtNetwork/QNetworkReply>

#include <QtLocation/private/qgeotiledmapreply_p.h>


QT_BEGIN_NAMESPACE

class GPX2VideoGeoTiledMapReply : public QGeoTiledMapReply {
	Q_OBJECT

public:
	GPX2VideoGeoTiledMapReply(QNetworkReply *reply, const QGeoTileSpec &spec, QObject *parent=nullptr);

private:
	Q_SLOT void networkReplyFinished();
	Q_SLOT void networkReplyError(QNetworkReply::NetworkError error);
};

QT_END_NAMESPACE

