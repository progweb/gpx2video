#include <QtCore/QObject>
#include <QtCore/QCommandLineParser>
#include <QtQml/QQmlApplicationEngine>
#include <QtQml/QQmlContext>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtLocation/QGeoServiceProvider>
#include <QDebug>

#ifdef GPX2VideoPlugin_Static
#include <QtPlugin>
#endif

#include "log.h"



int main(int argc, char *argv[], char *envp[]) {
#ifdef GPX2VideoPlugin_Static
	Q_IMPORT_PLUGIN(GPX2VideoPlugin);
#endif

	Q_UNUSED(envp);

	// Banner
	QTextStream(stdout) << "gpx2video-qt v0.0.0" << Qt::endl;

	// Application
	QApplication app(argc, argv);

	// Command line options
	QCommandLineParser parser;
	parser.setApplicationDescription("GPX2Video Qt Interface");
	const QCommandLineOption helpOption = parser.addHelpOption();

	const QCommandLineOption verboseOption("v", QCoreApplication::translate("main", "Verbose mode"));
	parser.addOption(verboseOption);

	// Process the actual command line arguments given by the user
	parser.process(app);

	bool help = parser.isSet(helpOption);
	bool verbose = parser.isSet(verboseOption);

	// Debug/Verbose
	gpx2video_log_setup("gpx2video-qt");

	if (verbose) {
		gpx2video_log_quiet(0);
		gpx2video_log_debug_enable(1);
	}

	if (help) {
	}

	// Build UI
	QQmlApplicationEngine engine;
	engine.rootContext()->setContextProperty("applicationDirPath", QGuiApplication::applicationDirPath());

	const QUrl url(QStringLiteral("qrc:/main.qml"));
	QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
		&app, [url](QObject *obj, const QUrl &objUrl) {
			if (!obj && url == objUrl)
				QCoreApplication::exit(-1);
		}, Qt::QueuedConnection);
	engine.load(url);

//	QComboBox *cb = engine.rootContext()->findChild<QComboBox *>(QString("cb"));
//	QObject::connect(&myObj,      SIGNAL(finishedGatheringDataForItem(QString)),
//                 contentView, SLOT(updateViewWithItem(QString)));

	return app.exec();
}


