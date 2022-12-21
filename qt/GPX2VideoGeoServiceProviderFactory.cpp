#include "GPX2VideoGeoTiledMappingManagerEngine.h"
#include "GPX2VideoGeoServiceProviderFactory.h"
#include "log.h"


#ifdef GPX2VideoPlugin_Static
Q_EXTERN_C Q_DECL_EXPORT const char *qt_plugin_query_metadata();
Q_EXTERN_C Q_DECL_EXPORT QT_PREPEND_NAMESPACE(QObject) *qt_plugin_instance();

const QT_PREPEND_NAMESPACE(QStaticPlugin) qt_static_plugin_GPX2VideoPlugin() {
	QT_PREPEND_NAMESPACE(QStaticPlugin) plugin = { qt_plugin_instance, qt_plugin_query_metadata};

	return plugin;
}
#endif


QT_USE_NAMESPACE


GPX2VideoGeoServiceProviderFactory::GPX2VideoGeoServiceProviderFactory() {
	log_call();
}


QGeoMappingManagerEngine * GPX2VideoGeoServiceProviderFactory::createMappingManagerEngine(
		const QVariantMap &parameters,
		QGeoServiceProvider::Error *error,
		QString *errorString) const {
	log_call();

	return new GPX2VideoGeoTiledMappingManagerEngine(parameters, error, errorString);
}

