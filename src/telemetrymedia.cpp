#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>

#include <time.h>

#include <GeographicLib/Geodesic.hpp>

#include "log.h"
#include "utils.h"
#include "datetime.h"
#include "telemetry/csv.h"
#include "telemetry/gpx.h"
#include "telemetry.h"



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
		acceleration_ = 0.0;
	
		is_pause_ = false;
		has_value_ = TelemetryData::DataNone;
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
}


void TelemetryData::dump(void) {
	printf("  [%d] '%s' Time: %s [%f, %f] Distance: %.3f km in %d seconds, current speed is %.3f (pause: %s) - Altitude: %.1f (%.1f%%)\n",
			line_,
			type2string(),
			::timestamp2string(ts_).c_str(),
			lon_, lat_,
			distance_/1000.0, (int) round(duration_), speed_, 
			is_pause_ ? "true": "false",
			ele_, grade_);
}


void TelemetryData::writeHeader(void) {
	printf("      | Line  | T | Datetime (ms)           | Distance    | Duration | Speed  | Acceleration | S | Latitude     | Longitude    | Altitude | Grade\n");
	printf("------+-------+---+-------------------------+-------------+----------+--------+--------------+---+--------------+--------------+----------+--------\n");
}


void TelemetryData::writeData(size_t index) const {
	printf("%5ld | %5d | %s | %s | %11.3f | %8d | %6.1f | %12.8f | %1s | %12.8f | %12.8f | %8.1f | %5.1f%%\n",
		index,
		line_,
		type2string(),
		::timestamp2string(ts_).c_str(),
		distance_/1000.0, (int) round(duration_), speed_, acceleration_ / 9.81,
		is_pause_ ? "S" : " ",
		lat_, lon_,
		ele_, grade_);  
}




TelemetrySource::TelemetrySource(const std::string &filename) 
	: eof_(false)
	, offset_(0) 
	, begin_(0)
	, end_(0)
	, from_(0)
	, to_(0) {
	log_call();

//	pool_.setNumberOfPoints(100);

    stream_ = std::ifstream(filename);

	skipBadPoint(false);
	setMethod(TelemetrySettings::MethodNone);
	setSmoothPoints(0);

	kalman_ = alloc_filter_velocity2d(10.0);
}


TelemetrySource::~TelemetrySource() {
}


void TelemetrySource::setNumberOfPoints(const unsigned long number) {
	log_call();

	pool_.setNumberOfPoints(number);
}


void TelemetrySource::skipBadPoint(bool check) {
	log_call();

	check_ = check;
}


void TelemetrySource::setFilter(enum TelemetrySettings::Filter filter) {
	log_call();

	filter_ = filter;
}


void TelemetrySource::setMethod(enum TelemetrySettings::Method method) {
	log_call();

	method_ = method;
}


void TelemetrySource::setSmoothPoints(int number) {
	log_call();

	smooth_points_ = number;
}


bool TelemetrySource::setDataRange(std::string begin, std::string end) {
	log_call();

	if (!begin.empty()) {
		// Convert begin range time to timestamp
		if ((begin_ = ::string2timestamp(begin)) == 0) {
			log_error("Parse 'begin' date range failure");
			return false;
		}
	}

	if (!end.empty()) {
		// Convert end range time to timestamp
		if ((end_ = ::string2timestamp(end)) == 0) {
			log_error("Parse 'end' date range failure");
			return false;
		}
	}

	return true;
}


bool TelemetrySource::setComputeRange(std::string from, std::string to) {
	log_call();

	if (!from.empty()) {
		// Convert race start time to timestamp
		if ((from_ = ::string2timestamp(from)) == 0) {
			log_error("Parse 'from' date range failure");
			return false;
		}
	}

	if (!to.empty()) {
		// Convert race start time to timestamp
		if ((to_ = ::string2timestamp(to)) == 0) {
			log_error("Parse 'to' date range failure");
			return false;
		}
	}

	return true;
}


int64_t TelemetrySource::timeOffset(void) const {
	return offset_;
}


void TelemetrySource::setTimeOffset(const int64_t& offset) {
	offset_ = offset;
}


void TelemetrySource::insert(TelemetrySource::Point &point) {
	// Insert point
	pool_.insert(point);

	// Move to inserted point
	pool_.seek(1);
}


void TelemetrySource::push(TelemetrySource::Point &point) {
	// Append point
	pool_.push(point);

	// Move to next point
	pool_.seek(1);
}


void TelemetrySource::compute_i(TelemetryData &data, bool force) {
	bool enable = true;

	double dc, dt, dz;

	double duration = 0;
	double distance = 0;
	double ridetime = 0;
	double elapsedtime = 0;
	double speed = 0;
	double maxspeed = 0;

	GeographicLib::Math::real lat1;
	GeographicLib::Math::real lon1;
	GeographicLib::Math::real lat2;
	GeographicLib::Math::real lon2;
	GeographicLib::Math::real d;

	GeographicLib::Geodesic gsic(6378137.0, 1.0/298.2572);

	TelemetrySource::Point prevPoint, curPoint;

	log_call();

	if (pool_.backlog() > 0) {
		curPoint = pool_.current();
		prevPoint = pool_.previous();

		if (prevPoint.type() == TelemetryData::TypeUnknown)
			return;

		// Compute data range
		if ((from_ != 0) && (curPoint.timestamp() < from_))
			enable = false;

		if ((to_ != 0) && (curPoint.timestamp() > to_))
			enable = false;

		// Maths
		lat1 = curPoint.lat_;
		lon1 = curPoint.lon_;
		lat2 = prevPoint.lat_;
		lon2 = prevPoint.lon_;

		gsic.Inverse(lat1, lon1, lat2, lon2, d);

		dc = d;
		dt = curPoint.ts_ - prevPoint.ts_;
		dz = (curPoint.ele_ - prevPoint.ele_);

		// Distance
		if (force || !curPoint.hasValue(TelemetryData::DataDistance)) {
			distance = prevPoint.distance_;

			if (enable)
				distance += dc;
		}
		else
			distance = curPoint.distance();

		// Elapsed time
		if (force || !curPoint.hasValue(TelemetryData::DataElapsedTime)) {
			elapsedtime = prevPoint.elapsedtime_ + (dt / 1000.0);
		}
		else
			elapsedtime = curPoint.elapsedTime();

		// Duration in seconds
		if (force || !curPoint.hasValue(TelemetryData::DataDuration)) {
			duration = prevPoint.duration_;

			if (enable)
				duration += (dt / 1000.0);
		}
		else
			duration = curPoint.duration();

		// Ride time
		if (force || !curPoint.hasValue(TelemetryData::DataRideTime))
			ridetime = prevPoint.ridetime_;
		else
			ridetime = curPoint.rideTime();

		// Speed
		if (force || !curPoint.hasValue(TelemetryData::DataSpeed))
			speed = prevPoint.speed_;
		else
			speed = curPoint.speed();

		// MaxSpeed
		if (force || !curPoint.hasValue(TelemetryData::DataMaxSpeed))
			maxspeed = prevPoint.maxspeed_;
		else
			maxspeed = curPoint.maxspeed();

		// Grade elevation
		if (curPoint.hasValue(TelemetryData::DataElevation)) {
			if (force || !curPoint.hasValue(TelemetryData::DataGrade)) {
				if (floor(dc) > 4)
					curPoint.setGrade(100.0 * dz / dc);
				else
					curPoint.setGrade(prevPoint.grade_);
			}
		}

		// Speed & maxspeed & ridetime
		if (dt > 0) {
			// Compute speed (km/h)
			if (force || !curPoint.hasValue(TelemetryData::DataSpeed))
				speed = (3600.0 * dc) / (1.0 * dt);

			// Compute acceleration (m/s²)
			curPoint.setAcceleration(1000 * (speed - prevPoint.speed_) / dt);

			if (enable) {
				if (force || !curPoint.hasValue(TelemetryData::DataMaxSpeed)) {
					if (speed > prevPoint.maxspeed_)
						maxspeed = speed;
				}

				if (speed >= 4.0) {
					if (force || !curPoint.hasValue(TelemetryData::DataRideTime))
						ridetime = prevPoint.ridetime_ + (dt / 1000.0);
				}
			}
		}

		// Update telemetry data
		//-----------------------

		// Distance
		curPoint.setDistance(distance);

		// Duration, elasped time, ride itme
		curPoint.setDuration(duration);
		curPoint.setElapsedTime(elapsedtime);
		curPoint.setRideTime(ridetime);

		// speed, max speed
		curPoint.setSpeed(speed);
		curPoint.setMaxSpeed(maxspeed);

		// average speed
		if ((duration > 0) && (force || !curPoint.hasValue(TelemetryData::DataAverageSpeed)))
			curPoint.setAverageSpeed((3600.0 * distance) / (1000.0 * duration));

		// average ride speed
		if ((ridetime > 0) && (force || !curPoint.hasValue(TelemetryData::DataAverageRideSpeed)))
			curPoint.setAverageRideSpeed((3600.0 * distance) / (1000.0 * ridetime));

		// Update telemetry data (between from & to)
		//-------------------------------------------
		if (enable) {
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

		pool_.current() = curPoint;
	}
	else {
		curPoint = pool_.current();
	}

	data = curPoint;
}


void TelemetrySource::clear(void) {
	log_call();

	pool_.clear();
}


bool TelemetrySource::load(void) {
	size_t count = 0;

	TelemetryData data;

	TelemetrySource::Point point;
	TelemetrySource::Point previous;

	enum TelemetrySource::Data type = TelemetrySource::DataUnknown;

	log_call();

	for (;;) {
		// Read next point
		point.setType(TelemetryData::TypeMeasured);

		type = read(point);

		if (type == TelemetrySource::DataEof)
			break;

		// Apply timestamp offset
		point.setTimestamp(point.timestamp() + offset_);

		// With garmin devices, some points are same whereas timestamp is different!
		if (check_ && (previous.raw_lat_ == point.raw_lat_) && (previous.raw_lon_ == point.raw_lon_)) {
			point.setType(TelemetryData::TypeError);
			count++;
		}

		// Append to pool & pre-compute data
		push(point);
		compute_i(data);

		// Save last point
		previous = point;
	}

	pool_.seek(1);
	compute_i(data);

	pool_.reset();

	if (count > 0)
		log_notice("%s: %lu skip points", name().c_str(), count);

	return true;
}


void TelemetrySource::filter(void) {
	double m;
	double M;

	size_t n = 0;

	std::deque<double> acc;

	log_call();

	// Filter method
	switch (filter_) {
//	case 1:
//		log_info("%s: Filter telemetry data in using '1'.", name().c_str());
//		break;
//
//	case 2:
//		log_info("%s: Filter telemetry data in using '2'.", name().c_str());
//		break;
//
//	case 3:
//		log_info("%s: Filter telemetry data in using '3'.", name().c_str());
//		break;

	case 1:
		log_info("%s: Filter telemetry data in using 'Iglewicz & Hoaglin' method.", name().c_str());

		// Build acceleration sorted list
		for (size_t i=0; i<=pool_.size(); i++)
			acc.emplace_back(pool_[i].acceleration());

		std::sort(acc.begin(), acc.end());

		// Compute median value
		m = acc[acc.size() / 2];

		// acc[i] = | acc[i] - m |
		for (size_t i=0; i<acc.size(); i++)
			acc[i] = fabs(acc[i] - m);

		std::sort(acc.begin(), acc.end());

		// Compute median
		M = acc[acc.size() / 2];

		// Tag outliers
		for (size_t i=0; i<=pool_.size(); i++) {
			if (fabs((0.6745 * (pool_[i].acceleration() - m)) / M) > 5.0) {
				pool_[i].setType(TelemetryData::TypeError);
				n++;
			}
		}

		log_notice("%s: %lu outliers detected", name().c_str(), n);
		break;

	default:
		// No filter
		break;
	}
}


void TelemetrySource::compute(void) {
	int ss = -1;
	int done = 0;

	double ele = 0.0;
	double grade = 0.0;

	uint64_t interval = 4000; // 4s

	uint64_t timestamp = -1;

	TelemetryData data;

	while (pool_.size() > 0) {
		pool_.seek(1);

		// Skip incoherent point
		if (pool_.current().type() == TelemetryData::TypeError)
			continue;

		compute_i(data, true);

		// > 2.8 m/us => 0.40 m/us 
		// < 2.8 m/us => 0.15 m/us
		//
		// * 3600 => km/h
		if (data.speed() > 1.5) {
			ss = -1;
			done = 0;
			timestamp = data.timestamp();
			grade = pool_.current().grade();
		}
		else {
			ss++;

			ele = pool_.current().elevation();
		}

		// Pause detection
		if ((ss >= 0) && (data.timestamp() > (timestamp + interval))) {
			// pause detected
			pool_.current().setPause(true);
			pool_.current().setSpeed(0);
			pool_.current().setAcceleration(0);
			pool_.current().setElevation(ele);
			pool_.current().setGrade(grade);

			for (int k=0; k<=ss-done; k++) {
				pool_.previous(k).setPause(true);
				pool_.previous(k).setSpeed(0);
				pool_.previous(k).setAcceleration(0);
				pool_.previous(k).setElevation(ele);
				pool_.previous(k).setGrade(grade);
			}

			done = ss;
		}
	}

	pool_.reset();
}


void TelemetrySource::smooth(void) {
	int count = 0;
	int window = (smooth_points_ * 2) + 1;

	double elevation = 0;
	double grade = 0;
	double speed = 0;
	double maxspeed = 0;

	std::deque<Point> points;

	TelemetrySource::Point nextPoint, previousPoint;

	if (window < 2)
		return;

	log_info("%s: Smooth telemetry data on window size '%d' (+/- %d points)", 
			name().c_str(), window, smooth_points_);

	// Fill window
	for (int i=0; i<window/2; i++) {
		nextPoint = (i == 0) ? pool_.current() : pool_.next(i);

		if (nextPoint.type() == TelemetryData::TypeError)
			break;

		// Save point
		points.emplace_back(nextPoint);

		elevation += nextPoint.elevation();
		grade += nextPoint.grade();
		speed += nextPoint.speed();
		count += 1;
	}

	// Compute average value for each point
	while (!pool_.empty()) {
		if (pool_.current().type() != TelemetryData::TypeError) {
			// Add new point
			nextPoint = pool_.next(window/2 - 1);

			if (nextPoint.type() != TelemetryData::TypeUnknown) {
				elevation += nextPoint.elevation();
				grade += nextPoint.grade();
				speed += nextPoint.speed();
				count += 1;
			}

			// Save point
			points.emplace_back(nextPoint);

			// Remove old point
			if (count > window) {
				previousPoint = points.front();
				points.pop_front();

				elevation -= previousPoint.elevation();
				grade -= previousPoint.grade();
				speed -= previousPoint.speed();
				count -= 1;
			}

			// New speed
			if (!pool_.current().isPause()) {
				maxspeed = MAX(maxspeed, speed / count);

				pool_.current().setElevation(elevation / count);
				pool_.current().setGrade(grade / count);
				pool_.current().setSpeed(speed / count);
				pool_.current().setMaxSpeed(maxspeed);
			}
		}

		// Move to next
		pool_.seek(1);
	}

	pool_.reset();
}


void TelemetrySource::fix(void) {
	TelemetrySource::Point currentPoint;
	TelemetrySource::Point prevPoint, nextPoint;

	prevPoint = pool_.current();

	while (!pool_.empty()) {
		pool_.seek(1);

		currentPoint = pool_.current();

		if (currentPoint.type() != TelemetryData::TypeError) {
			prevPoint = currentPoint;
			continue;
		}

		// Search next point
		nextPoint = pool_.next();

		if (nextPoint.type() == TelemetryData::TypeUnknown) {
		}
		// Fix point by interpolation
		else {
			double grade;

			double speed = prevPoint.speed_ + ((int) (currentPoint.ts_ - prevPoint.ts_)) * (nextPoint.speed_ - prevPoint.speed_) / ((int) (nextPoint.ts_ - prevPoint.ts_));
			double distance = prevPoint.distance_ + ((int) (currentPoint.ts_ - prevPoint.ts_)) * (nextPoint.distance_ - prevPoint.distance_) / ((int) (nextPoint.ts_ - prevPoint.ts_));
			double ele = prevPoint.ele_ + ((int) (currentPoint.ts_ - prevPoint.ts_)) * (nextPoint.ele_ - prevPoint.ele_) / ((int) (nextPoint.ts_ - prevPoint.ts_));

			// Compute grade
			if (speed > 0)
				grade = 100.0 * (ele - prevPoint.ele_) / (currentPoint.distance_ - prevPoint.distance_);
			else
				grade = prevPoint.grade();

			currentPoint.setSpeed(speed);
			currentPoint.setDistance(distance);
			currentPoint.setElevation(ele);
			currentPoint.setGrade(grade);

			if (speed == 0)
				currentPoint.setPause(true);

			currentPoint.setType(TelemetryData::TypeFixed);

			// Upgrade grade for next point
			if (!nextPoint.isPause())
				nextPoint.setGrade(grade);

			// Save points
			pool_.next() = nextPoint;
			pool_.current() = currentPoint;
		}
	}

	pool_.reset();
}


void TelemetrySource::trim(void) {
	TelemetrySource::Point point;

	if ((begin_ == 0) && (end_ == 0))
		return;

	while (!pool_.empty()) {
		point = pool_.current();

		// Drop data before 'begin_' timestamp
		if ((begin_ != 0) && (point.timestamp() < begin_)) {
			pool_.remove();
			continue;
		}

		// Drop data after 'end_' timestamp
		if ((end_ != 0) && (point.timestamp() > end_)) {
			pool_.remove();
			continue;
		}

		pool_.seek(1);
	}

	pool_.reset();
}


void TelemetrySource::dump(bool content) {
	std::cout << "Telemetry settings:" << std::endl;
	std::cout << " - offset: " << offset_ << std::endl;
	std::cout << " - begin: " << begin_ << std::endl;
	std::cout << " - end: " << end_ << std::endl;
	std::cout << " - from: " << from_ << std::endl;
	std::cout << " - to: " << to_ << std::endl;

	pool_.dump(content);
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


enum TelemetrySource::Data TelemetrySource::loadData(void) {
	enum TelemetrySource::Data type = TelemetrySource::DataAgain;

	log_call();

	reset();
	clear();
	load();
	filter();
	compute();
	smooth();
	fix();
	trim();

	return type;
}


void TelemetrySource::updateData(TelemetryData &data) {
	log_call();

	if (pool_.empty())
		goto skip;

	data = pool_.current();

skip:
	return;
}


void TelemetrySource::predictData(TelemetryData &data, uint64_t timestamp) {
	uint64_t offset;

	TelemetrySource::Point point;

	TelemetrySource::Point prevPoint, curPoint, nextPoint;

	TelemetrySettings::Method method = method_;

	log_call();

	// Compute timestamp offset for the new predict point
	offset = timestamp - data.timestamp();

	// Predict at timestamp
	point.setType(TelemetryData::TypePredicted);
	point.setTimestamp(timestamp);

	// Can't predict data, so use an other method...
	if (pool_.count() < 2)
		method = TelemetrySettings::MethodNone;

	switch (method) {
	case TelemetrySettings::MethodKalman:
	case TelemetrySettings::MethodInterpolate:
		if (pool_.size() == 0) {
			if (pool_.backlog() > 0)
				method = TelemetrySettings::MethodLinear;
			else
				method = TelemetrySettings::MethodSample;
		}
		break;

	case TelemetrySettings::MethodLinear:
		if (pool_.backlog() == 0) {
			if (pool_.size() > 0)
				method = TelemetrySettings::MethodInterpolate;
			else
				method = TelemetrySettings::MethodSample;
		}
		break;

	default:
		// No change
		break;
	}
			
	// Finally, apply predict method
	switch (method) {
	case TelemetrySettings::MethodKalman:
	case TelemetrySettings::MethodInterpolate:
		double lat, lon;

		prevPoint = pool_.current();
		nextPoint = pool_.next();

		// Restore previous computed data
		point.restore(prevPoint);

		// Compute new position
		if (method == TelemetrySettings::MethodKalman) {
			::update(kalman_);
			::get_lat_long(kalman_, &lat, &lon);
		}
		else {
			lat = prevPoint.lat_ + ((int) (timestamp - prevPoint.ts_)) * (nextPoint.lat_ - prevPoint.lat_) / ((int) (nextPoint.ts_ - prevPoint.ts_));
			lon = prevPoint.lon_ + ((int) (timestamp - prevPoint.ts_)) * (nextPoint.lon_ - prevPoint.lon_) / ((int) (nextPoint.ts_ - prevPoint.ts_));
		}

		point.setPosition(timestamp, lat, lon);

		// Measured data
		if (prevPoint.hasValue(TelemetryData::DataElevation) && nextPoint.hasValue(TelemetryData::DataElevation))
			point.setElevation(prevPoint.ele_ + ((int) (timestamp - prevPoint.ts_)) * (nextPoint.ele_ - prevPoint.ele_) / ((int) (nextPoint.ts_ - prevPoint.ts_)));

		if (prevPoint.hasValue(TelemetryData::DataCadence) && nextPoint.hasValue(TelemetryData::DataCadence))
			point.setCadence(prevPoint.cadence_ + ((int) (timestamp - prevPoint.ts_)) * (nextPoint.cadence_ - prevPoint.cadence_) / ((int) (nextPoint.ts_ - prevPoint.ts_)));

		if (prevPoint.hasValue(TelemetryData::DataHeartrate) && nextPoint.hasValue(TelemetryData::DataHeartrate))
			point.setHeartrate(prevPoint.heartrate_ + ((int) (timestamp - prevPoint.ts_)) * (nextPoint.heartrate_ - prevPoint.heartrate_) / ((int) (nextPoint.ts_ - prevPoint.ts_)));

		if (prevPoint.hasValue(TelemetryData::DataTemperature) && nextPoint.hasValue(TelemetryData::DataTemperature))
			point.setTemperature(prevPoint.temperature_ + ((int) (timestamp - prevPoint.ts_)) * (nextPoint.temperature_ - prevPoint.temperature_) / ((int) (nextPoint.ts_ - prevPoint.ts_)));

		if (prevPoint.hasValue(TelemetryData::DataPower) && nextPoint.hasValue(TelemetryData::DataPower))
			point.setPower(prevPoint.power_ + ((int) (timestamp - prevPoint.ts_)) * (nextPoint.power_ - prevPoint.power_) / ((int) (nextPoint.ts_ - prevPoint.ts_)));

		// Computed data
		if (prevPoint.hasValue(TelemetryData::DataSpeed) && nextPoint.hasValue(TelemetryData::DataSpeed))
			point.setSpeed(prevPoint.speed_ + ((int) (timestamp - prevPoint.ts_)) * (nextPoint.speed_ - prevPoint.speed_) / ((int) (nextPoint.ts_ - prevPoint.ts_)));

		if (prevPoint.hasValue(TelemetryData::DataDistance) && nextPoint.hasValue(TelemetryData::DataDistance))
			point.setDistance(prevPoint.distance_ + ((int) (timestamp - prevPoint.ts_)) * (nextPoint.distance_ - prevPoint.distance_) / ((int) (nextPoint.ts_ - prevPoint.ts_)));

		if (prevPoint.hasValue(TelemetryData::DataGrade) && nextPoint.hasValue(TelemetryData::DataGrade))
			point.setGrade(prevPoint.grade_ + ((int) (timestamp - prevPoint.ts_)) * (nextPoint.grade_ - prevPoint.grade_) / ((int) (nextPoint.ts_ - prevPoint.ts_)));

		if (prevPoint.hasValue(TelemetryData::DataAverageSpeed) && nextPoint.hasValue(TelemetryData::DataAverageSpeed))
			point.setAverageSpeed(prevPoint.avgspeed_ + ((int) (timestamp - prevPoint.ts_)) * (nextPoint.avgspeed_ - prevPoint.avgspeed_) / ((int) (nextPoint.ts_ - prevPoint.ts_)));

		if (prevPoint.hasValue(TelemetryData::DataAverageRideSpeed) && nextPoint.hasValue(TelemetryData::DataAverageRideSpeed))
			point.setAverageRideSpeed(prevPoint.avgridespeed_ + ((int) (timestamp - prevPoint.ts_)) * (nextPoint.avgridespeed_ - prevPoint.avgridespeed_) / ((int) (nextPoint.ts_ - prevPoint.ts_)));
		break;

	case TelemetrySettings::MethodLinear:
		prevPoint = pool_.previous();
		curPoint = pool_.current();

		// Restore previous computed data
		point.restore(curPoint);

		// Compute new position
		point.setPosition(
			timestamp,
			data.lat_ + (curPoint.lat_ - prevPoint.lat_),
			data.lon_ + (curPoint.lon_ - prevPoint.lon_)
		);

		// Measured data
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

		// Computed data
		// TODO
		//
		break;

	case TelemetrySettings::MethodSample:
		// Create a new point (just copy previous & update timestamp)
		point = pool_.current();

		point.setType(TelemetryData::TypePredicted);

		point.setTimestamp(timestamp);
		break;

	case TelemetrySettings::MethodNone:
	default:
		data.type_ = TelemetryData::TypeUnchanged;

		data.ts_ = timestamp;
		break;
	}

	if (method_ != TelemetrySettings::MethodNone) {
		// Update common data
		//--------------------

		if (data.hasValue(TelemetryData::DataElapsedTime))
			point.setElapsedTime(data.elapsedTime() + (offset / 1000.0));
		// TODO between from & to
		if (data.hasValue(TelemetryData::DataDuration))
			point.setDuration(data.duration() + (offset / 1000.0));
		// TODO between from & to
		if (data.hasValue(TelemetryData::DataRideTime) && (data.speed() > 0))
			point.setRideTime(data.rideTime() + (offset / 1000.0));

		// TODO from & to
		if (data.hasValue(TelemetryData::DataMaxSpeed))
			point.setMaxSpeed(MAX(data.maxspeed(), point.speed()));

		// Return predict point
		data = point;
	}
}


enum TelemetrySource::Data TelemetrySource::retrieveData(TelemetryData &data) {
	enum TelemetrySource::Data type = TelemetrySource::DataAgain;

	log_call();

	if (pool_.size() == 0) {
		type = TelemetrySource::DataEof;
		goto done;
	}

	pool_.seek(1);

	updateData(data);

done:
	return type;
}


enum TelemetrySource::Data TelemetrySource::retrieveFirst(TelemetryData &data) {
	log_call();

	pool_.reset();

	data = pool_.first();

	return (data.type() != TelemetryData::TypeUnknown) ? TelemetrySource::DataAgain : TelemetrySource::DataEof;
}


enum TelemetrySource::Data TelemetrySource::retrieveFrom(TelemetryData &data) {
	enum TelemetrySource::Data result;

	result = retrieveFirst(data);

	if (from_ != 0) {
		result = retrieveNext(data, from_);

//		start_ = data;
//
//		data.reset();
	}

	return result;
}


enum TelemetrySource::Data TelemetrySource::retrieveNext(TelemetryData &data, uint64_t timestamp) {
	TelemetrySource::Point nextPoint;

	enum TelemetrySource::Data type = TelemetrySource::DataUnknown;

	log_call();

	// Last point
	nextPoint = pool_.next();

//	printf(" <ts %lu> ", timestamp);

	// Read next points if need
	do {
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
//				printf(" <predict %lu %lu> ", timestamp, nextPoint.timestamp());
				predictData(data, timestamp);
			}
			else {
//				printf(" <next> ");

				if (this->retrieveData(data) == TelemetrySource::DataEof)
					goto eof;
	
				nextPoint = pool_.next();
			}
		}
	} while ((timestamp != (uint64_t) -1) && (data.timestamp() < timestamp));

//	printf("\n");

	return type;

eof:
	return TelemetrySource::Data::DataEof;
}


enum TelemetrySource::Data TelemetrySource::retrieveLast(TelemetryData &data) {
	log_call();

	data = pool_.last();

	return (data.type() != TelemetryData::TypeUnknown) ? TelemetrySource::DataAgain : TelemetrySource::DataEof;
}


TelemetrySource * TelemetryMedia::open(const std::string &filename, const TelemetrySettings &settings) {
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
		// Timestamp offset
		source->setTimeOffset(settings.telemetryOffset());

		// Telemetry range
		source->setDataRange(settings.telemetryBegin(), settings.telemetryEnd());
		source->setComputeRange(settings.telemetryFrom(), settings.telemetryTo());

		// Telemetry data filter
		source->skipBadPoint(settings.telemetryCheck());
		source->setFilter(settings.telemetryFilter());
		source->setMethod(settings.telemetryMethod());
		source->setSmoothPoints(settings.telemetrySmoothPoints());

		// Load telemtry data
		source->loadData();
	}

	return source;
}



