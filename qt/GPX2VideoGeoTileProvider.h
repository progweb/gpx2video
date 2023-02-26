#ifndef __GPX2VIDEOGEOTILEPROVIDER_H__
#define __GPX2VIDEOGEOTILEPROVIDER_H__

#include <QtCore/QObject>
#include <QtLocation/private/qgeomaptype_p.h>
#include <QtLocation/private/qgeocameracapabilities_p.h>

#include "../src/mapsettings.h"


QT_BEGIN_NAMESPACE

class GPX2VideoGeoTileProvider : public QObject {
	Q_OBJECT

public:
	GPX2VideoGeoTileProvider(
			MapSettings::Source source, 
			const QGeoMapType &map_type,
			const QGeoCameraCapabilities &camera_capabilities);
	~GPX2VideoGeoTileProvider();

	const MapSettings::Source &source() const;
	int minimumZoomLevel() const;
	int maximumZoomLevel() const;
	QString getRepoURI() const;

	const QGeoMapType &mapType() const;
	QGeoCameraCapabilities cameraCapabilities() const;

protected:
	MapSettings::Source source_;
	QGeoMapType map_type_;
	QGeoCameraCapabilities camera_capabilities_;
};

QT_END_NAMESPACE

#endif

