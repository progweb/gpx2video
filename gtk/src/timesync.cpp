#include <iostream>
#include <memory>

#include "log.h"
#include "timesync.h"


GPX2VideoTimeSync::GPX2VideoTimeSync(GPXApplication &app, const ExtractorSettings &settings)
	: TimeSync(app, settings)
	, dispatcher_() {
	log_call();

	app.insert(this);
}


GPX2VideoTimeSync::~GPX2VideoTimeSync() {
	log_call();
}


GPX2VideoTimeSync * GPX2VideoTimeSync::create(GPXApplication &app, MediaContainer *container) {
	log_call();

	ExtractorSettings extractorSettings;
	extractorSettings.setFormat(ExtractorSettings::FormatNone);

	GPX2VideoTimeSync *timesync = new GPX2VideoTimeSync(app, extractorSettings);

	timesync->init(container);

	return timesync;
}


void GPX2VideoTimeSync::exec(void) {
	log_call();

	go();
}


bool GPX2VideoTimeSync::start(void) {
	log_call();

	return TimeSync::start();
}


bool GPX2VideoTimeSync::stop(void) {
	log_call();

	bool result = TimeSync::stop();

	// Broadcast event
	dispatcher_.emit();

	return result;
}
