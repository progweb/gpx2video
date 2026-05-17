#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>

#include <time.h>

#include <GeographicLib/Geodesic.hpp>

#include "log_i.h"
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

	index_ = -1;

	if (all) {
		line_ = 0;

		type_ = TelemetryData::TypeUnknown;

		ts_ = 0;
		datetime_ = 0;
		lat_ = 0.0;
		lon_ = 0.0;
		ele_ = 0.0;

		raw_lat_ = 0;
		raw_lon_ = 0;

		cadence_ = 0;
		heartrate_ = 0;
		temperature_ = 0;
		power_ = 0;
		batterylevel_ = 0.0;
	
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
	verticalspeed_ = 0.0;
	homedistance_ = 0.0;

	lap_ = 1;
	in_lap_ = false;
}


void TelemetryData::dump(void) const {
	printf("  [%d] '%s' (Fix: %c) Time: %s [%f, %f] Distance: %.3f km in %d sec, speed: %.3f (pause: %s) - Altitude: %.1f (%.1f%%)\n",
			line_,
			type2string(),
			hasValue(TelemetryData::DataFix) ? 'Y' : 'N',
			Datetime::timestamp2string(ts_).c_str(),
			lon_, lat_,
			distance_/1000.0, (int) round(duration_), speed_, 
			is_pause_ ? "true": "false",
			ele_, grade_);
}


void TelemetryData::writeHeader(void) {
	printf("      | Line  | T | Datetime (ms)           | Duration | Distance    | Speed  | Acceleration | S | Latitude     | Longitude    | Altitude | Head | Grade   \n");
	printf("------+-------+---+-------------------------+----------+-------------+--------+--------------+---+--------------+--------------+----------+------+---------\n");
}


void TelemetryData::writeData(size_t index) const {
	printf("%5ld | %5d | %s | %s | %8d | %11.3f | %6.1f | %12.8f | %1s | %12.8f | %12.8f | %8.1f | %4d° | %5.1f%%\n",
		index,
		line_,
		type2string(),
		Datetime::timestamp2string(ts_).c_str(),
		(int) round(duration_), distance_/1000.0, speed_, acceleration_ / 9.81,
		is_pause_ ? "S" : " ",
		lat_, lon_,
		ele_, (int) heading_, grade_);
}


double TelemetrySource::Point::distanceTo(const Point &to) {
	double d = 0.0;

	GeographicLib::Geodesic gsic(6378137.0, 1.0/298.2572);

	// Maths
	gsic.Inverse(lat_, lon_, to.lat_, to.lon_, d);

	return d;
}


double TelemetrySource::Point::headingTo(const Point &to) {
	double azi1, azi2;

	GeographicLib::Geodesic gsic(6378137.0, 1.0/298.2572);

	// Maths
	gsic.Inverse(lat_, lon_, to.lat_, to.lon_, azi1, azi2);

	if (azi1 < 0)
		azi1 += 380.0;

	return azi1; //1 + 180.0;
}


TelemetrySource::TelemetrySource(const std::string &filename) 
	: filename_(filename)
	, quiet_(true)
	, eof_(false)
	, check_(false)
	, offset_(0) 
	, begin_(0)
	, end_(0)
	, view_start_(0)
	, view_stop_(0)
	, compute_start_(0)
	, compute_stop_(0) {
	log_call();

//	pool_.setNumberOfPoints(100);

    stream_ = std::ifstream(filename);

	kalman_ = alloc_filter_velocity2d(10.0);
}


TelemetrySource::~TelemetrySource() {
}


const std::string& TelemetrySource::filename(void) const {
	return filename_;
}


void TelemetrySource::setQuiet(const bool quiet) {
	log_call();

	quiet_ = quiet;
}


void TelemetrySource::setNumberOfPoints(const unsigned long number) {
	log_call();

	pool_.setNumberOfPoints(number);
}


size_t TelemetrySource::numberOfPoints(void) const {
	return pool_.count();
}


bool TelemetrySource::inRange(uint64_t timestamp) const {
	bool in_range = true;

	uint64_t from, to;

	from = !settings().telemetryComputeFrom() ? settings().telemetryBegin() : settings().telemetryComputeFrom();
	to = !settings().telemetryComputeTo() ? settings().telemetryEnd() : settings().telemetryComputeTo();

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
	double heading = 0;
	double ridetime = 0;
	double elapsedtime = 0;
	double speed = 0;
	double maxspeed = 0;
	double acceleration = 0;
	double verticalspeed = 0;
	double homedistance = 0;

	GeographicLib::Math::real d, h;

	TelemetrySource::Point firstPoint;
	TelemetrySource::Point prevPoint, curPoint;

	log_call();

	// First point
	firstPoint = pool_.first();

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
		d = prevPoint.distanceTo(curPoint);
		h = prevPoint.headingTo(curPoint);

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

		// Home distance
		if (force || !curPoint.hasValue(TelemetryData::DataHomeDistance))
			homedistance = curPoint.distanceTo(firstPoint);
		else
			homedistance = curPoint.homedistance();

		// Heading
		if (force || !curPoint.hasValue(TelemetryData::DataHeading))
			heading = h;
		else
			heading = curPoint.heading();

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

		if (curPoint.hasValue(TelemetryData::DataElevation)) {
			// Grade elevation
			if (force || !curPoint.hasValue(TelemetryData::DataGrade)) {
				if (floor(dc) > 0)
					curPoint.setGrade(100.0 * dz / dc);
				else
					curPoint.setGrade(prevPoint.grade_);
			}

			// Vertical speed
			if (force || !curPoint.hasValue(TelemetryData::DataVerticalSpeed))
				verticalspeed = prevPoint.verticalspeed_;
			else
				verticalspeed = curPoint.verticalspeed();
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

			// Compute vertical speed (m/s)
			if (force || !curPoint.hasValue(TelemetryData::DataVerticalSpeed))
				verticalspeed = 1000.0 * dz / (1.0 * dt);
		}

		// Update telemetry data
		//-----------------------

		// In computed range
		curPoint.setComputed(enable);

		// Distance
		curPoint.setDistance(distance);

		// Home distance
		curPoint.setHomeDistance(homedistance);

		// Heading
		curPoint.setHeading(heading);

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

		// Vertical speed
		curPoint.setVerticalSpeed(verticalspeed);

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
		if (!enable && (curPoint.timestamp() < compute_start_)) {
			// Before first point, no computed data
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
			curPoint.setHomeDistance(0);
			curPoint.setDuration(0);
			curPoint.setRideTime(0);
			curPoint.setHeading(pool_.next().heading());
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


void TelemetrySource::config(void) {
	log_call();

	// Timestamp offset
	offset_ = settings().telemetryOffset();

	// Telemetry range
	begin_ = settings().telemetryBegin();
	end_ = settings().telemetryEnd();

	view_start_ = settings().telemetryViewFrom();
	view_stop_ = settings().telemetryViewTo();

	compute_start_ = settings().telemetryComputeFrom();
	compute_stop_ = settings().telemetryComputeTo();

	// Telemtry data filter
	check_ = settings().telemetryCheck();
	pause_detection_ = settings().telemetryPauseDetection();
	filter_ = settings().telemetryFilter();
	method_ = settings().telemetryMethod();
	rate_ = settings().telemetryRate();
}


bool TelemetrySource::load(void) {
	size_t count = 0;

	TelemetryData data;

	TelemetrySource::Point point;
	TelemetrySource::Point previous;

	enum TelemetrySource::Data type = TelemetrySource::DataUnknown;

	log_call();

	// For each point
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

	// Back to first point
	pool_.reset();

	// Compute first point again
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

	// Set view range [from:to]
	view_start_ = (view_start_ != 0) ? MAX(begin_, view_start_) : begin_;
	view_stop_ = (view_stop_ != 0) ? MIN(view_stop_, end_) : end_;

	// Set compute range [from:to]
	compute_start_ = (compute_start_ != 0) ? MAX(begin_, compute_start_) : begin_;
	compute_stop_ = (compute_stop_ != 0) ? MIN(compute_stop_, end_) : end_;

	// Data range sumup
	if (!quiet_) {
		log_info("%s: Telemetry available data range from '%s' to '%s'", 
				name().c_str(),
				Datetime::timestamp2string(begin_).c_str(),
				Datetime::timestamp2string(end_).c_str());
	}

	// For each point
	while (pool_.size() > 0) {
		pool_.seek(1);

		// Insert points
		if ((pool_.current().timestamp() < begin_) && (pool_.next().timestamp() > begin_))
			insertData(begin_);
		else if ((pool_.current().timestamp() < compute_start_) && (pool_.next().timestamp() > compute_start_))
			insertData(compute_start_);
		else if ((pool_.current().timestamp() < compute_stop_) && (pool_.next().timestamp() > compute_stop_))
			insertData(compute_stop_);
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
	switch (settings().telemetryFilter()) {
	case TelemetrySettings::FilterOutlier:
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
	int gs = -1;
	int ss = -1;
	int done = 0;

	double dc = 0.0;
	double dz = 0.0;

	double sele = 0.0;
	double sheading = 0.0;

	double grade = 0.0;
	double distance = 0.0;
	double elevation = 0.0;

	uint64_t interval = 4000; // 4s

	uint64_t timestamp = -1;

	TelemetryData data;

	if (!quiet_) {
		log_info("%s: Compute telemetry data from '%s' to '%s'", 
				name().c_str(),
				Datetime::timestamp2string(compute_start_).c_str(),
				Datetime::timestamp2string(compute_stop_).c_str());
		log_info("%s: Pause detection: %s",
				name().c_str(),
				pause_detection_ ? "enabled" : "disabled");
	}

	if ((settings().telemetryFilter() == TelemetrySettings::FilterNone) && !pause_detection_)
		return;

	// Start elevation
	elevation = pool_.first().elevation();

	// For each point
	while (pool_.size() > 0) {
		pool_.seek(1);

		// Skip incoherent point
		if (pool_.current().type() == TelemetryData::TypeError)
			continue;

		compute_i(data, true);

		// Compute grade, each 10m
		dc += pool_.current().distance() - distance;
		dz += pool_.current().elevation() - elevation;

		// Distance enough (> 10 meter), compute grade
		if ((ss == -1) && (dc > 10)) {
			grade = 100.0 * dz / dc;

			if (pool_.current().hasValue(TelemetryData::DataFix))
				pool_.current().setGrade(grade);

			for (int k=0; k<=gs; k++) {
				if (pool_.previous(k).hasValue(TelemetryData::DataFix))
					pool_.previous(k).setGrade(grade);
			}

			dc = 0;
			dz = 0;

			gs = -1;
		}
		else
			gs++;

		// > 2.8 m/us => 0.40 m/us 
		// < 2.8 m/us => 0.15 m/us
		// Detect move above...
		if (!data.hasValue(TelemetryData::DataSpeed)) {
			// If no data, we can't assume that it's a pause
			ss = -1;
			done = 0;
			timestamp = data.timestamp();
		}
		else if (!pause_detection_ || data.speed() > 1.5) {
			// In move
			ss = -1;
			done = 0;
			timestamp = data.timestamp();
			grade = pool_.current().grade();
		}
		else {
			// In pause
			ss++;

			if (done == 0) {
				sele = pool_.current().elevation();
				sheading = pool_.current().heading();
			}
		}

		// Pause detection
		if (pause_detection_) {
			if ((ss >= 0) && (data.timestamp() > (timestamp + interval))) {
				// pause detected
				if (pool_.current().hasValue(TelemetryData::DataFix)) {
					pool_.current().setPause(true);
					pool_.current().setSpeed(0);
					pool_.current().setAcceleration(0);
					pool_.current().setHeading(sheading);
					pool_.current().setElevation(sele);
					pool_.current().setGrade(grade);

					for (int k=0; k<=ss-done; k++) {
						pool_.previous(k).setPause(true);

						if (pool_.previous(k).hasValue(TelemetryData::DataFix)) {
							pool_.previous(k).setSpeed(0);
							pool_.previous(k).setAcceleration(0);
							pool_.previous(k).setHeading(sheading);
							pool_.previous(k).setElevation(sele);
							pool_.previous(k).setGrade(grade);
						}
					}
				
					done = ss;
				}
			}
		}

		// Save last data
		distance = pool_.current().distance();
		elevation = pool_.current().elevation();
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
void TelemetrySource::smooth_step_one(void) {
	bool first = true;

	int gs = -1;

	double dc = 0;
	double dz = 0;

	int speed_count = 0;
	int heading_count = 0;
	int elevation_count = 0;
	int acceleration_count = 0;

	size_t speed_window = 0;
	size_t heading_window = 0;
	size_t elevation_window = 0;
	size_t acceleration_window = 0;

	double grade = 0;
	double distance = 0;
	double elevation = 0;
	double maxspeed = 0;

	double speed_sum = 0;
	double heading_sum = 0;
	double elevation_sum = 0;
	double acceleration_sum = 0;

	TelemetrySettings::Smooth speed_method;
	TelemetrySettings::Smooth heading_method;
	TelemetrySettings::Smooth elevation_method;
	TelemetrySettings::Smooth acceleration_method;

	std::deque<Point> speed_points;
	std::deque<Point> heading_points;
	std::deque<Point> elevation_points;
	std::deque<Point> acceleration_points;

	TelemetrySource::Point nextPoint, previousPoint;

	log_call();

	speed_method = settings().telemetrySmoothMethod(TelemetryData::DataSpeed);
	heading_method = settings().telemetrySmoothMethod(TelemetryData::DataHeading);
	elevation_method = settings().telemetrySmoothMethod(TelemetryData::DataElevation);
	acceleration_method = settings().telemetrySmoothMethod(TelemetryData::DataAcceleration);

	speed_window = (settings().telemetrySmoothMethod(TelemetryData::DataSpeed) == TelemetrySettings::SmoothWindowedMovingAverage) ? 
		(settings().telemetrySmoothPoints(TelemetryData::DataSpeed) * 2) + 1 : 0;
	heading_window = (settings().telemetrySmoothMethod(TelemetryData::DataHeading) == TelemetrySettings::SmoothWindowedMovingAverage) ? 
		(settings().telemetrySmoothPoints(TelemetryData::DataHeading) * 2) + 1 : 0;
	elevation_window = (settings().telemetrySmoothMethod(TelemetryData::DataElevation) == TelemetrySettings::SmoothWindowedMovingAverage) ? 
		(settings().telemetrySmoothPoints(TelemetryData::DataElevation) * 2) + 1 : 0;
	acceleration_window = (settings().telemetrySmoothMethod(TelemetryData::DataAcceleration) == TelemetrySettings::SmoothWindowedMovingAverage) ? 
		(settings().telemetrySmoothPoints(TelemetryData::DataAcceleration) * 2) + 1 : 0;

	// Fill 'speed' window
	for (size_t i=0; i<speed_window/2; i++) {
		nextPoint = pool_.next(i);

		if (nextPoint.type() == TelemetryData::TypeUnknown)
			break;

		// Save point
		speed_points.emplace_back(nextPoint);

		if (nextPoint.hasValue(TelemetryData::DataFix)) {
			speed_sum += nextPoint.speed();
			speed_count += 1;
		}
	}

	// Fill 'heading' window
	for (size_t i=0; i<heading_window/2; i++) {
		nextPoint = pool_.next(i);

		if (nextPoint.type() == TelemetryData::TypeUnknown)
			break;

		// Save point
		heading_points.emplace_back(nextPoint);

		if (nextPoint.hasValue(TelemetryData::DataFix)) {
			heading_sum += nextPoint.heading();
			heading_count += 1;
		}
	}

	// Fill 'elevation' window
	for (size_t i=0; i<elevation_window/2; i++) {
		nextPoint = pool_.next(i);

		if (nextPoint.type() == TelemetryData::TypeUnknown)
			break;

		// Save point
		elevation_points.emplace_back(nextPoint);

		if (nextPoint.hasValue(TelemetryData::DataFix)) {
			elevation_sum += nextPoint.elevation();
			elevation_count += 1;
		}
	}

	// Fill 'acceleration' window
	for (size_t i=0; i<acceleration_window/2; i++) {
		nextPoint = pool_.next(i);

		if (nextPoint.type() == TelemetryData::TypeUnknown)
			break;

		// Save point
		acceleration_points.emplace_back(nextPoint);

		if (nextPoint.hasValue(TelemetryData::DataFix)) {
			acceleration_sum += nextPoint.acceleration();
			acceleration_count += 1;
		}
	}

	// Move to first point
	pool_.seek(1);

	// Compute average value for each point
	while (!pool_.empty()) {
		if (pool_.current().type() != TelemetryData::TypeError) {
			// speed
			//-------

			if (speed_method == TelemetrySettings::SmoothWindowedMovingAverage) {
				if (speed_window > 1) {
					// Add new point
					nextPoint = pool_.next(speed_window/2 - 1);

					// Save point
					speed_points.emplace_back(nextPoint);

					if (nextPoint.type() != TelemetryData::TypeUnknown) {
						if (nextPoint.hasValue(TelemetryData::DataFix)) {
							speed_sum += nextPoint.speed();
							speed_count += 1;
						}
					}

					// Remove old point
					if (speed_points.size() > speed_window) {
						previousPoint = speed_points.front();
						speed_points.pop_front();

						if (previousPoint.hasValue(TelemetryData::DataFix)) {
							speed_sum -= previousPoint.speed();
							speed_count -= 1;
						}
					}

					if (pool_.current().hasValue(TelemetryData::DataFix)) {
						// Compute 'speed' smooth values
						if (!pool_.current().isPause()) {
							pool_.current().setSpeed(speed_sum / speed_count);

							if (pool_.current().hasValue(TelemetryData::DataMaxSpeed)) {
								maxspeed = MAX(maxspeed, pool_.current().speed());

								pool_.current().setMaxSpeed(maxspeed);
							}
						}
						// Don't update values in pause
						else
							pool_.current().setMaxSpeed(pool_.previous().maxspeed());
					}
				}
			}
			else if (speed_method == TelemetrySettings::SmoothButterworth) {
				if (pool_.tell() > 2) {
					double a = 4.0;
					double z = 0.7;

					if (pool_.current().hasValue(TelemetryData::DataFix)) {
						// Compute 'speed' smooth values
						if (!pool_.current().isPause()) {
							pool_.current().setSpeed(
									(pool_.current().speed() + (pool_.previous().speed() * (a + ((a * a) / (2 * z * z)))) \
									- (pool_.previous(1).speed() * (a * a) / (4 * z * z))) / (1 + a + ((a * a) / (4 * z * z)))
							);

							if (pool_.current().hasValue(TelemetryData::DataMaxSpeed)) {
								maxspeed = MAX(maxspeed, pool_.current().speed());

								pool_.current().setMaxSpeed(maxspeed);
							}
						}
						// Don't update values in pause
						else
							pool_.current().setMaxSpeed(pool_.previous().maxspeed());
					}
				}
			}

			// heading
			//---------

			if (heading_method == TelemetrySettings::SmoothWindowedMovingAverage) {
				if (heading_window > 1) {
					// Add new point
					nextPoint = pool_.next(heading_window/2 - 1);

					// Save point
					heading_points.emplace_back(nextPoint);

					if (nextPoint.type() != TelemetryData::TypeUnknown) {
						if (nextPoint.hasValue(TelemetryData::DataFix)) {
							heading_sum += nextPoint.heading();
							heading_count += 1;
						}
					}

					// Remove old point
					if (heading_points.size() > heading_window) {
						previousPoint = heading_points.front();
						heading_points.pop_front();

						if (previousPoint.hasValue(TelemetryData::DataFix)) {
							heading_sum -= previousPoint.heading();
							heading_count -= 1;
						}
					}

					// Compute 'heading' smooth values
					if (pool_.current().hasValue(TelemetryData::DataFix)) {
						if (!pool_.current().isPause()) {
							pool_.current().setHeading(heading_sum / heading_count);
						}
					}
				}
			}
			else if (heading_method == TelemetrySettings::SmoothButterworth) {
				if (pool_.tell() > 2) {
					double a = 4.0;
					double z = 0.7;

					if (pool_.current().hasValue(TelemetryData::DataFix)) {
						if (!pool_.current().isPause()) {
							pool_.current().setHeading(
									(pool_.current().heading() + (pool_.previous().heading() * (a + ((a * a) / (2 * z * z)))) \
									- (pool_.previous(1).heading() * (a * a) / (4 * z * z))) / (1 + a + ((a * a) / (4 * z * z)))
							);
						}
					}
				}
			}

			// elevation
			//-----------

			if (elevation_method == TelemetrySettings::SmoothWindowedMovingAverage) {
				if (elevation_window > 1) {
					// Add new point
					nextPoint = pool_.next(elevation_window/2 - 1);

					// Save point
					elevation_points.emplace_back(nextPoint);

					if (nextPoint.type() != TelemetryData::TypeUnknown) {
						if (nextPoint.hasValue(TelemetryData::DataFix)) {
							elevation_sum += nextPoint.elevation();
							elevation_count += 1;
						}
					}

					// Remove old point
					if (elevation_points.size() > elevation_window) {
						previousPoint = elevation_points.front();
						elevation_points.pop_front();

						if (previousPoint.hasValue(TelemetryData::DataFix)) {
							elevation_sum -= previousPoint.elevation();
							elevation_count -= 1;
						}
					}

					if (pool_.current().hasValue(TelemetryData::DataFix)) {
						// Compute 'elevation' smooth values
						if (!pool_.current().isPause())
							pool_.current().setElevation(elevation_sum / elevation_count);
						// Don't update values in pause
						else
							pool_.current().setElevation(pool_.previous().elevation());
					}
				}
			}
			else if (elevation_method == TelemetrySettings::SmoothButterworth) {
				if (pool_.tell() > 2) {
					double a = 4.0;
					double z = 0.7;

					if (pool_.current().hasValue(TelemetryData::DataFix)) {
						// Compute 'elevation' smooth values
						if (!pool_.current().isPause()) {
							pool_.current().setElevation(
									(pool_.current().elevation() + (pool_.previous().elevation() * (a + ((a * a) / (2 * z * z)))) \
									- (pool_.previous(1).elevation() * (a * a) / (4 * z * z))) / (1 + a + ((a * a) / (4 * z * z)))
							);
						}
					}
				}
			}

			// acceleration
			//--------------

			if (acceleration_method == TelemetrySettings::SmoothWindowedMovingAverage) {
				if (acceleration_window > 1) {
					// Add new point
					nextPoint = pool_.next(acceleration_window/2 - 1);

					// Save point
					acceleration_points.emplace_back(nextPoint);

					if (nextPoint.type() != TelemetryData::TypeUnknown) {
						if (nextPoint.hasValue(TelemetryData::DataFix)) {
							acceleration_sum += nextPoint.acceleration();
							acceleration_count +=1;
						}
					}

					// Remove old point
					if (acceleration_points.size() > acceleration_window) {
						previousPoint = acceleration_points.front();
						acceleration_points.pop_front();

						if (previousPoint.hasValue(TelemetryData::DataFix)) {
							acceleration_sum -= previousPoint.acceleration();
							acceleration_count -= 1;
						}
					}

					// Compute 'acceleration' smooth values
					if (pool_.current().hasValue(TelemetryData::DataFix)) {
						if (!pool_.current().isPause()) {
							pool_.current().setAcceleration(acceleration_sum / acceleration_count);
						}
					}
				}
			}
			else if (acceleration_method == TelemetrySettings::SmoothButterworth) {
				if (pool_.tell() > 2) {
					double a = 4.0;
					double z = 0.7;

					// Compute 'acceleration' smooth values
					if (pool_.current().hasValue(TelemetryData::DataFix)) {
						if (!pool_.current().isPause()) {
							pool_.current().setAcceleration(
									(pool_.current().acceleration() + (pool_.previous().acceleration() * (a + ((a * a) / (2 * z * z)))) \
									- (pool_.previous(1).acceleration() * (a * a) / (4 * z * z))) / (1 + a + ((a * a) / (4 * z * z)))
							);
						}
					}
				}
			}

			// slope (if elevation has changed)
			//----------------------------------

			if (elevation_method != TelemetrySettings::SmoothNone) {
				// Compute again slope with new smooth values
				if (first) {
					elevation = pool_.current().elevation();
					first = false;
				}

				dc += pool_.current().distance() - distance;
				dz += pool_.current().elevation() - elevation;

				// Distance enough (> 10 meter), compute grade
				if (!pool_.current().isPause() && (dc > 10)) {
					grade = 100.0 * dz / dc;

					if (pool_.current().hasValue(TelemetryData::DataFix))
						pool_.current().setGrade(grade);

					for (int k=0; k<=gs; k++) {
						if (pool_.previous(k).hasValue(TelemetryData::DataFix))
							pool_.previous(k).setGrade(grade);
					}

					dc = 0;
					dz = 0;

					gs = -1;
				}
				else 
					gs++;

				// Save last data
				distance = pool_.current().distance();
				elevation = pool_.current().elevation();
			}
		}

		// Move to next
		pool_.seek(1);
	}

	// Reset
	pool_.reset();
}


void TelemetrySource::smooth_step_two(void) {
	int grade_count = 0;
	int verticalspeed_count = 0;

	size_t grade_window = 0;
	size_t verticalspeed_window = 0;

	double grade_sum = 0;
	double verticalspeed_sum = 0;

	TelemetrySettings::Smooth grade_method;
	TelemetrySettings::Smooth verticalspeed_method;

	std::deque<Point> grade_points;
	std::deque<Point> verticalspeed_points;

	TelemetrySource::Point nextPoint, previousPoint;

	log_call();

	grade_method = settings().telemetrySmoothMethod(TelemetryData::DataGrade);
	verticalspeed_method = settings().telemetrySmoothMethod(TelemetryData::DataVerticalSpeed);

	grade_window = (settings().telemetrySmoothMethod(TelemetryData::DataGrade) == TelemetrySettings::SmoothWindowedMovingAverage) ? 
		(settings().telemetrySmoothPoints(TelemetryData::DataGrade) * 2) + 1 : 0;
	verticalspeed_window = (settings().telemetrySmoothMethod(TelemetryData::DataVerticalSpeed) == TelemetrySettings::SmoothWindowedMovingAverage) 
		? (settings().telemetrySmoothPoints(TelemetryData::DataVerticalSpeed) * 2) + 1 : 0;

	// Fill 'grade' window
	for (size_t i=0; i<grade_window/2; i++) {
		nextPoint = pool_.next(i);

		if (nextPoint.type() == TelemetryData::TypeUnknown)
			break;

		// Save point
		grade_points.emplace_back(nextPoint);

		if (nextPoint.hasValue(TelemetryData::DataFix)) {
			grade_sum += nextPoint.grade();
			grade_count += 1;
		}
	}

	// Fill 'verticalspeed' window
	for (size_t i=0; i<verticalspeed_window/2; i++) {
		nextPoint = pool_.next(i);

		if (nextPoint.type() == TelemetryData::TypeUnknown)
			break;

		// Save point
		verticalspeed_points.emplace_back(nextPoint);

		if (nextPoint.hasValue(TelemetryData::DataFix)) {
			verticalspeed_sum += nextPoint.verticalspeed();
			verticalspeed_count += 1;
		}
	}

	// Move to first point
	pool_.seek(1);

	// Compute average value for each point
	while (!pool_.empty()) {
		if (pool_.current().type() != TelemetryData::TypeError) {
			// grade
			//-------

			if (grade_method == TelemetrySettings::SmoothWindowedMovingAverage) {
				if (grade_window > 1) {
					// Add new point
					nextPoint = pool_.next(grade_window/2 - 1);

					// Save point
					grade_points.emplace_back(nextPoint);

					if (nextPoint.type() != TelemetryData::TypeUnknown) {
						if (nextPoint.hasValue(TelemetryData::DataFix)) {
							grade_sum += nextPoint.grade();
							grade_count += 1;
						}
					}

					// Remove old point
					if (grade_points.size() > grade_window) {
						previousPoint = grade_points.front();
						grade_points.pop_front();

						if (previousPoint.hasValue(TelemetryData::DataFix)) {
							grade_sum -= previousPoint.grade();
							grade_count -= 1;
						}
					}

					if (pool_.current().hasValue(TelemetryData::DataFix)) {
						// Compute smooth values: grade
						if (!pool_.current().isPause()) {
							pool_.current().setGrade(grade_sum / grade_count);
						}
						// Don't update values in pause
						else {
							pool_.current().setGrade(pool_.previous().grade());
						}
					}
				}
			}
			else if (grade_method == TelemetrySettings::SmoothButterworth) {
				if (pool_.tell() > 2) {
					double a = 4.0;
					double z = 0.7;

					if (pool_.current().hasValue(TelemetryData::DataFix)) {
						// Compute smooth values: grade
						if (!pool_.current().isPause()) {
							pool_.current().setGrade(
									(pool_.current().grade() + (pool_.previous().grade() * (a + ((a * a) / (2 * z * z)))) \
									- (pool_.previous(1).grade() * (a * a) / (4 * z * z))) / (1 + a + ((a * a) / (4 * z * z)))
							);
						}
						// Don't update values in pause
						else {
							pool_.current().setGrade(pool_.previous().grade());
						}
					}
				}
			}

			// verticalspeed
			//--------------

			if (verticalspeed_method == TelemetrySettings::SmoothWindowedMovingAverage) {
				if (verticalspeed_window > 1) {
					// Add new point
					nextPoint = pool_.next(verticalspeed_window/2 - 1);

					// Save point
					verticalspeed_points.emplace_back(nextPoint);

					if (nextPoint.type() != TelemetryData::TypeUnknown) {
						if (nextPoint.hasValue(TelemetryData::DataFix)) {
							verticalspeed_sum += nextPoint.verticalspeed();
							verticalspeed_count += 1;
						}
					}

					// Remove old point
					if (verticalspeed_points.size() > verticalspeed_window) {
						previousPoint = verticalspeed_points.front();
						verticalspeed_points.pop_front();

						if (previousPoint.hasValue(TelemetryData::DataFix)) {
							verticalspeed_sum -= previousPoint.verticalspeed();
							verticalspeed_count -= 1;
						}
					}

					// Compute 'verticalspeed' smooth values
					if (pool_.current().hasValue(TelemetryData::DataFix)) {
						if (!pool_.current().isPause()) {
							pool_.current().setVerticalSpeed(verticalspeed_sum / verticalspeed_count);
						}
					}
				}
			}
			else if (verticalspeed_method == TelemetrySettings::SmoothButterworth) {
				if (pool_.tell() > 2) {
					double a = 4.0;
					double z = 0.7;

					if (pool_.current().hasValue(TelemetryData::DataFix)) {
						if (!pool_.current().isPause()) {
							pool_.current().setVerticalSpeed(
									(pool_.current().verticalspeed() + (pool_.previous().verticalspeed() * (a + ((a * a) / (2 * z * z)))) \
									- (pool_.previous(1).verticalspeed() * (a * a) / (4 * z * z))) / (1 + a + ((a * a) / (4 * z * z)))
							);
						}
					}
				}
			}
		}

		// Move to next
		pool_.seek(1);
	}

	// Reset
	pool_.reset();
}


void TelemetrySource::smooth(void) {
	size_t grade_window = (settings().telemetrySmoothPoints(TelemetryData::DataGrade) * 2) + 1;
	size_t speed_window = (settings().telemetrySmoothPoints(TelemetryData::DataSpeed) * 2) + 1;
	size_t heading_window = (settings().telemetrySmoothPoints(TelemetryData::DataHeading) * 2) + 1;
	size_t elevation_window = (settings().telemetrySmoothPoints(TelemetryData::DataElevation) * 2) + 1;
	size_t acceleration_window = (settings().telemetrySmoothPoints(TelemetryData::DataAcceleration) * 2) + 1;
	size_t verticalspeed_window = (settings().telemetrySmoothPoints(TelemetryData::DataVerticalSpeed) * 2) + 1;

	log_call();

	if (!quiet_) {
		// 1/
		printf("%s: Smooth telemetry data using 'windowed moving average' filter\n", name().c_str());

		if ((settings().telemetrySmoothMethod(TelemetryData::DataGrade) == TelemetrySettings::SmoothWindowedMovingAverage) && (grade_window > 1))
			printf("     - grade: window size '%ld' (+/- %d points)\n", 
					grade_window, settings().telemetrySmoothPoints(TelemetryData::DataGrade));
		else
			printf("     - grade: no\n");

		if ((settings().telemetrySmoothMethod(TelemetryData::DataSpeed) == TelemetrySettings::SmoothWindowedMovingAverage) && (speed_window > 1))
			printf("     - speed: window size '%ld' (+/- %d points)\n", 
					speed_window, settings().telemetrySmoothPoints(TelemetryData::DataSpeed));
		else
			printf("     - speed: no\n");

		if ((settings().telemetrySmoothMethod(TelemetryData::DataHeading) == TelemetrySettings::SmoothWindowedMovingAverage) && (heading_window > 1))
			printf("     - heading: window size '%ld' (+/- %d points)\n", 
					heading_window, settings().telemetrySmoothPoints(TelemetryData::DataHeading));
		else
			printf("     - heading: no\n");

		if ((settings().telemetrySmoothMethod(TelemetryData::DataElevation) == TelemetrySettings::SmoothWindowedMovingAverage) && (elevation_window > 1))
			printf("     - elevation: window size '%ld' (+/- %d points)\n", 
					elevation_window, settings().telemetrySmoothPoints(TelemetryData::DataElevation));
		else
			printf("     - elevation: no\n");

		if ((settings().telemetrySmoothMethod(TelemetryData::DataAcceleration) == TelemetrySettings::SmoothWindowedMovingAverage) && (acceleration_window > 1))
			printf("     - acceleration: window size '%ld' (+/- %d points)\n", 
					acceleration_window, settings().telemetrySmoothPoints(TelemetryData::DataAcceleration));
		else
			printf("     - acceleration: no\n");

		if ((settings().telemetrySmoothMethod(TelemetryData::DataVerticalSpeed) == TelemetrySettings::SmoothWindowedMovingAverage) && (verticalspeed_window > 1))
			printf("     - verticalspeed: window size '%ld' (+/- %d points)\n", 
					verticalspeed_window, settings().telemetrySmoothPoints(TelemetryData::DataVerticalSpeed));
		else
			printf("     - verticalspeed: no\n");

		// 2/
		printf("%s: Smooth telemetry data using 'butterworth' filter\n", name().c_str());

		if (settings().telemetrySmoothMethod(TelemetryData::DataGrade) == TelemetrySettings::SmoothButterworth)
			printf("     - grade: a = 4.0 / z = 0.7\n");
		else
			printf("     - grade: no\n");

		if (settings().telemetrySmoothMethod(TelemetryData::DataSpeed) == TelemetrySettings::SmoothButterworth)
			printf("     - speed: a = 4.0 / z = 0.7\n");
		else
			printf("     - speed: no\n");

		if (settings().telemetrySmoothMethod(TelemetryData::DataHeading) == TelemetrySettings::SmoothButterworth)
			printf("     - heading: a = 4.0 / z = 0.7\n");
		else
			printf("     - heading: no\n");

		if (settings().telemetrySmoothMethod(TelemetryData::DataElevation) == TelemetrySettings::SmoothButterworth)
			printf("     - elevation: a = 4.0 / z = 0.7\n");
		else
			printf("     - elevation: no\n");

		if (settings().telemetrySmoothMethod(TelemetryData::DataAcceleration) == TelemetrySettings::SmoothButterworth)
			printf("     - acceleration: a = 4.0 / z = 0.7\n");
		else
			printf("     - acceleration: no\n");

		if (settings().telemetrySmoothMethod(TelemetryData::DataVerticalSpeed) == TelemetrySettings::SmoothButterworth)
			printf("     - verticalspeed: a = 4.0 / z = 0.7\n");
		else
			printf("     - verticalspeed: no\n");
	}

	smooth_step_one();
	smooth_step_two();
}


/**
 * Fix bad point by interpolation:
 *  - speed, avgspeed, avgridespeed
 *  - acceleration
 *  - distance
 *  - elevation
 * So compute:
 *  - grade
 *  - heading
 *  - maxspeed
 *  - verticalspeed
 */
void TelemetrySource::fix(void) {
	TelemetrySource::Point currentPoint;
	TelemetrySource::Point prevPoint, nextPoint;

	// Move to first point
	pool_.seek(1);

	prevPoint = pool_.current();
	pool_.seek(1);

	// Process each point
	for (; !pool_.empty(); pool_.seek(1)) {
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

			// Compute for each point
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
//				double distance = currentPoint.distanceTo(prevPoint);
//				double grade = ((distance > 0) && (currentPoint.speed() > 0)) ? 100.0 * (currentPoint.elevation() - prevPoint.ele_) / distance : prevPoint.grade();
				double grade = prevPoint.grade_ + k * (nextPoint.grade_ - prevPoint.grade_);
				currentPoint.setGrade(grade);

//				// Upgrade grade for next point
//				if (!nextPoint.isPause())
//					nextPoint.setGrade(grade);
			}

			if (currentPoint.hasValue(TelemetryData::DataHeading)) {
				double heading = prevPoint.heading_ + k * (nextPoint.heading_ - prevPoint.heading_);
				currentPoint.setHeading(heading);
			}

			if (currentPoint.hasValue(TelemetryData::DataAcceleration)) {
				double acceleration = prevPoint.acceleration_ + k * (nextPoint.acceleration_ - prevPoint.acceleration_);
				currentPoint.setAcceleration(acceleration);
			}

			if (currentPoint.hasValue(TelemetryData::DataVerticalSpeed)) {
				double verticalspeed = prevPoint.verticalspeed_ + k * (nextPoint.verticalspeed_ - prevPoint.verticalspeed_);
				currentPoint.setVerticalSpeed(verticalspeed);
			}

			if (currentPoint.hasValue(TelemetryData::DataHomeDistance)) {
				double distance = prevPoint.homedistance_ + k * (nextPoint.homedistance_ - prevPoint.homedistance_);
				currentPoint.setHomeDistance(distance);
			}

			// Compute if we are in the range defined by the user
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
	std::cout << "  offset:               " << offset_ << std::endl;
	std::cout << "  skip bad point:       " << (check_ ? "true" : "false") << std::endl;
	std::cout << "  pause detection:      " << (pause_detection_ ? "true" : "false") << std::endl;
	std::cout << "  begin data range:     " << Datetime::timestamp2string(begin_, Datetime::FormatDatetime, false).c_str() << " (timestamp: " << begin_ << ")" << std::endl;
	std::cout << "  end data range:       " << Datetime::timestamp2string(end_, Datetime::FormatDatetime, false).c_str() << " (timestamp: " << end_ << ")" << std::endl;
	std::cout << "  from compute range:   " << Datetime::timestamp2string(compute_start_, Datetime::FormatDatetime, false).c_str() << " (timestamp: " << compute_start_ << ")" << std::endl;
	std::cout << "  to compute range:     " << Datetime::timestamp2string(compute_stop_, Datetime::FormatDatetime, false).c_str() << " (timestamp: " << compute_stop_ << ")" << std::endl;
	std::cout << "  from view range:      " << Datetime::timestamp2string(view_start_, Datetime::FormatDatetime, false).c_str() << " (timestamp: " << view_start_ << ")" << std::endl;
	std::cout << "  to view range:        " << Datetime::timestamp2string(view_stop_, Datetime::FormatDatetime, false).c_str() << " (timestamp: " << view_stop_ << ")" << std::endl;
	std::cout << "  interpolation method: " << method_ << " (rate: " << settings().telemetryRate() << ")" << std::endl;

	pool_.dump(content);
}


bool TelemetrySource::getBoundingBox(TelemetrySource::Range range, TelemetryData *p1, TelemetryData *p2) {
	TelemetryData data;

	enum TelemetrySource::Data type = TelemetrySource::DataUnknown;

	log_call();

	// Get & check each point
	for (type = retrieveFirst(data); type != TelemetrySource::DataEof; type = retrieveNext(data)) {
		if (!data.hasValue(TelemetryData::DataFix))
			continue;

		if ((range == TelemetrySource::RangeData) && (data.timestamp() < begin_))
			continue;
		if ((range == TelemetrySource::RangeCompute) && (data.timestamp() < compute_start_))
			continue;
		if ((range == TelemetrySource::RangeView) && (data.timestamp() < view_start_))
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

		if ((range == TelemetrySource::RangeData) && (data.timestamp() > end_))
			break;
		if ((range == TelemetrySource::RangeCompute) && (data.timestamp() > compute_stop_))
			break;
		if ((range == TelemetrySource::RangeView) && (data.timestamp() > view_stop_))
			break;
	}

	return (p1->hasValue(TelemetryData::DataFix) && p2->hasValue(TelemetryData::DataFix));
}


bool TelemetrySource::isOpen(void) const {
	return stream_.is_open();
}


enum TelemetrySource::Data TelemetrySource::loadData(void) {
	enum TelemetrySource::Data type = TelemetrySource::DataAgain;

	log_call();

	reset();
	clear();
	config();
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
	point.setBatteryLevel(data.batterylevel());

	// Set values flag
	flags = data.has_value_ & (
			TelemetryData::DataFix
			| TelemetryData::DataElevation
			| TelemetryData::DataPower
			| TelemetryData::DataCadence
			| TelemetryData::DataHeartrate
			| TelemetryData::DataTemperature
			| TelemetryData::DataBatteryLevel);
	point.setValue(flags);

	// Insert point
	pool_.insert(point);
}


void TelemetrySource::predictData(TelemetryData &data, TelemetrySettings::Method method, uint64_t timestamp) {
	uint64_t offset;

	double k;

	bool in_range = true;

	TelemetrySource::Point point;

	TelemetrySource::Point firstPoint;
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

		if (prevPoint.hasValue(TelemetryData::DataBatteryLevel) && nextPoint.hasValue(TelemetryData::DataBatteryLevel))
			point.setBatteryLevel(prevPoint.batterylevel_ + k * (nextPoint.batterylevel_ - prevPoint.batterylevel_));

		// Computed data
		if (prevPoint.hasValue(TelemetryData::DataSpeed) && nextPoint.hasValue(TelemetryData::DataSpeed))
			point.setSpeed(prevPoint.speed_ + k * (nextPoint.speed_ - prevPoint.speed_));

		if (prevPoint.hasValue(TelemetryData::DataVerticalSpeed) && nextPoint.hasValue(TelemetryData::DataVerticalSpeed))
			point.setVerticalSpeed(prevPoint.verticalspeed_ + k * (nextPoint.verticalspeed_ - prevPoint.verticalspeed_));

		if (prevPoint.hasValue(TelemetryData::DataAcceleration) && nextPoint.hasValue(TelemetryData::DataAcceleration))
			point.setAcceleration(prevPoint.acceleration_ + k * (nextPoint.acceleration_ - prevPoint.acceleration_));

		if (prevPoint.hasValue(TelemetryData::DataGrade) && nextPoint.hasValue(TelemetryData::DataGrade))
			point.setGrade(prevPoint.grade_ + k * (nextPoint.grade_ - prevPoint.grade_));

		if (prevPoint.hasValue(TelemetryData::DataHeading) && nextPoint.hasValue(TelemetryData::DataHeading))
			point.setHeading(prevPoint.heading_ + k * (nextPoint.heading_ - prevPoint.heading_));

		if (prevPoint.hasValue(TelemetryData::DataHomeDistance) && nextPoint.hasValue(TelemetryData::DataHomeDistance))
			point.setHomeDistance(prevPoint.homedistance_ + k * (nextPoint.homedistance_ - prevPoint.homedistance_));

		// Computed data in range
		if (prevPoint.hasValue(TelemetryData::DataDistance) && nextPoint.hasValue(TelemetryData::DataDistance)) {
			double distance = in_range ? prevPoint.distance_ + k * (nextPoint.distance_ - prevPoint.distance_) : prevPoint.distance_;
			point.setDistance(distance);
		}

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

		if (prevPoint.hasValue(TelemetryData::DataBatteryLevel) && curPoint.hasValue(TelemetryData::DataBatteryLevel))
			point.setBatteryLevel(curPoint.batterylevel_ + k * (curPoint.batterylevel_ - prevPoint.batterylevel_));

		// Computed data
		if (prevPoint.hasValue(TelemetryData::DataSpeed) && curPoint.hasValue(TelemetryData::DataSpeed))
			point.setSpeed(curPoint.speed_ + k * (curPoint.speed_ - prevPoint.speed_));

		if (prevPoint.hasValue(TelemetryData::DataVerticalSpeed) && curPoint.hasValue(TelemetryData::DataVerticalSpeed))
			point.setVerticalSpeed(curPoint.verticalspeed_ + k * (curPoint.verticalspeed_ - prevPoint.verticalspeed_));

		if (prevPoint.hasValue(TelemetryData::DataAcceleration) && curPoint.hasValue(TelemetryData::DataAcceleration))
			point.setAcceleration(curPoint.acceleration_ + k * (curPoint.acceleration_ - prevPoint.acceleration_));

		if (prevPoint.hasValue(TelemetryData::DataGrade) && curPoint.hasValue(TelemetryData::DataGrade))
			point.setGrade(curPoint.grade_ + k * (curPoint.grade_ - prevPoint.grade_));

		if (prevPoint.hasValue(TelemetryData::DataHeading) && curPoint.hasValue(TelemetryData::DataHeading))
			point.setHeading(curPoint.heading_ + k * (curPoint.heading_ - prevPoint.heading_));

		if (prevPoint.hasValue(TelemetryData::DataHomeDistance) && curPoint.hasValue(TelemetryData::DataHomeDistance))
			point.setHomeDistance(curPoint.homedistance_ + k * (curPoint.homedistance_ - prevPoint.homedistance_));

		// Computed data in range
		if (prevPoint.hasValue(TelemetryData::DataDistance) && curPoint.hasValue(TelemetryData::DataDistance)) {
			double distance = in_range ? curPoint.distance_ + k * (curPoint.distance_ - prevPoint.distance_) : curPoint.distance_;
			point.setDistance(distance);
		}

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
			| TelemetryData::DataBatteryLevel

			| TelemetryData::DataSpeed
			| TelemetryData::DataGrade
			| TelemetryData::DataHeading
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
	data.setIndex(pool_.tell());

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

	if (compute_start_ != 0) {
		result = retrieveNext(data, compute_start_);
	}

	return result;
}


enum TelemetrySource::Data TelemetrySource::retrieveNext(TelemetryData &data, uint64_t timestamp) {
	TelemetrySource::Point nextPoint;

	TelemetrySettings::Method method = settings().telemetryMethod();

	enum TelemetrySource::Data type = TelemetrySource::DataUnknown;

	log_call();

	// Restore pool context
	pool_.seek(data.index(), SEEK_SET);

	// Last point
	nextPoint = pool_.next();

//	printf(" <ts %lu> ", timestamp);

	// By default no change
	data.type_ = TelemetryData::TypeUnchanged;

	// Read next points if need
	do {
		if (timestamp == (uint64_t) -1) {
			if (this->retrieveData(data) == TelemetrySource::DataEof)
				goto eof;
		}
		else {
			if (timestamp <= data.timestamp()) {
//				printf(" <unchanged> ");
			}
			else if (timestamp < nextPoint.timestamp()) {
//				printf(" <predict %lu %lu> ", timestamp, nextPoint.timestamp());
				predictData(data, method, timestamp);
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

	if (compute_stop_ != 0) {
		result = retrieveFirst(data);

		result = retrieveNext(data, compute_stop_);
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

	// Source readable
	if ((source != NULL) && !source->isOpen()) {
		delete source;
		source = NULL;
	}

	// Init
	if (source != NULL) {
		bool ok = true;

		// Verbose info
		source->setQuiet(quiet);

		// Telemetry settings
		source->setSettings(settings);

		// Load telemetry data
		source->loadData();

		if (!ok) {
			delete source;

			source = NULL;
		}
	}

	return source;
}

