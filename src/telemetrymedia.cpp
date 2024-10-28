#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>

#include <time.h>

#include <GeographicLib/Geodesic.hpp>

#include "telemetry/csv.h"
#include "telemetry/gpx.h"
#include "telemetry.h"


std::string timestamp2string(uint64_t value) {
	int u;
	time_t t;

	struct tm tm;

	char str[128];
	char time[64];

	t = value / 1000;
	u = value % 1000;
	::localtime_r(&t, &tm);
	::strftime(time, sizeof(time), "%Y-%m-%d %H:%M:%S", &tm);

	::snprintf(str, sizeof(str), "%s.%03d", time, u);

	return std::string(str);
}



TelemetryData::TelemetryData() {
	reset(true);
}


TelemetryData::~TelemetryData() {
}


void TelemetryData::reset(bool all) {
	log_call();

	if (all) {
		line_ = 0;

		type_ = TelemetryData::TypeUnknown;

		ts_ = 0;
		lat_ = 0.0;
		lon_ = 0.0;
		ele_ = 0.0;

		cadence_ = 0;
		heartrate_ = 0;
		temperature_ = 0;
		power_ = 0;

		grade_ = 0.0;
		speed_ = 0.0;
	}

	distance_ = 0.0;
	duration_ = 0.0;
	maxspeed_ = 0.0;
	ridetime_ = 0.0;
	elapsedtime_ = 0.0;
	avgspeed_ = 0.0;
	avgridespeed_ = 0.0;

	lap_ = 1;
	in_lap_ = false;

	has_value_ = TelemetryData::DataNone;
}


void TelemetryData::dump(bool debug) {
	char s[128];

	struct tm tm;

	time_t time = ts_ / 1000;

	localtime_r(&time, &tm);

	strftime(s, sizeof(s), "%Y-%m-%d %H:%M:%S", &tm);

	printf("  [%d] '%s' Time: %s.%d Distance: %.3f km in %d seconds, current speed is %.3f - Altitude: %.0f (%.1f%%)\n",
		line_,
		type2string(),
		s,
		(int) (ts_ % 1000),
		distance_/1000.0, (int) round(duration_), speed_,
		ele_, grade_);  
	
	if (debug) {
		printf("  - lon: %f\n", lon_);
		printf("  - lat: %f\n", lat_);
	}
}




TelemetrySource::TelemetrySource(const std::string &filename) 
	: eof_(false)
	, enable_(false)
	, offset_(0) 
	, begin_(0)
	, end_(0)
	, from_(0)
	, to_(0) {
	log_call();

	setNumberOfPoints(100);

    stream_ = std::ifstream(filename);

	kalman_ = alloc_filter_velocity2d(10.0);
}


TelemetrySource::~TelemetrySource() {
}


void TelemetrySource::setNumberOfPoints(const unsigned long number) {
	log_call();

	nbr_points_max_ = number;
}


void TelemetrySource::setFilter(enum TelemetrySettings::Filter filter) {
	log_call();

	filter_ = filter;
}


void TelemetrySource::setMethod(enum TelemetrySettings::Method method) {
	log_call();

	method_ = method;
}


bool TelemetrySource::setDataRange(std::string begin, std::string end) {
	struct tm time;

	log_call();

	if (!begin.empty()) {
		memset(&time, 0, sizeof(time));

		if (::strptime(begin.c_str(), "%Y-%m-%d %H:%M:%S", &time) == NULL) {
			log_error("Parse begin date range failure");
			return false;
		}

		time.tm_isdst = -1;

		// Convert begin range time in UTC time
		begin_ = timelocal(&time) * 1000;
	}

	if (!end.empty()) {
		memset(&time, 0, sizeof(time));

		if (::strptime(end.c_str(), "%Y-%m-%d %H:%M:%S", &time) == NULL) {
			log_error("Parse end date range failure");
			return false;
		}

		time.tm_isdst = -1;

		// Convert end range time in UTC time
		end_ = timelocal(&time) * 1000;
	}

	return true;
}


bool TelemetrySource::setFrom(std::string from) {
	struct tm time;

	TelemetryData data;

	log_call();

	memset(&time, 0, sizeof(time));

	if (from.empty())
		goto skip;

	if (::strptime(from.c_str(), "%Y-%m-%d %H:%M:%S", &time) == NULL) {
		log_error("Parse begin date failure");
		return false;
	}

	time.tm_isdst = -1;

	// Convert race start time in UTC time
	from_ = timelocal(&time) * 1000;

	// Init start position
	retrieveFrom(data);

skip:
	return true;
}


bool TelemetrySource::setTo(std::string to) {
	struct tm time;

	log_call();

	memset(&time, 0, sizeof(time));

	if (to.empty())
		goto skip;

	if (::strptime(to.c_str(), "%Y-%m-%d %H:%M:%S", &time) == NULL) {
		log_error("Parse end date failure");
		return false;
	}

	time.tm_isdst = -1;

	// Convert race start time in UTC time
	to_ = timelocal(&time) * 1000;

skip:
	return true;
}


int64_t TelemetrySource::timeOffset(void) const {
	return offset_;
}


void TelemetrySource::setTimeOffset(const int64_t& offset) {
	offset_ = offset;
}


void TelemetrySource::push(TelemetrySource::Point &point) {
	points_.emplace_back(point);

	while (points_.size() > nbr_points_max_)
		points_.pop_front();
}


void TelemetrySource::filter(void) {
	size_t i;
	size_t size;

	double lat, lon;

	TelemetrySource::Point pt, pt1, pt2;

	if (points_.empty())
		goto skip;

	size = points_.size();

	switch (filter_) {
	case 1:
		if (size > 2) {
			double a = 6;
			double a2 = a * a;
			double z = 0.7;
			double z2 = z * z;

			double k1 = a + a2/(2*z2);
			double k2 = a2/(4*z2);

			i = size - 1;

			pt = points_.at(i);
			pt1 = points_.at(i - 1);
			pt2 = points_.at(i - 2);

			lat = (pt.latitude() + (pt1.latitude() * k1) - (pt2.latitude() * k2)) / (1 + a + k2);
			lon = (pt.longitude() + (pt1.longitude() * k1) - (pt2.longitude() * k2)) / (1 + a + k2);

			pt.setPosition(lat, lon);

			points_[i] = pt;
		}
		break;

	default:
		// No filter
		break;
	}

skip:
	return;
}


void TelemetrySource::compute(TelemetryData &data) {
	size_t k;

	double dc, dt, dz;

	double duration = 0;
	double distance = 0;
	double ridetime = 0;
	double speed = 0;

	GeographicLib::Math::real lat1;
	GeographicLib::Math::real lon1;
	GeographicLib::Math::real lat2;
	GeographicLib::Math::real lon2;
	GeographicLib::Math::real d;

	GeographicLib::Geodesic gsic(6378137.0, 1.0/298.2572);

	TelemetrySource::Point prevPoint, curPoint;

	log_call();

	// + .... | Pprev | Pcur | Pnext
	// +      |       | Pn-1 |   Pn
	// +-----------------------------
	//   next:   k
	//   cur:    k - 1;
	k = points_.size() - 1;

	if (!eof_ && (k > 1))
		k -= 1;

	if (data.ts_ != 0) {
		curPoint = points_.at(k);
		prevPoint = points_.at(k - 1);

		// Copy computed data from previous point (except if already imported)
		curPoint.restore(prevPoint);

		// Maths
		lat1 = curPoint.lat_;
		lon1 = curPoint.lon_;
		lat2 = prevPoint.lat_;
		lon2 = prevPoint.lon_;

		gsic.Inverse(lat1, lon1, lat2, lon2, d);

		dc = d;
		dt = curPoint.ts_ - prevPoint.ts_;
		dz = (curPoint.ele_ - prevPoint.ele_);

		// Duration in seconds
		if (!curPoint.hasValue(TelemetryData::DataDuration))
			duration = prevPoint.duration_ + (dt / 1000.0);
		else
			duration = curPoint.duration();

		// Distance
		if (!curPoint.hasValue(TelemetryData::DataDistance))
			distance = prevPoint.distance_ + dc;
		else
			distance = curPoint.distance();

		// Grade elevation
		if (curPoint.hasValue(TelemetryData::DataElevation)) {
			if (!curPoint.hasValue(TelemetryData::DataGrade)) {
				if (floor(dc) > 0)
					curPoint.setGrade(100.0 * dz / dc);
			}
		}

		// Speed & maxspeed & ridetime
		if (dt > 0) {
			speed = (3600.0 * dc) / (1.0 * dt);

			if (speed < 0.8)
				speed = 0;

			if (curPoint.hasValue(TelemetryData::DataSpeed))
				speed = curPoint.speed();

			if (abs((int) (speed - prevPoint.speed_)) < 50) {
				curPoint.setSpeed(speed);

				if (enable_) {
					if (speed > prevPoint.maxspeed_)
						curPoint.setMaxSpeed(speed);

					if (speed >= 4.0) {
						if (!curPoint.hasValue(TelemetryData::DataRideTime))
							ridetime = prevPoint.ridetime_ + (dt / 1000.0);
						else
							ridetime = curPoint.rideTime();
						curPoint.setRideTime(ridetime);
					}
				}
			}
		}

		// Update telemetry data
		if (enable_) {
			// Duration
			curPoint.setDuration(duration);

			// Elapsed time
			curPoint.setElapsedTime(prevPoint.elapsedtime_ + (dt / 1000.0));

			// Distance
			curPoint.setDistance(distance);

			// average speed
			if ((duration > 0) && !curPoint.hasValue(TelemetryData::DataAverageSpeed))
				curPoint.setAverageSpeed((3600.0 * distance) / (1000.0 * duration));

			// average ride speed
			if ((ridetime > 0) && !curPoint.hasValue(TelemetryData::DataAverageRideSpeed))
				curPoint.setAverageRideSpeed((3600.0 * distance) / (1000.0 * ridetime));

			// Determine current lap
			lat1 = curPoint.lat_;
			lon1 = curPoint.lon_;
			lat2 = start_.lat_;
			lon2 = start_.lon_;

			gsic.Inverse(lat1, lon1, lat2, lon2, d);

			if (prevPoint.in_lap_ && (d < 20)) {
				curPoint.setLap(prevPoint.lap_ + 1); // lap_++;
				curPoint.in_lap_ = false;
			}
			else if (!prevPoint.in_lap_ && (d > 30)) {
				curPoint.in_lap_ = true;
			}
		}

		points_[k] = curPoint;
	}
	else {
		curPoint = points_.at(k - 1);
		points_[k - 1] = curPoint;
	}

	data = curPoint;
}


void TelemetrySource::update(TelemetryData &data) {
	size_t size;

	log_call();

	if (points_.empty())
		goto skip;

	size = points_.size();

	if (size > 1) {
		if (method_ == TelemetrySettings::MethodKalman) {
			TelemetryData point = points_.back();

			::update_velocity2d(kalman_, point.latitude(), point.longitude(), 1.0);
		}

		// Filter
		filter();
		// Then compute
		compute(data);
	}
	else
		data = points_.front();

skip:
	return;
}


void TelemetrySource::predict(TelemetryData &data) {
	size_t size;

	uint64_t timestamp;

	TelemetrySource::Point point;

	TelemetrySource::Point prevPoint, curPoint, nextPoint;

	TelemetrySettings::Method method = method_;

	log_call();

	if (points_.empty())
		return;

	size = points_.size();

	if (size <= 1) {
		log_error("Predict failure, not enough point!");
		return;
	}

	// Compute timestamp for the new predict point
	timestamp = data.ts_ / 1000;
	timestamp = timestamp * 1000;
	timestamp += 1000;

	if ((method == TelemetrySettings::MethodLinear) && (size == 2))
		method = TelemetrySettings::MethodInterpolate;

	switch (method) {
	case TelemetrySettings::MethodKalman:
	case TelemetrySettings::MethodInterpolate:
		double lat, lon;

		prevPoint = points_.at(size - 2);
		nextPoint = points_.at(size - 1);

		// Create a new point
		point.setType(TelemetryData::TypePredicted);

		// Compute new position
		if (method == TelemetrySettings::MethodKalman) {
			::update(kalman_);
			::get_lat_long(kalman_, &lat, &lon);
		}
		else {
			lat = data.lat_ + (timestamp - prevPoint.ts_) * (nextPoint.lat_ - prevPoint.lat_) / (nextPoint.ts_ - prevPoint.ts_);
			lon = data.lon_ + (timestamp - prevPoint.ts_) * (nextPoint.lon_ - prevPoint.lon_) / (nextPoint.ts_ - prevPoint.ts_);
		}

		point.setPosition(timestamp, lat, lon);

		if (prevPoint.hasValue(TelemetryData::DataElevation) && nextPoint.hasValue(TelemetryData::DataElevation))
			point.setElevation(data.ele_ + (timestamp - prevPoint.ts_) * (nextPoint.ele_ - prevPoint.ele_) / (nextPoint.ts_ - prevPoint.ts_));

		if (prevPoint.hasValue(TelemetryData::DataCadence) && nextPoint.hasValue(TelemetryData::DataCadence))
			point.setCadence(data.cadence_ + (timestamp - prevPoint.ts_) * (nextPoint.cadence_ - prevPoint.cadence_) / (nextPoint.ts_ - prevPoint.ts_));

		if (prevPoint.hasValue(TelemetryData::DataHeartrate) && nextPoint.hasValue(TelemetryData::DataHeartrate))
			point.setHeartrate(data.heartrate_ + (timestamp - prevPoint.ts_) * (nextPoint.heartrate_ - prevPoint.heartrate_) / (nextPoint.ts_ - prevPoint.ts_));

		if (prevPoint.hasValue(TelemetryData::DataTemperature) && nextPoint.hasValue(TelemetryData::DataTemperature))
			point.setTemperature(data.temperature_ + (timestamp - prevPoint.ts_) * (nextPoint.temperature_ - prevPoint.temperature_) / (nextPoint.ts_ - prevPoint.ts_));

		if (prevPoint.hasValue(TelemetryData::DataPower) && nextPoint.hasValue(TelemetryData::DataPower))
			point.setPower(data.power_ + (timestamp - prevPoint.ts_) * (nextPoint.power_ - prevPoint.power_) / (nextPoint.ts_ - prevPoint.ts_));

		break;

	case TelemetrySettings::MethodLinear:
		prevPoint = points_.at(size - 3);
		curPoint = points_.at(size - 2);

		// Create a new point
		point.setType(TelemetryData::TypePredicted);

		// Compute new position
		point.setPosition(
			timestamp,
			data.lat_ + (curPoint.lat_ - prevPoint.lat_),
			data.lon_ + (curPoint.lon_ - prevPoint.lon_)
		);

		if (prevPoint.hasValue(TelemetryData::DataElevation) && curPoint.hasValue(TelemetryData::DataElevation))
			point.setElevation(data.ele_ + (curPoint.ele_ - prevPoint.ele_));

		if (prevPoint.hasValue(TelemetryData::DataCadence) && curPoint.hasValue(TelemetryData::DataCadence))
			point.setCadence(data.cadence_ + (curPoint.cadence_ - prevPoint.cadence_));

		if (prevPoint.hasValue(TelemetryData::DataHeartrate) && curPoint.hasValue(TelemetryData::DataHeartrate))
			point.setHeartrate(data.heartrate_ + (curPoint.heartrate_ - prevPoint.heartrate_));

		if (prevPoint.hasValue(TelemetryData::DataTemperature) && curPoint.hasValue(TelemetryData::DataTemperature))
			point.setTemperature(data.temperature_ + (curPoint.temperature_ - prevPoint.temperature_));

		if (prevPoint.hasValue(TelemetryData::DataPower) && curPoint.hasValue(TelemetryData::DataPower))
			point.setPower(data.power_ + (curPoint.power_ - prevPoint.power_));

		break;

	case TelemetrySettings::MethodSample:
		// Create a new point (just copy previous & update timestamp)
		point = points_.at(size - 2);

		point.setType(TelemetryData::TypePredicted);

		point.ts_ = timestamp;
		break;

	case TelemetrySettings::MethodNone:
	default:
		data.type_ = TelemetryData::TypeUnchanged;

		data.ts_ += 1000;
		if (enable_) {
			data.elapsedtime_ += 1000;
			if (data.speed_ >= 4.0)
				data.ridetime_ += 1000;
		}
		break;
	}

	// Insert new point
	if (method_ != TelemetrySettings::MethodNone) {
		auto pos = points_.end();
		points_.insert(std::prev(pos, 1), point);

		// Filter
		filter();
		// Then compute
		compute(data);
	}
}


bool TelemetrySource::getBoundingBox(TelemetryData *p1, TelemetryData *p2) {
	TelemetryData data;

	enum TelemetrySource::Data type = TelemetrySource::DataUnknown;

	log_call();

	// Get & check each point
	for (type = retrieveFirst(data); type != TelemetrySource::DataEof; type = retrieveNext(data)) {
		if (!data.hasValue(TelemetryData::DataFix))
			continue;

		if ((from_ != 0) && (data.timestamp() < from_))
			continue;

		if (!p1->hasValue(TelemetryData::DataFix))
			*p1 = data;
		if (!p2->hasValue(TelemetryData::DataFix))
			*p2 = data;

		// top-left bounding box
		if (data.lon_ < p1->lon_)
			p1->lon_ = data.lon_;
		if (data.lat_ > p1->lat_)
			p1->lat_ = data.lat_;

		// bottom-right bounding box
		if (data.lon_ > p2->lon_)
			p2->lon_ = data.lon_;
		if (data.lat_ < p2->lat_)
			p2->lat_ = data.lat_;

		if ((to_ != 0) && (data.timestamp() > to_))
			break;
	}

	return (p1->hasValue(TelemetryData::DataFix) && p2->hasValue(TelemetryData::DataFix));
}


enum TelemetrySource::Data TelemetrySource::retrieveData(TelemetryData &data) {
	TelemetrySource::Point point;

	enum TelemetrySource::Data type = TelemetrySource::DataUnknown;

	log_call();

	point.setType(TelemetryData::TypeMeasured);

	type = read(point);

	if ((end_ != 0) && (point.timestamp() > (end_ + offset_))) {
		// Simulate end of gpx data stream
		type = TelemetrySource::DataEof;
		goto eof;
	}

	if (type != TelemetrySource::DataEof)
		push(point);
	else if (eof_) {
		type = TelemetrySource::DataEof;
		goto eof;
	}
	else {
		eof_ = true;
		type = TelemetrySource::DataAgain;
	}

	update(data);

eof:
	return type;
}


enum TelemetrySource::Data TelemetrySource::retrieveFirst_i(TelemetryData &data) {
	enum TelemetrySource::Data type = TelemetrySource::DataUnknown;

	log_call();

	reset();

	eof_ = false;

	points_.clear();

	data = TelemetryData();
	type = retrieveData(data);

	if (from_ == 0)
		start_ = data;

	data = TelemetryData();
	type = retrieveData(data);

	return type;
}


enum TelemetrySource::Data TelemetrySource::retrieveFirst(TelemetryData &data) {
	TelemetrySource::Point point;

	enum TelemetrySource::Data result;

	result = retrieveFirst_i(data);

	if (begin_ != 0) {
		result = retrieveNext(data, begin_ - offset_);

		data.reset();

		while (points_.size() > 2)
			points_.pop_front();

		for (size_t i=0; i<points_.size(); i++) {
			point = points_[i];
			point.reset();
			points_[i] = point;
		}
	}

	return result;
}


enum TelemetrySource::Data TelemetrySource::retrieveFrom(TelemetryData &data) {
	enum TelemetrySource::Data result;

	result = retrieveFirst(data);

	if (from_ != 0) {
		result = retrieveNext(data, from_ - offset_);

		start_ = data;

		data.reset();
	}

	return result;
}


enum TelemetrySource::Data TelemetrySource::retrieveNext(TelemetryData &data, uint64_t timestamp) {
	TelemetrySource::Point nextPoint;

	enum TelemetrySource::Data type = TelemetrySource::DataUnknown;

	log_call();

	// Apply timestamp offset
	if (timestamp != (uint64_t) -1) {
		timestamp += offset_;
//		timestamp = timestamp / 1000;
//		timestamp = timestamp * 1000;
	}

	// Last point
	nextPoint = points_.back();

	// Read next points if need
	do {
		if ((from_ != 0) && (data.timestamp() < from_)) {
//				printf(" <from> ");
			disableCompute();
		}
		else if ((to_ != 0) && (data.timestamp() > to_)) {
//				printf(" <to>");
			disableCompute();
		}

		if (timestamp == (uint64_t) -1) {
			if (this->retrieveData(data) == TelemetrySource::DataEof)
				goto eof;
		}
		else {
			if (timestamp <= data.timestamp()) {
//				printf(" <unchanged> ");
				data.type_ = TelemetryData::TypeUnchanged;
			}
			else if (timestamp < nextPoint.timestamp()) {
//				printf(" <predict> ");
				predict(data);
			}
			else {
//				printf(" <next> ");

				if (this->retrieveData(data) == TelemetrySource::DataEof)
					goto eof;
	
				nextPoint = points_.back();
			}
		}

		if ((from_ == 0) || (from_ < data.timestamp()))
			enableCompute();
	} while ((timestamp != (uint64_t) -1) && (data.timestamp() < timestamp));

	return type;

eof:
	return TelemetrySource::Data::DataEof;
}


enum TelemetrySource::Data TelemetrySource::retrieveLast(TelemetryData &data) {
	TelemetryData save;

	enum TelemetrySource::Data type = TelemetrySource::DataUnknown;

	log_call();

	// Get first point
	type = retrieveFirst(data);

	save = data;

	// Get & check next point
	while (type != TelemetrySource::DataEof) {
		if ((to_ != 0) && (data.timestamp() > to_))
			break;

		save = data;

		type = retrieveNext(data);
	}

	data = save;

	return TelemetrySource::DataEof;
}



TelemetrySource * TelemetryMedia::open(const std::string &filename, TelemetrySettings::Method method) {
	TelemetryData data;

	TelemetrySource *source = NULL;

	std::string ext = std::filesystem::path(filename).extension();

	std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

	if (ext == ".gpx") {
		source = new GPX(filename);
	}
	else if (ext == ".csv") {
		source = new CSV(filename);
	}
	else
		std::cout << "Can't determine '" << filename << "' input file format!" << std::endl;

	// Init
	if (source != NULL) {
		source->setMethod(method);
		source->retrieveFrom(data);
	}

	return source;
}



