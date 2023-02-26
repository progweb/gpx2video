#include <QtLocation/QGeoServiceProvider>
#include <QtLocation/private/qgeotiledmappingmanagerengine_p.h>

#include "GPX2VideoGeoTileProvider.h"


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

protected:
	void updateMapTypes();

private:
	QVector<GPX2VideoGeoTileProvider *> providers_;
};

QT_END_NAMESPACE

