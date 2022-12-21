#include <QtLocation/QGeoServiceProvider>
#include <QtLocation/private/qgeotiledmappingmanagerengine_p.h>


QT_BEGIN_NAMESPACE

class GPX2VideoGeoTiledMappingManagerEngine : public QGeoTiledMappingManagerEngine {
	Q_OBJECT

public:
	GPX2VideoGeoTiledMappingManagerEngine(
		const QVariantMap &parameters,
		QGeoServiceProvider::Error *error,
		QString *errorString);
	~GPX2VideoGeoTiledMappingManagerEngine();

	QGeoMap *createMap() override;
};

QT_END_NAMESPACE

