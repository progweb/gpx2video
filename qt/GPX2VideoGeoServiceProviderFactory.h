#include <QtCore/QObject>
#include <QtLocation/qgeoserviceproviderfactory.h>


QT_BEGIN_NAMESPACE

class GPX2VideoGeoServiceProviderFactory : public QObject, public QGeoServiceProviderFactoryV2 
{
	Q_OBJECT
	Q_INTERFACES(QGeoServiceProviderFactoryV2)
	Q_PLUGIN_METADATA(IID "org.qt-project.qt.geoservice.serviceproviderfactory/5.0"
		FILE "gpx2video_plugin.json")

public:
	explicit GPX2VideoGeoServiceProviderFactory();

	QGeoMappingManagerEngine *createMappingManagerEngine(
		const QVariantMap &parameters,
		QGeoServiceProvider::Error *error,
		QString *errorString) const;

};

QT_END_NAMESPACE

