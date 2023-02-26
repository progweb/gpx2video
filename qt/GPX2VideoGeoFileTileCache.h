#ifndef __GPX2VIDEOGEOFILETILECACHE_H__
#define __GPX2VIDEOGEOFILETILECACHE_H__

#include <QtLocation/private/qgeofiletilecache_p.h>

#include "GPX2VideoGeoTileProvider.h"


QT_BEGIN_NAMESPACE

class GPX2VideoGeoFileTileCache : public QGeoFileTileCache {
	Q_OBJECT

public:
	GPX2VideoGeoFileTileCache(
			const QVector<GPX2VideoGeoTileProvider *> &providers,
			const QString &directory=QString(), 
			QObject *parent=NULL);
	~GPX2VideoGeoFileTileCache();

	QSharedPointer<QGeoTileTexture> get(const QGeoTileSpec &spec);

protected:
	void init() override;
	void loadTiles();
	void loadTiles(int provider, int zoom);

	QSharedPointer<QGeoTileTexture> getFromOfflineStorage(const QGeoTileSpec &spec);

	QString tileSpecToFilename(const QGeoTileSpec &spec, const QString &format, const QString &directory) const override;
	QString tileSpecToFilename(const QGeoTileSpec &spec, const QString &format, int provider) const;
	QGeoTileSpec filenameToTileSpec(const QString &filename) const override;

private:
	const QVector<GPX2VideoGeoTileProvider *> &providers_;
};

QT_END_NAMESPACE

#endif

