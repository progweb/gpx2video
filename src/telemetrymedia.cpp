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
	
		in_range_ = false;
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
	printf("  [%d] '%s' (Fix: %c) Time: %s [%f, %f] Distance: %.3f km in %d seconds, current speed is %.3f (pause: %s) - Altitude: %.1f (%.1f%%)\n",
			line_,
			type2string(),
			hasValue(TelemetryData::DataFix) ? 'Y' : 'N',
			::timestamp2string(ts_).c_str(),
			lon_, lat_,
			distance_/1000.0, (int) round(duration_), speed_, 
			is_pause_ ? "true": "false",
			ele_, grade_);
}


void TelemetryData::writeHeader(void) {
	printf("      | Line  | T | Datetime (ms)           | Distance    | Duration | Speed  | Acceleration | S | Latitude     | Longitude    | Altitude | Grade  \n");
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


double TelemetrySource::Point::distanceTo(const Point &to) {
	double d;

	GeographicLib::Geodesic gsic(6378137.0, 1.0/298.2572);

	// Maths
	gsic.Inverse(lat_, lon_, to.lat_, to.lon_, d);

	return d;
}


TelemetrySource::TelemetrySource(const std::string &filename) 
	: quiet_(true)
	, eof_(false)
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


void TelemetrySource::setQuiet(const bool quiet) {
	log_call();

	quiet_ = quiet;
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


bool TelemetrySource::setDataRange(std::string strbegin, std::string strend) {
	uint64_t end = 0;
	uint64_t begin = 0;

	log_call();

	if (!strbegin.empty()) {
		// Convert begin range time to timestamp
		if ((begin = ::string2timestamp(strbegin)) == 0) {
			log_error("Parse 'begin' date range failure");
			return false;
		}
	}

	if (!strend.empty()) {
		// Convert end range time to timestamp
		if ((end = ::string2timestamp(strend)) == 0) {
			log_error("Parse 'end' date range failure");
			return false;
		}

		if (end < begin) {
			log_error("'begin-end' data range values invalid");
			return false;
		}
	}

	// save
	end_ = end;
	begin_ = begin;

	return true;
}


bool TelemetrySource::setComputeRange(std::string strfrom, std::string strto) {
	uint64_t to = 0;
	uint64_t from = 0;

	log_call();

	if (!strfrom.empty()) {
		// Convert race start time to timestamp
		if ((from = ::string2timestamp(strfrom)) == 0) {
			log_error("Parse 'from' date range failure");
			return false;
		}
	}

	if (!strto.empty()) {
		// Convert race start time to timestamp
		if ((to = ::string2timestamp(strto)) == 0) {
			log_error("Parse 'to' date range failure");
			return false;
		}

		if (to < from) {
			log_error("'[from:to]' compute range values invalid");
			return false;
		}
	}

	// Save
	to_ = to;
	from_ = from;

	return true;
}


int64_t TelemetrySource::timeOffset(void) const {
	return offset_;
}


void TelemetrySource::setTimeOffset(const int64_t& offset) {
	offset_ = offset;
}


bool TelemetrySource::inRange(uint64_t timestamp) const {
	bool in_range = true;

	uint64_t from, to;

	from = !from_ ? begin_ : from_;
	to = !to_ ? end_ : to_;

	if (from && (timestamp <= from))
		in_range = false;
	if (to && (timestamp > to))
		in_range = false;

	return in_range;
}


uint64_t TelemetrySource::beginTimestamp(void) const {
	return begin_;
}


uint64_t TelemetrySource::endTimestamp(void) const {
	return end_;
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
	double acceleration = 0;

	GeographicLib::Math::real d;

	TelemetrySource::Point prevPoint, curPoint;

	log_call();

	// Compute data for current point
	curPoint = pool_.current();

	// Compute data range
	enable = inRange(curPoint.timestamp());

	if (pool_.backlog() > 0) {
		// Retrieve previous point
		prevPoint = pool_.previous();

		if (prevPoint.type() == TelemetryData::TypeUnknown)
			return;

		// Maths
		d = curPoint.distanceTo(prevPoint);

		dc = (curPoint.hasValue(TelemetryData::DataFix)) ? d : 0;
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
			acceleration = 1000 * (speed - prevPoint.speed_) / dt;

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

		// In computed range
		curPoint.setComputed(enable);

		// Distance
		curPoint.setDistance(distance);

		// Duration, elasped time, ride itme
		curPoint.setDuration(duration);
		curPoint.setElapsedTime(elapsedtime);
		curPoint.setRideTime(ridetime);

		// speed, max speed
		curPoint.setSpeed(speed);
		curPoint.setMaxSpeed(maxspeed);
		curPoint.setAcceleration(acceleration);

		// average speed
		if (force || !curPoint.hasValue(TelemetryData::DataAverageSpeed)) {
			double avgspeed = (duration > 0) ? (3600.0 * distance) / (1000.0 * duration) : prevPoint.avgspeed_;
			curPoint.setAverageSpeed(avgspeed);
		}

		// average ride speed
		if (force || !curPoint.hasValue(TelemetryData::DataAverageRideSpeed)) {
			double avgridespeed = (ridetime > 0) ? (3600.0 * distance) / (1000.0 * ridetime) : prevPoint.avgridespeed_;
			curPoint.setAverageRideSpeed(avgridespeed);
		}

		// By default, in the same lap
		curPoint.setLap(prevPoint.lap());

		// Update telemetry data (between from & to)
		//-------------------------------------------
		if (enable) {
			// Determine current lap
			d = curPoint.distanceTo(start_);

			if (prevPoint.in_lap_ && (d < 20)) {
				curPoint.setLap(prevPoint.lap_ + 1); // lap_++;
				curPoint.in_lap_ = false;
			}
			else if (!prevPoint.in_lap_ && (d > 30)) {
				curPoint.in_lap_ = true;
			}
		}
	
		// Clear flag
		if (!enable && (curPoint.timestamp() < from_)) {
			// Before first point, no data
			curPoint.clearValue(
					TelemetryData::DataDuration
					| TelemetryData::DataDistance
					| TelemetryData::DataMaxSpeed
					| TelemetryData::DataRideTime
					| TelemetryData::DataAverageSpeed
					| TelemetryData::DataAverageRideSpeed
				);
		}

		pool_.current() = curPoint;
	}
	else {
		curPoint = pool_.current();

		// In computed range
		curPoint.setComputed(enable);

		// Set default value & flags
		if (enable) {
			curPoint.setDistance(0);
			curPoint.setDuration(0);
			curPoint.setRideTime(0);
		}

		curPoint.setElapsedTime(0);

		pool_.current() = curPoint;
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

	if (!quiet_ && (count > 0))
		log_notice("%s: %lu skip points", name().c_str(), count);

	return true;
}


void TelemetrySource::range(void) {
	uint64_t first = pool_.first().timestamp();
	uint64_t last = pool_.last().timestamp();

	// Set data range [begin:end]
	begin_ = (begin_ != 0) ? MAX(first, begin_) : first;
	end_ = (end_ != 0) ? MIN(last, end_) : last;

	// Set compute range [from:to]
	from_ = (from_ != 0) ? MAX(begin_, from_) : begin_;
	to_ = (to_ != 0) ? MIN(to_, end_) : end_;

	// Data range sumup
	if (!quiet_) {
		log_info("%s: Telemetry available data range from '%s' to '%s'", 
				name().c_str(),
				::timestamp2string(begin_).c_str(),
				::timestamp2string(end_).c_str());
	}

	// For each point
	while (pool_.size() > 0) {
		pool_.seek(1);

		// Insert points
		if ((pool_.current().timestamp() < begin_) && (pool_.next().timestamp() > begin_))
			insertData(begin_);
		else if ((pool_.current().timestamp() < from_) && (pool_.next().timestamp() > from_))
			insertData(from_);
		else if ((pool_.current().timestamp() < to_) && (pool_.next().timestamp() > to_))
			insertData(to_);
		else if ((pool_.current().timestamp() < end_) && (pool_.next().timestamp() > end_))
			insertData(end_);
	}

	pool_.reset();
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
		if (!quiet_)
			log_info("%s: Filter telemetry data in using 'Iglewicz & Hoaglin' method.", name().c_str());

		// Build acceleration sorted list
		for (size_t i=1; i<=pool_.size(); i++)
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
		for (size_t i=1; i<=pool_.size(); i++) {
			if (fabs((0.6745 * (pool_[i].acceleration() - m)) / M) > 5.0) {
				pool_[i].setType(TelemetryData::TypeError);
				n++;
			}
		}

		if (!quiet_)
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

	if (!quiet_) {
		log_info("%s: Compute telemetry data from '%s' to '%s'", 
				name().c_str(),
				::timestamp2string(from_).c_str(),
				::timestamp2string(to_).c_str());
	}

	// For each point
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
		if (!data.hasValue(TelemetryData::DataSpeed)) {
			// If no data, we can't assume that it's a pause
			ss = -1;
			done = 0;
			timestamp = data.timestamp();
		}
		else if (data.speed() > 1.5) {
			// In move
			ss = -1;
			done = 0;
			timestamp = data.timestamp();
			grade = pool_.current().grade();
		}
		else {
			// In pause
			ss++;

			ele = pool_.current().elevation();
		}

		// Pause detection
		if ((ss >= 0) && (data.timestamp() > (timestamp + interval))) {
			// pause detected
			if (pool_.current().hasValue(TelemetryData::DataFix)) {
				pool_.current().setPause(true);
				pool_.current().setSpeed(0);
				pool_.current().setAcceleration(0);
				pool_.current().setElevation(ele);
				pool_.current().setGrade(grade);

				for (int k=0; k<=ss-done; k++) {
					pool_.previous(k).setPause(true);

					if (pool_.previous(k).hasValue(TelemetryData::DataFix)) {
						pool_.previous(k).setSpeed(0);
						pool_.previous(k).setAcceleration(0);
						pool_.previous(k).setElevation(ele);
						pool_.previous(k).setGrade(grade);
					}
				}
			
				done = ss;
			}
		}
	}

	pool_.reset();
}


/**
 * Smooth data:
 *  - elevation
 *  - grade
 *  - speed
 *  - acceleration
 * So re-compute:
 *  - max speed
 */
void TelemetrySource::smooth(void) {
	int count = 0;
	size_t window = (smooth_points_ * 2) + 1;

	double elevation = 0;
	double grade = 0;
	double speed = 0;
	double maxspeed = 0;
	double acceleration = 0;

	std::deque<Point> points;

	TelemetrySource::Point nextPoint, previousPoint;

	if (window < 2)
		return;

	if (!quiet_) {
		log_info("%s: Smooth telemetry data on window size '%ld' (+/- %d points)", 
				name().c_str(), window, smooth_points_);
	}

	// Fill window
	for (size_t i=0; i<window/2; i++) {
		nextPoint = pool_.next(i);

		if (nextPoint.type() == TelemetryData::TypeUnknown)
			break;

		// Save point
		points.emplace_back(nextPoint);

		if (nextPoint.hasValue(TelemetryData::DataFix)) {
			elevation += nextPoint.elevation();
			grade += nextPoint.grade();
			speed += nextPoint.speed();
			acceleration += nextPoint.acceleration();
			count += 1;
		}
	}

	// Move to first point
	pool_.seek(1);

	// Compute average value for each point
	while (!pool_.empty()) {
		if (pool_.current().type() != TelemetryData::TypeError) {
			// Add new point
			nextPoint = pool_.next(window/2 - 1);

			// Save point
			points.emplace_back(nextPoint);

			if (nextPoint.type() != TelemetryData::TypeUnknown) {
				if (nextPoint.hasValue(TelemetryData::DataFix)) {
					elevation += nextPoint.elevation();
					grade += nextPoint.grade();
					speed += nextPoint.speed();
					acceleration += nextPoint.acceleration();
					count += 1;
				}
			}

			// Remove old point
			if (points.size() > window) {
				previousPoint = points.front();
				points.pop_front();

				if (previousPoint.hasValue(TelemetryData::DataFix)) {
					elevation -= previousPoint.elevation();
					grade -= previousPoint.grade();
					speed -= previousPoint.speed();
					acceleration -= previousPoint.acceleration();
					count -= 1;
				}
			}

			// New speed
			if (!pool_.current().isPause()) {
				if (pool_.current().hasValue(TelemetryData::DataFix)) {
					if (pool_.current().hasValue(TelemetryData::DataMaxSpeed))
						maxspeed = MAX(maxspeed, speed / count);

					pool_.current().setElevation(elevation / count);
					pool_.current().setGrade(grade / count);
					pool_.current().setSpeed(speed / count);
					pool_.current().setAcceleration(acceleration / count);

					if (pool_.current().hasValue(TelemetryData::DataMaxSpeed))
						pool_.current().setMaxSpeed(maxspeed);
				}
			}
		}

		// Move to next
		pool_.seek(1);
	}

	pool_.reset();
}


/**
 * Fix bad point by interpolation:
 *  - speed, avgspeed, avgridespeed
 *  - acceleration
 *  - distance
 *  - elevation
 * So compute:
 *  - grade
 *  - maxspeed
 */
void TelemetrySource::fix(void) {
	TelemetrySource::Point currentPoint;
	TelemetrySource::Point prevPoint, nextPoint;

	// Move to first point
	pool_.seek(1);

	prevPoint = pool_.current();

	// Process each point
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
			bool in_range;

			double k =  (double) (currentPoint.ts_ - prevPoint.ts_) / (double) (nextPoint.ts_ - prevPoint.ts_);

			// Check if we have to and able to compute data
			in_range = inRange(currentPoint.timestamp());

			if (currentPoint.hasValue(TelemetryData::DataSpeed)) {
				double speed = prevPoint.speed_ + k * (nextPoint.speed_ - prevPoint.speed_);
				currentPoint.setSpeed(speed);
				currentPoint.setPause((speed == 0));
			}

			if (currentPoint.hasValue(TelemetryData::DataDuration)) {
				double duration = prevPoint.duration_;
				if (!currentPoint.isPause())
				   duration += (currentPoint.ts_ - prevPoint.ts_) / 1000;
				currentPoint.setDuration(duration);
			}

			if (currentPoint.hasValue(TelemetryData::DataRideTime)) {
				double ridetime = prevPoint.ridetime_;
				if (!currentPoint.isPause())
				   ridetime += (currentPoint.ts_ - prevPoint.ts_) / 1000;
				currentPoint.setRideTime(ridetime);
			}

			if (currentPoint.hasValue(TelemetryData::DataElevation)) {
				double ele = prevPoint.ele_ + k * (nextPoint.ele_ - prevPoint.ele_);
				currentPoint.setElevation(ele);
			}
			
			if (currentPoint.hasValue(TelemetryData::DataGrade)) {
				double distance = currentPoint.distanceTo(prevPoint);
				double grade = ((distance > 0) && (currentPoint.speed() > 0)) ? 100.0 * (currentPoint.elevation() - prevPoint.ele_) / distance : prevPoint.grade();
				currentPoint.setGrade(grade);

				// Upgrade grade for next point
				if (!nextPoint.isPause())
					nextPoint.setGrade(grade);
			}

			if (currentPoint.hasValue(TelemetryData::DataAcceleration)) {
				double acceleration = prevPoint.acceleration_ + k * (nextPoint.acceleration_ - prevPoint.acceleration_);
				currentPoint.setAcceleration(acceleration);
			}

			if (currentPoint.hasValue(TelemetryData::DataDistance)) {
				double distance = in_range ? prevPoint.distance_ + k * (nextPoint.distance_ - prevPoint.distance_) : prevPoint.distance_;
				currentPoint.setDistance(distance);
			}

			if (currentPoint.hasValue(TelemetryData::DataMaxSpeed)) {
				double maxspeed = in_range ? MAX(currentPoint.speed(), prevPoint.maxspeed()) : prevPoint.maxspeed();
				currentPoint.setMaxSpeed(maxspeed);
			}

			if (currentPoint.hasValue(TelemetryData::DataAverageSpeed)) {
				double avgspeed = in_range ? prevPoint.avgspeed_ + k * (nextPoint.avgspeed_ - prevPoint.avgspeed_) : prevPoint.avgspeed_;
				currentPoint.setAverageSpeed(avgspeed);
			}

			if (currentPoint.hasValue(TelemetryData::DataAverageRideSpeed)) {
				double avgridespeed = in_range ? prevPoint.avgridespeed_ + k * (nextPoint.avgridespeed_ - prevPoint.avgridespeed_) : prevPoint.avgridespeed_;
				currentPoint.setAverageRideSpeed(avgridespeed);
			}

			currentPoint.setType(TelemetryData::TypeFixed);

			// Save points
			pool_.next() = nextPoint;
			pool_.current() = currentPoint;
		}
	}

	pool_.reset();
}


void TelemetrySource::trim(void) {
	TelemetrySource::Point point;

	// Move to first point
	pool_.seek(1);

	// Check each point
	while (!pool_.empty()) {
		point = pool_.current();

		// Drop data before 'begin_' timestamp
		if (point.timestamp() < begin_) {
			pool_.remove();
			continue;
		}

		// Drop data after 'end_' timestamp
		if (point.timestamp() > end_) {
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

		if (data.timestamp() < beginTimestamp())
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

		if (data.timestamp() > endTimestamp())
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
	range();
	filter();
	compute();
	smooth();
	fix();
	trim();

	return type;
}


void TelemetrySource::insertData(uint64_t timestamp) {
	int flags;

	TelemetryData data;

	TelemetrySource::Point point;

	// Current data
	data = pool_.current();

	// Predict data by interpolation
	predictData(data, TelemetrySettings::MethodInterpolate, timestamp);

	// Create the new point
	point.setType(TelemetryData::TypePredicted);
	point.setPosition(timestamp, data.latitude(), data.longitude());
	point.setElevation(data.elevation());
	point.setPower(data.power());
	point.setCadence(data.cadence());
	point.setHeartrate(data.heartrate());
	point.setTemperature(data.temperature());

	// Set values flag
	flags = data.has_value_ & (
			TelemetryData::DataFix
			| TelemetryData::DataElevation
			| TelemetryData::DataPower
			| TelemetryData::DataCadence
			| TelemetryData::DataHeartrate
			| TelemetryData::DataTemperature);
	point.setValue(flags);

	// Insert point
	pool_.insert(point);
}


void TelemetrySource::predictData(TelemetryData &data, TelemetrySettings::Method method, uint64_t timestamp) {
	uint64_t offset;

	double k;

	bool in_range = true;

	TelemetrySource::Point point;

	TelemetrySource::Point prevPoint, curPoint, nextPoint;

	log_call();

	// Check if we have to and able to compute data
	in_range = inRange(timestamp);

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
		if ((pool_.tell() == -1) || (pool_.size() == 0)) {
			if (pool_.backlog() > 1)
				method = TelemetrySettings::MethodLinear;
			else
				method = TelemetrySettings::MethodSample;
		}
		break;

	case TelemetrySettings::MethodLinear:
		if (pool_.backlog() < 2) {
			if ((pool_.size() > 0) && (pool_.backlog() > 0))
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

		// K
		k = (double) (timestamp - prevPoint.ts_) / (double) (nextPoint.ts_ - prevPoint.ts_);

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
			point.setElevation(prevPoint.ele_ + k * (nextPoint.ele_ - prevPoint.ele_));

		if (prevPoint.hasValue(TelemetryData::DataCadence) && nextPoint.hasValue(TelemetryData::DataCadence))
			point.setCadence(prevPoint.cadence_ + k * (nextPoint.cadence_ - prevPoint.cadence_));

		if (prevPoint.hasValue(TelemetryData::DataHeartrate) && nextPoint.hasValue(TelemetryData::DataHeartrate))
			point.setHeartrate(prevPoint.heartrate_ + k * (nextPoint.heartrate_ - prevPoint.heartrate_));

		if (prevPoint.hasValue(TelemetryData::DataTemperature) && nextPoint.hasValue(TelemetryData::DataTemperature))
			point.setTemperature(prevPoint.temperature_ + k * (nextPoint.temperature_ - prevPoint.temperature_));

		if (prevPoint.hasValue(TelemetryData::DataPower) && nextPoint.hasValue(TelemetryData::DataPower))
			point.setPower(prevPoint.power_ + k * (nextPoint.power_ - prevPoint.power_));

		// Computed data
		if (prevPoint.hasValue(TelemetryData::DataSpeed) && nextPoint.hasValue(TelemetryData::DataSpeed))
			point.setSpeed(prevPoint.speed_ + k * (nextPoint.speed_ - prevPoint.speed_));

		if (prevPoint.hasValue(TelemetryData::DataAcceleration) && nextPoint.hasValue(TelemetryData::DataAcceleration))
			point.setAcceleration(prevPoint.acceleration_ + k * (nextPoint.acceleration_ - prevPoint.acceleration_));

		if (prevPoint.hasValue(TelemetryData::DataDistance) && nextPoint.hasValue(TelemetryData::DataDistance)) {
			double distance = in_range ? prevPoint.distance_ + k * (nextPoint.distance_ - prevPoint.distance_) : prevPoint.distance_;
			point.setDistance(distance);
		}

		if (prevPoint.hasValue(TelemetryData::DataGrade) && nextPoint.hasValue(TelemetryData::DataGrade))
			point.setGrade(prevPoint.grade_ + k * (nextPoint.grade_ - prevPoint.grade_));

		if (prevPoint.hasValue(TelemetryData::DataAverageSpeed) && nextPoint.hasValue(TelemetryData::DataAverageSpeed)) {
			double avgspeed = in_range ? prevPoint.avgspeed_ + k * (nextPoint.avgspeed_ - prevPoint.avgspeed_) : prevPoint.avgspeed_;
			point.setAverageSpeed(avgspeed);
		}

		if (prevPoint.hasValue(TelemetryData::DataAverageRideSpeed) && nextPoint.hasValue(TelemetryData::DataAverageRideSpeed)) {
			double avgridespeed = in_range ? prevPoint.avgridespeed_ + k * (nextPoint.avgridespeed_ - prevPoint.avgridespeed_) : prevPoint.avgridespeed_;
			point.setAverageRideSpeed(avgridespeed);
		}
		break;

	case TelemetrySettings::MethodLinear:
		prevPoint = pool_.previous();
		curPoint = pool_.current();

		// K
		k =  (double) (timestamp - prevPoint.ts_) / (double) (curPoint.ts_ - prevPoint.ts_);

		// Restore previous computed data
		point.restore(curPoint);

		// Compute new position
		point.setPosition(
			timestamp,
			data.lat_ + k * (curPoint.lat_ - prevPoint.lat_),
			data.lon_ + k * (curPoint.lon_ - prevPoint.lon_)
		);

		// Measured data
		if (prevPoint.hasValue(TelemetryData::DataElevation) && curPoint.hasValue(TelemetryData::DataElevation))
			point.setElevation(curPoint.ele_ + k * (curPoint.ele_ - prevPoint.ele_));

		if (prevPoint.hasValue(TelemetryData::DataCadence) && curPoint.hasValue(TelemetryData::DataCadence))
			point.setCadence(curPoint.cadence_ + k * (curPoint.cadence_ - prevPoint.cadence_));

		if (prevPoint.hasValue(TelemetryData::DataHeartrate) && curPoint.hasValue(TelemetryData::DataHeartrate))
			point.setHeartrate(curPoint.heartrate_ + k * (curPoint.heartrate_ - prevPoint.heartrate_));

		if (prevPoint.hasValue(TelemetryData::DataTemperature) && curPoint.hasValue(TelemetryData::DataTemperature))
			point.setTemperature(curPoint.temperature_ + k * (curPoint.temperature_ - prevPoint.temperature_));

		if (prevPoint.hasValue(TelemetryData::DataPower) && curPoint.hasValue(TelemetryData::DataPower))
			point.setPower(curPoint.power_ + k * (curPoint.power_ - prevPoint.power_));

		// Computed data
		if (prevPoint.hasValue(TelemetryData::DataSpeed) && curPoint.hasValue(TelemetryData::DataSpeed))
			point.setSpeed(curPoint.speed_ + k * (curPoint.speed_ - prevPoint.speed_));

		if (prevPoint.hasValue(TelemetryData::DataAcceleration) && curPoint.hasValue(TelemetryData::DataAcceleration))
			point.setAcceleration(curPoint.acceleration_ + k * (curPoint.acceleration_ - prevPoint.acceleration_));

		if (prevPoint.hasValue(TelemetryData::DataDistance) && curPoint.hasValue(TelemetryData::DataDistance)) {
			double distance = in_range ? curPoint.distance_ + k * (curPoint.distance_ - prevPoint.distance_) : curPoint.distance_;
			point.setDistance(distance);
		}

		if (prevPoint.hasValue(TelemetryData::DataGrade) && curPoint.hasValue(TelemetryData::DataGrade))
			point.setGrade(curPoint.grade_ + k * (curPoint.grade_ - prevPoint.grade_));

		if (prevPoint.hasValue(TelemetryData::DataAverageSpeed) && curPoint.hasValue(TelemetryData::DataAverageSpeed)) {
			double avgspeed = in_range ? curPoint.avgspeed_ + k * (curPoint.avgspeed_ - prevPoint.avgspeed_) : curPoint.avgspeed_;
			point.setAverageSpeed(avgspeed);
		}

		if (prevPoint.hasValue(TelemetryData::DataAverageRideSpeed) && curPoint.hasValue(TelemetryData::DataAverageRideSpeed)) {
			double avgridespeed = in_range ? curPoint.avgridespeed_ + k * (curPoint.avgridespeed_ - prevPoint.avgridespeed_) : curPoint.avgridespeed_;
			point.setAverageRideSpeed(avgridespeed);
		}
		break;

	case TelemetrySettings::MethodSample:
		// Create a new point (just copy previous & update timestamp)
		if (pool_.tell() >= 0) 
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

	if (method != TelemetrySettings::MethodNone) {
		// Update common data
		//--------------------
		double duration = data.duration();
		double ridetime = data.rideTime();
		double maxspeed = data.maxspeed();

		point.setElapsedTime(data.elapsedTime() + (offset / 1000.0));

		if (in_range || data.hasValue(TelemetryData::DataDuration)) {
			duration += in_range ? (offset / 1000.0) : 0;
			point.setDuration(duration);
		}

		if (in_range || data.hasValue(TelemetryData::DataRideTime)) {
			ridetime += in_range && (data.speed() > 0) ? (offset / 1000.0) : 0;
			point.setRideTime(ridetime);
		}

		if (in_range || data.hasValue(TelemetryData::DataMaxSpeed)) {
			if (in_range)
				maxspeed = MAX(maxspeed, point.speed());
			point.setMaxSpeed(maxspeed);
		}

		// Return predict point
		data = point;
	}
}


void TelemetrySource::cleanData(TelemetryData &data, uint64_t timestamp) {
	TelemetrySource::Point point;
	TelemetrySource::Point prevPoint;

	log_call();

	// Get last point
	prevPoint = pool_.current();

	// Predict at timestamp
	point.setType(TelemetryData::TypePredicted);
	point.setTimestamp(timestamp);

	// Restore previous computed data
	point.restore(prevPoint, true);

	// Clean
	point.clearValue(
			TelemetryData::DataFix
			| TelemetryData::DataElevation
			| TelemetryData::DataCadence
			| TelemetryData::DataHeartrate
			| TelemetryData::DataTemperature
			| TelemetryData::DataPower

			| TelemetryData::DataSpeed
			| TelemetryData::DataGrade
			| TelemetryData::DataAcceleration
		);

	// Return point
	data = point;
}


void TelemetrySource::updateData(TelemetryData &data) {
	log_call();

	if (pool_.empty())
		goto skip;

	data = pool_.current();

skip:
	return;
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

	return retrieveData(data);
}


enum TelemetrySource::Data TelemetrySource::retrieveFrom(TelemetryData &data) {
	enum TelemetrySource::Data result;

	result = retrieveFirst(data);

	if (from_ != 0) {
		result = retrieveNext(data, from_);
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
				predictData(data, method_, timestamp);
			}
			else {
//				printf(" <next> ");

				if (this->retrieveData(data) == TelemetrySource::DataEof) {
//					printf(" <clean %lu> ", timestamp);
					cleanData(data, timestamp);
					goto eof;
				}
	
				nextPoint = pool_.next();
			}
		}
	} while ((timestamp != (uint64_t) -1) && (data.timestamp() < timestamp));

//	printf("\n");

	return type;

eof:
	return TelemetrySource::Data::DataEof;
}


enum TelemetrySource::Data TelemetrySource::retrieveTo(TelemetryData &data) {
	enum TelemetrySource::Data result;

	if (to_ != 0) {
		result = retrieveFirst(data);

		result = retrieveNext(data, to_);
	}
	else
		result = retrieveLast(data);

	return result;
}


enum TelemetrySource::Data TelemetrySource::retrieveLast(TelemetryData &data) {
	log_call();

	data = pool_.last();

	return (data.type() != TelemetryData::TypeUnknown) ? TelemetrySource::DataAgain : TelemetrySource::DataEof;
}


TelemetrySource * TelemetryMedia::open(const std::string &filename, const TelemetrySettings &settings, bool quiet) {
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
		bool ok = true;

		// Verbose info
		source->setQuiet(quiet);

		// Timestamp offset
		source->setTimeOffset(settings.telemetryOffset());

		// Telemetry range
		ok &= source->setDataRange(settings.telemetryBegin(), settings.telemetryEnd());
		ok &= source->setComputeRange(settings.telemetryFrom(), settings.telemetryTo());

		// Telemetry data filter
		source->skipBadPoint(settings.telemetryCheck());
		source->setFilter(settings.telemetryFilter());
		source->setMethod(settings.telemetryMethod());
		source->setSmoothPoints(settings.telemetrySmoothPoints());

		// Load telemtry data
		source->loadData();

		if (!ok) {
			delete source;

			source = NULL;
		}
	}

	return source;
}

