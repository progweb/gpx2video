#include <QtLocation/private/qgeofiletilecache_p.h>


QT_BEGIN_NAMESPACE

class GPX2VideoGeoFileTileCache : public QGeoFileTileCache {
	Q_OBJECT

public:
	GPX2VideoGeoFileTileCache(const QString &directory=QString(), QObject *parent=NULL);
	~GPX2VideoGeoFileTileCache();

	QSharedPointer<QGeoTileTexture> get(const QGeoTileSpec &spec);

protected:
	void init() override;
	void loadTiles();
	void loadTiles(int zoom);

	QSharedPointer<QGeoTileTexture> getFromOfflineStorage(const QGeoTileSpec &spec);

	QString tileSpecToFilename(const QGeoTileSpec &spec, const QString &format, const QString &directory) const override;
	QGeoTileSpec filenameToTileSpec(const QString &filename) const override;
};

QT_END_NAMESPACE

