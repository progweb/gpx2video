#include <QtNetwork/QNetworkAccessManager>

#include <QtLocation/private/qgeotilefetcher_p.h>


QT_BEGIN_NAMESPACE

class GPX2VideoGeoTileFetcher : public QGeoTileFetcher {
	Q_OBJECT

public:
	GPX2VideoGeoTileFetcher(
		const QVariantMap &parameters,
		QGeoMappingManagerEngine *parent);

private:
	QGeoTiledMapReply* getTileImage(const QGeoTileSpec &spec) override;
	QString getUrl(const QGeoTileSpec &spec) const;

private:
	QNetworkAccessManager *networkManager_;
};

QT_END_NAMESPACE

