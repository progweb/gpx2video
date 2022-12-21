#include <QDir>
#include <QDirIterator>
#include <QFileInfo>

#include "GPX2VideoGeoFileTileCache.h"
#include "log.h"


QT_BEGIN_NAMESPACE

GPX2VideoGeoFileTileCache::GPX2VideoGeoFileTileCache(const QString &directory, QObject *parent) 
	: QGeoFileTileCache(directory, parent) {
	log_call();
}


GPX2VideoGeoFileTileCache::~GPX2VideoGeoFileTileCache() {
	log_call();
}


QSharedPointer<QGeoTileTexture> GPX2VideoGeoFileTileCache::get(const QGeoTileSpec &spec) {
	log_call();

	QSharedPointer<QGeoTileTexture> tt = getFromMemory(spec);

	// Mkdir
	QDir dir(directory_ + "/15");
	dir.mkdir(QString::number(spec.zoom()));

	if (tt)
		return tt;

	if ((tt = getFromOfflineStorage(spec)))
		return tt;

	return getFromDisk(spec);
}


void GPX2VideoGeoFileTileCache::init() {
	log_call();

	loadTiles();
}


void GPX2VideoGeoFileTileCache::loadTiles() {
	bool ok = false;

	log_call();

	// MapSource: 15
	QDir dir(directory_ + "/15");
	dir.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);

	QStringList directories = dir.entryList();

	for (int i=0; i<directories.size(); i++) {
		int zoom = directories.at(i).toInt(&ok);
		if (!ok)
			continue;

		loadTiles(zoom);
	}
}


void GPX2VideoGeoFileTileCache::loadTiles(int zoom) {
	log_call();

	// Filter
	QStringList formats;
	formats << QLatin1String("*.*");

	// MapSource: 15
	QString dirname = QString("%1/%2/%3").arg(directory_).arg(15).arg(zoom);

	QDir dir(dirname);
	QStringList files = dir.entryList(formats, QDir::Files);

	for (int i=0; i<files.size(); i++) {
		QString filename = dir.filePath(files.at(i));

		QGeoTileSpec spec = filenameToTileSpec(filename);

		if (spec.zoom() == -1)
			continue;

		addToDiskCache(spec, filename);
	}
}


QSharedPointer<QGeoTileTexture> GPX2VideoGeoFileTileCache::getFromOfflineStorage(const QGeoTileSpec &spec) {
	log_call();

	QSharedPointer<QGeoCachedTileDisk> td = diskCache_.object(spec);
	if (td) {
		const QString format = QFileInfo(td->filename).suffix();
		QFile file(td->filename);
		file.open(QIODevice::ReadOnly);
		QByteArray bytes = file.readAll();
		file.close();

		QImage image;
		// Some tiles from the servers could be valid images but the tile fetcher
		// might be able to recognize them as tiles that should not be shown.
		// If that's the case, the tile fetcher should write "NoRetry" inside the file.
		if (isTileBogus(bytes)) {
			QSharedPointer<QGeoTileTexture> tt(new QGeoTileTexture);
			tt->spec = spec;
			tt->image = image;
			return tt;
		}

		// This is a truly invalid image. The fetcher should try again.
		if (!image.loadFromData(bytes)) {
			handleError(spec, QLatin1String("Problem with tile image"));
			return QSharedPointer<QGeoTileTexture>(0);
		}

		// Converting it here, instead of in each QSGTexture::bind()
		if (image.format() != QImage::Format_RGB32 && image.format() != QImage::Format_ARGB32_Premultiplied)
			image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);

		addToMemoryCache(spec, bytes, format);
		QSharedPointer<QGeoTileTexture> tt = addToTextureCache(td->spec, image);
		if (tt)
			return tt;
	
		log_error("failure...\n");

	}

		qDebug() << "plugin: " << spec.plugin();
		log_error("failure... %d %dx%d (zoom: %d)\n", spec.mapId(), spec.x(), spec.y(), spec.zoom());

	return QSharedPointer<QGeoTileTexture>();
}



QString GPX2VideoGeoFileTileCache::tileSpecToFilename(const QGeoTileSpec &spec, const QString &format, const QString &directory) const {
//	QString filename = spec.plugin();
//	filename += QLatin1String("-");
//	filename += QString::number(spec.mapId());
//	filename += QLatin1String("-");
//	filename += QString::number(spec.zoom());
//	filename += QLatin1String("-");
//	filename += QString::number(spec.x());
//	filename += QLatin1String("-");
//	filename += QString::number(spec.y());                                                                                                                  
//
//	//Append version if real version number to ensure backwards compatibility and eviction of old tiles
//	if (spec.version() != -1) {
//		filename += QLatin1String("-");
//		filename += QString::number(spec.version());
//	}
//
//	filename += QLatin1String(".");
//	filename += format;

	QString filename = "tile_";
	filename += QString::number(spec.y());                                                                                                                  
	filename += QLatin1String("_");
	filename += QString::number(spec.x());
	filename += ".";
	filename += format; // ".png";

	QDir dir = QDir(directory + "/15/" + QString::number(spec.zoom()));

	QString result = dir.filePath(filename);

	qDebug() << "RESULT " << result;

	return result;
}


QGeoTileSpec GPX2VideoGeoFileTileCache::filenameToTileSpec(const QString &filename) const {
	bool ok = false;

	log_call();

	// 15/16/tile_23705_33096.png
	QGeoTileSpec emptySpec;

	QDir dir = QFileInfo(filename).dir();

	int zoom = dir.dirName().toInt(&ok);
	if (!ok)
		return emptySpec;

	// Parse filename
	QStringList parts = QFileInfo(filename).fileName().split('.');

	if (parts.length() != 2)
		return emptySpec;

	QString name = parts.at(0);
	QStringList fields = name.split('_');

	int length = fields.length();
	if (length != 3)
		return emptySpec;

	int y = fields.at(1).toInt(&ok);
	if (!ok)
		return emptySpec;

	int x = fields.at(2).toInt(&ok);
	if (!ok)
		return emptySpec;

	// (const QString &plugin, int mapId, int zoom, int x, int y, int version = -1);
	return QGeoTileSpec("GPX2Video_100",
                      0,
                      zoom,
                      x,
                      y);
}

QT_END_NAMESPACE

