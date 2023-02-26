#include <QtNetwork/QNetworkAccessManager>
#include <QtLocation/private/qgeotilefetcher_p.h>

#include "GPX2VideoGeoTileProvider.h"


QT_BEGIN_NAMESPACE

class GPX2VideoGeoTileFetcher : public QGeoTileFetcher {
	Q_OBJECT

public:
	GPX2VideoGeoTileFetcher(
			const QVector<GPX2VideoGeoTileProvider *> &providers,
			const QVariantMap &parameters,
			QGeoMappingManagerEngine *parent);

private:
	QGeoTiledMapReply* getTileImage(const QGeoTileSpec &spec) override;
	QString getUrl(const QGeoTileSpec &spec) const;

private:
	const QVector<GPX2VideoGeoTileProvider *> providers_;

	QNetworkAccessManager *networkManager_;
};

QT_END_NAMESPACE

