#include <string>
#include <iomanip>

#include "log.h"
#include "telemetry.h"


// Telemetry settings
//--------------------

TelemetrySettings::TelemetrySettings() {
}


TelemetrySettings::~TelemetrySettings() {
}


const TelemetrySettings::Filter& TelemetrySettings::filter(void) const {
	return filter_;
}


void TelemetrySettings::setFilter(const TelemetrySettings::Filter &filter) {
	filter_ = filter;
}


const std::string TelemetrySettings::getFriendlyName(const TelemetrySettings::Filter &filter) {
	switch (filter) {
	case FilterNone:
		return "None";
	case FilterKalman:
		return "Apply Kalman filter on GPX data (lat., lon. and ele.)";
	case FilterLinear:
		return "Apply a simple linear filter on GPX data";
	case FilterInterpolate:
		return "Interpolate GPX data filter";
	case FilterCount:
	default:
		return "";
	}

	return "";
}



// Telemetry API
//---------------

Telemetry::Telemetry(GPX2Video &app) 
	: Task(app) 
	, app_(app) {
}


Telemetry::~Telemetry() {
}


Telemetry * Telemetry::create(GPX2Video &app) {
	Telemetry *telemetry = new Telemetry(app);

	telemetry->init();

	return telemetry;
}


void Telemetry::init(void) {
	log_call();

	gpx_ = GPX::open(app_.settings().gpxfile(), app_.settings().telemetryFilter());
}


bool Telemetry::start(void) {
	bool result = true;

	std::string filename = app_.settings().outputfile();

	log_call();

	// Start time activity
	gpx_->retrieveFirst(data_);
	gpx_->setStartTime(data_.time());

	// Open output stream
    out_ = std::ofstream(filename);
       
	if (!out_.is_open()) {
		log_error("Open '%s' failure", filename.c_str());
		result = false;
		goto done;
	}

	// Header
	out_ << "# Timestamp, Time, Duration, Data, Lat, Lon, Ele, Distance, Speed, MaxSpeed, Average" << std::endl;

	// Read GPX from start
	timecode_ms_ = 0;

done:
	return result;
}


bool Telemetry::run(void) {
	// Read GPX data each 1 second
	struct tm tm;

	char time[128];

	enum GPX::Data type;

	if (app_.settings().telemetryFilter() == TelemetrySettings::FilterNone) 
		timecode_ms_ = -1;

	if ((type = gpx_->retrieveNext(data_, timecode_ms_)) == GPX::DataEof) {
		goto done;
	}

	localtime_r(&data_.time(), &tm);
	strftime(time, sizeof(time), "%Y-%m-%d %H:%M:%S", &tm);

	out_ << std::setprecision(8);
	out_ << data_.time();
	out_ << ", \"" << time << "\"";
	out_ << ", " << data_.elapsedTime();
	out_ << ", " << ((type == GPX::DataPredicted) ? "P" : "M");
	out_ << ", " << data_.position().lat;
	out_ << ", " << data_.position().lon;
	out_ << ", " << data_.position().ele;
	out_ << ", " << data_.distance(); 
	out_ << ", " << data_.speed(); 
	out_ << ", " << data_.maxspeed(); 
	out_ << ", " << data_.avgspeed(); 
	out_ << std::endl;

	timecode_ms_ += 1000;

	schedule();

	return true;

done:
	complete();

	return true;
}


bool Telemetry::stop(void) {
	if (out_.is_open())
		out_.close();

	return true;
}

