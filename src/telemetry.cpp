#include <cmath>
#include <string>
#include <iomanip>
#include <algorithm>
#include <filesystem>

#include "log.h"
#include "telemetry.h"


// Telemetry settings
//--------------------

TelemetrySettings::TelemetrySettings(
		TelemetrySettings::Method method,
		int rate,
		TelemetrySettings::Format format)
		: telemetry_format_(format)
		, telemetry_method_(method)
		, telemetry_rate_(rate) {
}


TelemetrySettings::~TelemetrySettings() {
}


const TelemetrySettings::Format& TelemetrySettings::telemetryFormat(void) const {
	return telemetry_format_;
}


const TelemetrySettings::Method& TelemetrySettings::telemetryMethod(void) const {
	return telemetry_method_;
}


const int& TelemetrySettings::telemetryRate(void) const {
	return telemetry_rate_;
}


const std::string TelemetrySettings::getFriendlyName(const TelemetrySettings::Method &method) {
	switch (method) {
	case MethodNone:
		return "None";
	case MethodSample:
		return "Get sample each second from telemetry data";
	case MethodLinear:
		return "Apply a simple linear method on telemtry data";
	case MethodInterpolate:
		return "Interpolate telemetry data method";
	case MethodKalman:
		return "Apply Kalman prediction on telemetry data (lat., lon. and ele.)";
	case MethodCount:
	default:
		return "";
	}

	return "";
}



// Telemetry API
//---------------

Telemetry::Telemetry(GPXApplication &app, TelemetrySettings &settings) 
	: Task(app)
	, app_(app) 
	, settings_(settings) {
}


Telemetry::~Telemetry() {
}


Telemetry * Telemetry::create(GPXApplication &app, TelemetrySettings &settings) {
	Telemetry *telemetry = new Telemetry(app, settings);

	telemetry->init();

	return telemetry;
}


void Telemetry::init(void) {
	log_call();

	source_ = TelemetryMedia::open(app_.settings().inputfile(), settings().telemetryMethod());

	output_format_ = settings().telemetryFormat();

	if (settings().telemetryFormat() == TelemetrySettings::FormatAuto) {
		std::string filename = app_.settings().outputfile();
	
		std::string ext = std::filesystem::path(filename).extension();

		std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

		if (ext == ".gpx")
			output_format_ = TelemetrySettings::FormatGPX;
		else if (ext == ".csv")
			output_format_ = TelemetrySettings::FormatCSV;
		else
			output_format_ = TelemetrySettings::FormatCSV;
	}
}


bool Telemetry::start(void) {
	bool result = true;

	std::string filename = app_.settings().outputfile();

	log_call();

	// Telemetry limits
	source_->setFrom(app_.settings().from());
	source_->setTo(app_.settings().to());

	// Start time activity
	source_->retrieveFirst(data_);

	// Open output stream
    out_ = std::ofstream(filename);
       
	if (!out_.is_open()) {
		log_error("Open '%s' failure", filename.c_str());
		result = false;
		goto done;
	}

	// Header
	switch (output_format_) {
	case TelemetrySettings::FormatGPX:
		out_ << "<?xml version=\"1.0\" encoding=\"utf-8\"?>" << std::endl;
		out_ << "<gpx version=\"1.0\"" << std::endl;
		out_ << "  creator=\"gpx2video\"" << std::endl;
		out_ << "  xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"" << std::endl;
		out_ << "  xmlns=\"http://www.topografix.com/GPX/1/0\"" << std::endl;
		out_ << "  xsi:schemaLocation=\"http://www.topografix.com/GPX/1/0 http://www.topografix.com/GPX/1/0/gpx.xsd\">" << std::endl;
		out_ << "  <trk>" << std::endl;
		out_ << "    <number>1</number>" << std::endl;
		out_ << "    <trkseg>" << std::endl;
		break;

	case TelemetrySettings::FormatCSV:
		out_ << "Timestamp, Time, Total duration, Partial duration, RideTime, Data, Lat, Lon, Ele, Grade, Distance, Speed, MaxSpeed, Average, Ride Average, Cadence, Heartrate, Lap" << std::endl;
		break;

	default:
		break;
	}

	// Read telemetry data from start
	timecode_ms_ = data_.time() * 1000;

done:
	return result;
}


bool Telemetry::run(void) {
	// Read telemetry data each 1 second
	int rate;

	struct tm tm;

	char buf[92];
	char time[128];

	enum TelemetrySource::Data type;

	log_call();

	rate = settings().telemetryRate();

	// Dump point

	switch (output_format_) {
	case TelemetrySettings::FormatGPX:
		gmtime_r(&data_.time(), &tm);
		strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", &tm);

		snprintf(time, sizeof(time), "%s.%03dZ", buf, (int) (data_.timestamp() % 1000));

		out_ << std::setprecision(9);
		out_ << "      <trkpt lat=\"" << data_.latitude() << "\" lon=\"" << data_.longitude() << "\">" << std::endl;
		if (data_.hasValue(TelemetryData::DataElevation))
			out_ << "        <ele>" << data_.elevation() << "</ele>" << std::endl;
		out_ << "        <time>" << time << "</time>" << std::endl;
		out_ << "      </trkpt>" << std::endl;
		break;

	case TelemetrySettings::FormatCSV:
		localtime_r(&data_.time(), &tm);
		strftime(time, sizeof(time), "%Y-%m-%d %H:%M:%S", &tm);

		out_ << std::setprecision(8);
		out_ << data_.time();
		out_ << ", \"" << time << "\"";
		out_ << ", " << round(data_.elapsedTime());
		out_ << ", " << round(data_.duration());
		out_ << ", " << round(data_.rideTime());
		out_ << ", " << data_.type2string();
		out_ << ", " << data_.latitude();
		out_ << ", " << data_.longitude();
		out_ << ", " << data_.elevation();
		out_ << ", " << data_.grade(); 
		out_ << ", " << data_.distance(); 
		out_ << ", " << data_.speed(); 
		out_ << ", " << data_.maxspeed(); 
		out_ << ", " << data_.avgspeed(); 
		out_ << ", " << data_.avgridespeed(); 
		out_ << ", " << data_.cadence(); 
		out_ << ", " << data_.heartrate(); 
		out_ << ", " << data_.lap(); 
		out_ << std::endl;
		break;

	default:
		break;
	};

	if (rate > 0)
		timecode_ms_ += 1000 / rate;
	else
		timecode_ms_ += 1000;

	// Next point
	if (settings().telemetryMethod() == TelemetrySettings::MethodNone) 
		timecode_ms_ = -1;

	if ((type = this->get(data_, timecode_ms_)) == TelemetrySource::DataEof) {
		goto done;
	}

	schedule();

	return true;

done:
	complete();

	return true;
}


bool Telemetry::stop(void) {
	switch (output_format_) {
	case TelemetrySettings::FormatGPX:
		out_ << "    </trkseg>" << std::endl;
		out_ << "  </trk>" << std::endl;
		out_ << "</gpx>" << std::endl;
		break;

	case TelemetrySettings::FormatCSV:
	default:
		break;
	}

	if (out_.is_open())
		out_.close();

	return true;
}


enum TelemetrySource::Data Telemetry::get(TelemetryData &data, int64_t timecode_ms) {
	enum TelemetrySource::Data type;

	type = source_->retrieveNext(data, timecode_ms);

	return type;
}

