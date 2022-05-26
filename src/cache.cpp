#include "log.h"
#include "utils.h"
#include "cache.h"


Cache::Cache(GPX2Video &app) 
	: Task(app) 
	, app_(app) {
}


Cache::~Cache() {
}


Cache * Cache::create(GPX2Video &app) {
	Cache *cache = new Cache(app);

	cache->init();

	return cache;
}


void Cache::init(void) {
	log_call();

	// Create gpx2video cache directories
	path_ = std::getenv("HOME") + std::string("/.gpx2video/cache");
	::mkpath(path_, 0700);
}


bool Cache::run(void) {
	log_call();

	log_notice("Cache initialization...");

	if (app_.command() != GPX2Video::CommandClear)
		goto done;

	rmpath(path_);

done:
	complete();

	return true;
}

