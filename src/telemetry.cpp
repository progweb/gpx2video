#include <cmath>
#include <string>
#include <iomanip>
#include <algorithm>
#include <filesystem>

#include "log.h"
#include "telemetry.h"
#include "telemetry/gpx.h"


// Telemetry settings
//--------------------

TelemetrySettings::TelemetrySettings(
		bool check,
		TelemetrySettings::Method method,
		int rate,
		TelemetrySettings::Format format)
		: telemetry_begin_("")
		, telemetry_end_("")
		, telemetry_format_(format)
		, telemetry_check_(check)
		, telemetry_filter_(TelemetrySettings::FilterNone)
		, telemetry_method_(method)
		, telemetry_rate_(rate) {
}


TelemetrySettings::~TelemetrySettings() {
}


void TelemetrySettings::setDataRange(const std::string &begin, const std::string &end) {
	telemetry_begin_ = begin;
	telemetry_end_ = end;
}


void TelemetrySettings::setFilter(enum TelemetrySettings::Filter filter) {
	telemetry_filter_ = filter;
}


const std::string& TelemetrySettings::telemetryBegin(void) const {
	return telemetry_begin_;
}


const std::string& TelemetrySettings::telemetryEnd(void) const {
	return telemetry_end_;
}


const TelemetrySettings::Format& TelemetrySettings::telemetryFormat(void) const {
	return telemetry_format_;
}


const bool& TelemetrySettings::telemetryCheck(void) const {
	return telemetry_check_;
}

const TelemetrySettings::Filter& TelemetrySettings::telemetryFilter(void) const {
	return telemetry_filter_;
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


void TelemetrySettings::dump(void) const {
	std::cout << "Telemetry settings: " << std::endl;
	std::cout << "  skip bad point: " << (telemetry_check_ ? "true" : "false") << std::endl;
	std::cout << "  begin data range: " << telemetry_begin_ << std::endl;
	std::cout << "  end data range: " << telemetry_end_ << std::endl;
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

	bool gpx_extension = false;
	enum GPX::Version gpx_version = GPX::V1_1;

	std::string filename = app_.settings().outputfile();

	log_call();

	// Input valid
	if (!source_) {
		result = false;
		goto done;
	}

	// Telemetry data range
	source_->setDataRange(settings().telemetryBegin(), settings().telemetryEnd());

	// Telemetry data filter
	source_->skipBadPoint(settings().telemetryCheck());
	source_->setFilter(settings().telemetryFilter());

	// Telemetry compute range
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
		gpx_extension = data_.hasValue((TelemetryData::Data) (TelemetryData::DataCadence | TelemetryData::DataHeartrate | TelemetryData::DataTemperature));

		out_ << "<?xml version=\"1.0\" encoding=\"utf-8\"?>" << std::endl;
		if (gpx_version == GPX::V1_1)
			out_ << "<gpx version=\"1.1\"" << std::endl;
		else
			out_ << "<gpx version=\"1.0\"" << std::endl;
		out_ << "  creator=\"gpx2video\"" << std::endl;
		out_ << "  xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"" << std::endl;
		if (gpx_extension) {
			out_ << "  xmlns:ns2=\"http://www.garmin.com/xmlschemas/GpxExtensions/v3\"" << std::endl;
			out_ << "  xmlns:ns3=\"http://www.garmin.com/xmlschemas/TrackPointExtension/v1\"" << std::endl;
		}
		if (gpx_version == GPX::V1_1) {
			out_ << "  xmlns=\"http://www.topografix.com/GPX/1/1\"" << std::endl;
			out_ << "  xsi:schemaLocation=\"http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/11.xsd\">" << std::endl;
		}
		else {
			out_ << "  xmlns=\"http://www.topografix.com/GPX/1/0\"" << std::endl;
			out_ << "  xsi:schemaLocation=\"http://www.topografix.com/GPX/1/0 http://www.topografix.com/GPX/1/0/gpx.xsd\">" << std::endl;
		}
		out_ << "  <trk>" << std::endl;
		out_ << "    <number>1</number>" << std::endl;
		out_ << "    <trkseg>" << std::endl;
		break;

	case TelemetrySettings::FormatCSV:
		out_ << "Timestamp, Time, Total duration, Partial duration, RideTime, Data, Lat, Lon, Ele, Grade, Distance, Speed, MaxSpeed, Average, Ride Average, Cadence, Heartrate, Temperature, Power, Lap" << std::endl;
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

	bool gpx_extension = false;

	enum TelemetrySource::Data type;

	log_call();

	rate = settings().telemetryRate();

	// Dump point

	switch (output_format_) {
	case TelemetrySettings::FormatGPX:
		gmtime_r(&data_.time(), &tm);
		strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", &tm);

		snprintf(time, sizeof(time), "%s.%03dZ", buf, (int) (data_.timestamp() % 1000));

		gpx_extension = data_.hasValue((TelemetryData::Data) (TelemetryData::DataCadence | TelemetryData::DataHeartrate | TelemetryData::DataTemperature));

		out_ << std::setprecision(9);
		out_ << "      <trkpt lat=\"" << data_.latitude() << "\" lon=\"" << data_.longitude() << "\">" << std::endl;
		if (data_.hasValue(TelemetryData::DataElevation))
			out_ << "        <ele>" << data_.elevation() << "</ele>" << std::endl;
		out_ << "        <time>" << time << "</time>" << std::endl;
		if (gpx_extension) {
			out_ << "        <extensions>" << std::endl;
			out_ << "          <ns3:TrackPointExtension>" << std::endl;
			if (data_.hasValue(TelemetryData::DataCadence))
				out_ << "            <ns3:cad>" << data_.cadence() << "</ns3:cad>" << std::endl;
			if (data_.hasValue(TelemetryData::DataHeartrate))
				out_ << "            <ns3:hr>" << data_.heartrate() << "</ns3:hr>" << std::endl;
			if (data_.hasValue(TelemetryData::DataTemperature))
				out_ << "            <ns3:atemp>" << data_.temperature() << "</ns3:atemp>" << std::endl;
			out_ << "          </ns3:TrackPointExtension>" << std::endl;
			out_ << "        </extensions>" << std::endl;
		}
		out_ << "      </trkpt>" << std::endl;
		break;

	case TelemetrySettings::FormatCSV:
		localtime_r(&data_.time(), &tm);
		strftime(time, sizeof(time), "%Y-%m-%d %H:%M:%S", &tm);

		out_ << std::setprecision(8);
		out_ << data_.timestamp();
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
		out_ << ", " << data_.temperature(); 
		out_ << ", " << data_.power(); 
		out_ << ", " << data_.lap(); 
		out_ << std::endl;
		break;

	default:
		break;
	};

	// Next point
	if (settings().telemetryMethod() != TelemetrySettings::MethodNone) {
		if (rate != 0)
			timecode_ms_ += rate;
		else
			timecode_ms_ += 1000;
	}
	else
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

