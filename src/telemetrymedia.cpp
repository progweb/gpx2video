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
#include "telemetry/tcx.h"
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


int TelemetryData::line(void) const {
	return line_;
}


int TelemetryData::index(void) const {
	return index_;
}


void TelemetryData::setIndex(int index) {
	index_ = index;
}


const TelemetryData::Type& TelemetryData::type(void) const {
	return type_;
}


const char * TelemetryData::type2string(void) const {
	const char *types[] = {
		"U", // Unknown
		"D", // Dummy
		"M", // Measured
		"F", // Fix
		"P", // Predict
		"C", // Unchanged
		"E", // Error
	};

	if (type_ > ARRAY_SIZE(types))
		return "";

	return types[type_];
}


const uint64_t& TelemetryData::datetime(void) const {
	return datetime_;
}


void TelemetryData::setDatetime(const uint64_t &datetime) {
	if (type_ == TypeUnknown)
		type_ = TypeDummy;

	datetime_ = datetime;
}


const uint64_t& TelemetryData::timestamp(void) const {
	return ts_;
}


const double& TelemetryData::latitude(bool raw) const {
	return raw ? raw_lat_ : lat_;
}


const double& TelemetryData::longitude(bool raw) const {
	return raw ? raw_lon_ : lon_;
}


double TelemetryData::elevation(TelemetryData::Unit unit, TelemetryData::Range range) const {
	double ele;

	switch (range) {
	case TelemetryData::RangeMin:
		ele = ele_min_;
		break;
	case TelemetryData::RangeMax:
		ele = ele_max_;
		break;
	case TelemetryData::RangeNone:
	default:
		ele = ele_;
		break;
	}

	switch (unit) {
	case TelemetryData::UnitFeet:
		return ele * 3.28084;

	case TelemetryData::UnitMiles:
		return (ele * 0.6213711922) / 1000.0;

	case TelemetryData::UnitMeter:
	case TelemetryData::UnitDefault:
	default:
		return ele;
	}
}


int TelemetryData::cadence(TelemetryData::Unit unit) const {
	(void) unit;

	// tr/min

	return cadence_;
}


int TelemetryData::heartrate(TelemetryData::Unit unit) const {
	(void) unit;

	// bpm

	return heartrate_;
}


double TelemetryData::temperature(TelemetryData::Unit unit) const {
	switch (unit) {
	case TelemetryData::UnitFarenheit:
		return (temperature_ * 9.0 / 5.0) + 32.0;

	case TelemetryData::UnitCelsius:
	case TelemetryData::UnitDefault:
	default:
		return temperature_;
	}
}


int TelemetryData::power(TelemetryData::Unit unit) const {
	(void) unit;

	// watt

	return power_;
}


double TelemetryData::duration(TelemetryData::Unit unit) const {
	(void) unit;

	// seconds - duration in the compute range

	return duration_;
}


double TelemetryData::distance(TelemetryData::Unit unit, TelemetryData::Range range) const {
	double distance;

	switch (range) {
	case TelemetryData::RangeMin:
		distance = distance_min_;
		break;
	case TelemetryData::RangeMax:
		distance = distance_max_;
		break;
	case TelemetryData::RangeNone:
	default:
		distance = distance_;
		break;
	}

	switch (unit) {
	case TelemetryData::UnitFeet:
		return distance * 3.28084;

	case TelemetryData::UnitMiles:
		return (distance * 0.6213711922) / 1000.0;

	case TelemetryData::UnitKm:
		return distance / 1000.0;

	case TelemetryData::UnitMeter:
	case TelemetryData::UnitDefault:
	default:
		return distance;
	}
}


double TelemetryData::course(TelemetryData::Unit unit) const {
	(void) unit;

	return course_;
}


double TelemetryData::heading(TelemetryData::Unit unit) const {
	(void) unit;

	return heading_;
}


double TelemetryData::grade(void) const {
	return grade_;
}


double TelemetryData::speed(TelemetryData::Unit unit) const {
	switch (unit) {
	case TelemetryData::UnitMilesPerHour:
		return speed_ * 0.6213711922;

	case TelemetryData::UnitMinPerMile:
		return 60.0 / (speed_ * 0.6213711922);

	case TelemetryData::UnitMinPerKm:
		return 60.0 / speed_;

	case TelemetryData::UnitMeterPerHour:
		return speed_ * 1000.0;

	case TelemetryData::UnitKmPerHour:
	case TelemetryData::UnitDefault:
	default:
		return speed_;
	}
}


double TelemetryData::maxspeed(TelemetryData::Unit unit) const {
	switch (unit) {
	case TelemetryData::UnitMilesPerHour:
		return maxspeed_ * 0.6213711922;

	case TelemetryData::UnitMinPerMile:
		return 60.0 / (maxspeed_ * 0.6213711922);

	case TelemetryData::UnitMinPerKm:
		return 60.0 / maxspeed_;

	case TelemetryData::UnitMeterPerHour:
		return maxspeed_ * 1000.0;

	case TelemetryData::UnitKmPerHour:
	case TelemetryData::UnitDefault:
	default:
		return maxspeed_;
	}
}


double TelemetryData::acceleration(TelemetryData::Unit unit) const {
	// acceleration m/s²
	// gforce g (= 9,80665 m/s²)

	switch (unit) {
	case TelemetryData::UnitG:
		return acceleration_ / 9.81;

	case TelemetryData::UnitMeterPerSec2:
	case TelemetryData::UnitDefault:
	default:
		return acceleration_;
	}
}


double TelemetryData::rideTime(TelemetryData::Unit unit) const {
	(void) unit;

	// seconds - ride duration as speed > 4.0 in the compute range

	return ridetime_;
}


double TelemetryData::elapsedTime(TelemetryData::Unit unit) const {
	(void) unit;

	// seconds - absolute duration during whole activity

	return elapsedtime_;
}


double TelemetryData::avgspeed(TelemetryData::Unit unit) const {
	switch (unit) {
	case TelemetryData::UnitMilesPerHour:
		return avgspeed_ * 0.6213711922;

	case TelemetryData::UnitMinPerMile:
		return 60.0 / (avgspeed_ * 0.6213711922);

	case TelemetryData::UnitMinPerKm:
		return 60.0 / avgspeed_;

	case TelemetryData::UnitMeterPerHour:
		return avgspeed_ * 1000.0;

	case TelemetryData::UnitKmPerHour:
	case TelemetryData::UnitDefault:
	default:
		return avgspeed_;
	}
}


double TelemetryData::avgridespeed(TelemetryData::Unit unit) const {
	switch (unit) {
	case TelemetryData::UnitMilesPerHour:
		return avgridespeed_ * 0.6213711922;

	case TelemetryData::UnitMinPerMile:
		return 60.0 / (avgridespeed_ * 0.6213711922);

	case TelemetryData::UnitMinPerKm:
		return 60.0 / avgridespeed_;

	case TelemetryData::UnitMeterPerHour:
		return avgridespeed_ * 1000.0;

	case TelemetryData::UnitKmPerHour:
	case TelemetryData::UnitDefault:
	default:
		return avgridespeed_;
	}
}


double TelemetryData::verticalspeed(TelemetryData::Unit unit) const {
	switch (unit) {
	case TelemetryData::UnitMilesPerSec:
		return (verticalspeed_ * 0.6213711922) / 1000.0;

	case TelemetryData::UnitFeetPerSec:
		return verticalspeed_ * 3.28084;

	case TelemetryData::UnitMeterPerSec:
	case TelemetryData::UnitDefault:
	default:
		return verticalspeed_;
	}
}


double TelemetryData::homedistance(TelemetryData::Unit unit) const {
	switch (unit) {
	case TelemetryData::UnitFeet:
		return homedistance_ * 3.28084;

	case TelemetryData::UnitMiles:
		return (homedistance_ * 0.6213711922) / 1000.0;

	case TelemetryData::UnitKm:
		return homedistance_ / 1000.0;

	case TelemetryData::UnitMeter:
	case TelemetryData::UnitDefault:
	default:
		return homedistance_;
	}
}


double TelemetryData::batterylevel(void) const {
	return batterylevel_;
}


int TelemetryData::lap(void) const {
	return lap_;
}


bool TelemetryData::inRange(void) const {
	return in_range_;
}


bool TelemetryData::isPause(void) const {
	return is_pause_;
}


bool TelemetryData::hasValue(Data type) const {
	return ((has_value_ & type) != 0);
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
	printf("      | Line  | T | Datetime (ms)           | Duration | Distance    | Speed  | Acceleration | S | Latitude     | Longitude    | Altitude | Cap. | Grade   \n");
	printf("------+-------+---+-------------------------+----------+-------------+--------+--------------+---+--------------+--------------+----------+------+---------\n");
}


void TelemetryData::writeData(size_t index) const {
	printf("%5ld | %5d | %s | %s | %8d | %11.3f | %6.1f | %12.8f | %1s | %12.8f | %12.8f | %8.1f | %3d° | %5.1f%%\n",
		index,
		line_,
		type2string(),
		Datetime::timestamp2string(ts_).c_str(),
		(int) round(duration_), distance_/1000.0, speed_, acceleration_ / 9.81,
		is_pause_ ? "S" : " ",
		lat_, lon_,
		ele_, (int) course_, grade_);
}


/**
 * TelemetrySource::Point
 */

const double TelemetrySource::Point::projection_ = 2 * 6378137.0 * M_PI / 2.0;


double TelemetrySource::Point::courseTo(const Point &to) {
	double azi1, azi2;

	GeographicLib::Geodesic gsic(6378137.0, 1.0/298.2572);

	// Maths
	gsic.Inverse(lat_, lon_, to.lat_, to.lon_, azi1, azi2);

	if (azi1 < 0)
		azi1 += 360.0;

	return azi1; //1 + 180.0;
}


double TelemetrySource::Point::distanceTo(const Point &to) {
	double d = 0.0;

	GeographicLib::Geodesic gsic(6378137.0, 1.0/298.2572);

	// Maths
	gsic.Inverse(lat_, lon_, to.lat_, to.lon_, d);

	return d;
}


void TelemetrySource::Point::project(void) {
	x_ = projection_ * lon_ / 180.0;
	y_ = log(tan((lat_ + 90.0 ) * M_PI / 360.0)) / (M_PI / 180.0) * projection_ / 180.0;
}


void TelemetrySource::Point::unproject(void) {
	double lat = (y_ / projection_) * 180.0;

	lat_ = 180.0 / M_PI * (2 * atan(exp(lat * M_PI / 180.0)) - M_PI / 2.0);
	lon_ = (x_ / projection_) * 180.0;
}


/**
 * TelemetrySource::SavitzkyGolay class
 */

std::vector<double> TelemetrySource::SavitzkyGolay::coefficients(int window_size, int poly_order, int deriv_order) {
	std::vector<double> null(0);

	if ((window_size == 0) || (poly_order == 0))
		return null;

    if (window_size % 2 == 0) {
		log_error("Window size must be odd");
		return null;
	}

    int m = window_size / 2;
    if (poly_order > m) {
        log_error("Poly order too large");
		return null;
	}

    int n = window_size;
    int p = poly_order;

    // Build design matrix A (n × (p+1))
    std::vector<double> A(n * (p + 1));
    for (int i = 0; i < n; ++i) {
        int t = i - m;
        double v = 1.0;
        for (int k = 0; k <= p; ++k) {
            A[i*(p+1) + k] = v;
            v *= t;
        }
    }

    // Compute ATA = AᵀA
    std::vector<double> ATA((p+1)*(p+1), 0.0);
    for (int i = 0; i <= p; ++i)
        for (int j = 0; j <= p; ++j)
            for (int r = 0; r < n; ++r)
                ATA[i*(p+1)+j] += A[r*(p+1)+i] * A[r*(p+1)+j];

    // Invert ATA (Gauss–Jordan)
    int dim = p + 1;
    std::vector<double> inv(dim * dim, 0.0);
    for (int i = 0; i < dim; ++i)
        inv[i*dim + i] = 1.0;

    for (int c = 0; c < dim; ++c) {
        double pivot = ATA[c*dim + c];
        double ip = 1.0 / pivot;

        for (int j = 0; j < dim; ++j) {
            ATA[c*dim + j] *= ip;
            inv[c*dim + j] *= ip;
        }

        for (int r = 0; r < dim; ++r) {
            if (r == c) continue;
            double f = ATA[r*dim + c];
            for (int j = 0; j < dim; ++j) {
                ATA[r*dim + j] -= f * ATA[c*dim + j];
                inv[r*dim + j] -= f * inv[c*dim + j];
            }
        }
    }

    // Derivative selector vector d
    std::vector<double> d(dim, 0.0);
    for (int k = deriv_order; k <= p; ++k) {
        double v = 1.0;
        for (int r = k - deriv_order + 1; r <= k; ++r)
            v *= r;
        d[k] = v;
    }

    // tmp = inv * d
    std::vector<double> tmp(dim, 0.0);
    for (int i = 0; i < dim; ++i)
        for (int j = 0; j < dim; ++j)
            tmp[i] += inv[i*dim + j] * d[j];

    // coeffs = A * tmp
    std::vector<double> coeffs(n, 0.0);
    for (int i = 0; i < n; ++i)
        for (int k = 0; k < dim; ++k)
            coeffs[i] += A[i*(p+1) + k] * tmp[k];

    return coeffs;
}


/**
 * TelemetrySource class
 */

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
	free_filter(kalman_);
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
	double course = 0;
	double ridetime = 0;
	double elapsedtime = 0;
	double speed = 0;
	double maxspeed = 0;
	double acceleration = 0;
	double verticalspeed = 0;
	double homedistance = 0;

	GeographicLib::Math::real c, d;

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
		c = prevPoint.courseTo(curPoint);
		d = prevPoint.distanceTo(curPoint);

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

		// Course
		if (force || !curPoint.hasValue(TelemetryData::DataCourse))
			course = c;
		else
			course = curPoint.course();

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

		// Course
		curPoint.setCourse(course);

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
			curPoint.setCourse(pool_.next().course());
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
		printf("%s: %lu skip points\n", name().c_str(), count);

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
		printf("%s: Telemetry available data range from '%s' to '%s'\n", 
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
			printf("%s: Filter telemetry data in using 'Iglewicz & Hoaglin' method.\n", name().c_str());

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
			printf("%s: %lu outliers detected\n", name().c_str(), n);
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
	double scourse = 0.0;

	double grade = 0.0;
	double distance = 0.0;
	double elevation = 0.0;

	uint64_t interval = 4000; // 4s

	uint64_t timestamp = -1;

	TelemetryData data;

	if (!quiet_) {
		printf("%s: Compute telemetry data from '%s' to '%s'\n", 
				name().c_str(),
				Datetime::timestamp2string(compute_start_).c_str(),
				Datetime::timestamp2string(compute_stop_).c_str());
		printf("%s: Pause detection: %s\n",
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
				scourse = pool_.current().course();
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
					pool_.current().setCourse(scourse);
					pool_.current().setElevation(sele);
					pool_.current().setGrade(grade);

					for (int k=0; k<=ss-done; k++) {
						pool_.previous(k).setPause(true);

						if (pool_.previous(k).hasValue(TelemetryData::DataFix)) {
							pool_.previous(k).setSpeed(0);
							pool_.previous(k).setAcceleration(0);
							pool_.previous(k).setCourse(scourse);
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
	int course_count = 0;
	int heading_count = 0;
	int elevation_count = 0;
	int acceleration_count = 0;

	size_t speed_window = 0;
	size_t course_window = 0;
	size_t heading_window = 0;
	size_t elevation_window = 0;
	size_t acceleration_window = 0;

	size_t speed_order = 0;
	size_t elevation_order = 0;
	size_t acceleration_order = 0;

	double grade = 0;
	double distance = 0;
	double elevation = 0;
	double maxspeed = 0;

	double speed_sum = 0;
	double course_x_sum = 0;
	double course_y_sum = 0;
	double heading_x_sum = 0;
	double heading_y_sum = 0;
	double elevation_sum = 0;
	double acceleration_sum = 0;

	std::vector<double> speed_coeff;
	std::vector<double> elevation_coeff;
	std::vector<double> acceleration_coeff;

	TelemetrySettings::Smooth speed_method;
	TelemetrySettings::Smooth course_method;
	TelemetrySettings::Smooth heading_method;
	TelemetrySettings::Smooth elevation_method;
	TelemetrySettings::Smooth acceleration_method;

	std::deque<Point> speed_points;
	std::deque<Point> course_points;
	std::deque<Point> heading_points;
	std::deque<Point> elevation_points;
	std::deque<Point> acceleration_points;

	TelemetrySource::Point nextPoint, previousPoint;

	log_call();

	speed_method = settings().telemetrySmoothMethod(TelemetryData::DataSpeed);
	course_method = settings().telemetrySmoothMethod(TelemetryData::DataCourse);
	heading_method = settings().telemetrySmoothMethod(TelemetryData::DataHeading);
	elevation_method = settings().telemetrySmoothMethod(TelemetryData::DataElevation);
	acceleration_method = settings().telemetrySmoothMethod(TelemetryData::DataAcceleration);

	speed_window = (settings().telemetrySmoothPoints(TelemetryData::DataSpeed) * 2) + 1;
	course_window = (settings().telemetrySmoothPoints(TelemetryData::DataCourse) * 2) + 1;
	heading_window = (settings().telemetrySmoothPoints(TelemetryData::DataHeading) * 2) + 1;
	elevation_window = (settings().telemetrySmoothPoints(TelemetryData::DataElevation) * 2) + 1;
	acceleration_window = (settings().telemetrySmoothPoints(TelemetryData::DataAcceleration) * 2) + 1;

	speed_order = settings().telemetrySmoothOrder(TelemetryData::DataSpeed);
	elevation_order = settings().telemetrySmoothOrder(TelemetryData::DataElevation);
	acceleration_order = settings().telemetrySmoothOrder(TelemetryData::DataAcceleration);

	// Fill 'speed' window
	if ((speed_method == TelemetrySettings::SmoothWindowedMovingAverage) 
			|| (speed_method == TelemetrySettings::SmoothSavitzkyGolay)) {
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
	}

	// Fill 'course' window
	if (course_method == TelemetrySettings::SmoothWindowedMovingAverage) {
		for (size_t i=0; i<course_window/2; i++) {
			nextPoint = pool_.next(i);

			if (nextPoint.type() == TelemetryData::TypeUnknown)
				break;

			// Save point
			course_points.emplace_back(nextPoint);

			if (nextPoint.hasValue(TelemetryData::DataFix)) {
				course_x_sum += std::cos(nextPoint.course() * M_PI / 180.0);
				course_y_sum += std::sin(nextPoint.course() * M_PI / 180.0);
				course_count += 1;
			}
		}
	}

	// Fill 'heading' window
	if (heading_method == TelemetrySettings::SmoothWindowedMovingAverage) {
		for (size_t i=0; i<heading_window/2; i++) {
			nextPoint = pool_.next(i);

			if (nextPoint.type() == TelemetryData::TypeUnknown)
				break;

			// Save point
			heading_points.emplace_back(nextPoint);

			if (nextPoint.hasValue(TelemetryData::DataFix)) {
				heading_x_sum += std::cos(nextPoint.heading() * M_PI / 180.0);
				heading_y_sum += std::sin(nextPoint.heading() * M_PI / 180.0);
				heading_count += 1;
			}
		}
	}

	// Fill 'elevation' window
	if ((elevation_method == TelemetrySettings::SmoothWindowedMovingAverage) 
			|| (elevation_method == TelemetrySettings::SmoothSavitzkyGolay)) {
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
	}

	// Fill 'acceleration' window
	if ((acceleration_method == TelemetrySettings::SmoothWindowedMovingAverage) 
			|| (acceleration_method == TelemetrySettings::SmoothSavitzkyGolay)) {
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
	}

	// Compute Savitzky Golay coefficients
	if (speed_method == TelemetrySettings::SmoothSavitzkyGolay)
	   	speed_coeff = SavitzkyGolay::coefficients(speed_window, speed_order);

	// Compute Savitzky Golay coefficients
	if (elevation_method == TelemetrySettings::SmoothSavitzkyGolay)
	   	elevation_coeff = SavitzkyGolay::coefficients(elevation_window, elevation_order);

	// Compute Savitzky Golay coefficients
	if (acceleration_method == TelemetrySettings::SmoothSavitzkyGolay)
	   	acceleration_coeff = SavitzkyGolay::coefficients(acceleration_window, acceleration_order);

	// Move to first point
	pool_.seek(1);

	// Compute average value for each point
	for (int i = 0; !pool_.empty(); i++) {
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
			else if (speed_method == TelemetrySettings::SmoothSavitzkyGolay) {
				if ((speed_window > 1) && (speed_coeff.size() > 0)) {
					ssize_t n = pool_.count();
					ssize_t half = speed_window / 2;

					// Add new point
					nextPoint = pool_.next(speed_window/2 - 1);

					// Save point
					if (nextPoint.type() != TelemetryData::TypeUnknown)
						speed_points.emplace_back(nextPoint);

					// Remove old point
					if (speed_points.size() > speed_window) {
						previousPoint = speed_points.front();
						speed_points.pop_front();
					}

					if (speed_points.size() >= (size_t) half) {
						double sum = 0;

						for (ssize_t k=-half; k<=half; k++) {
							int idx = i + k;

							// Mirror padding
							if (idx < 0)
								idx = -idx;
							else if (idx >= n) 
								idx = (2*n - idx - 2) - (i - half);
							else if (i > half)
								idx -= i - half;

							sum += speed_points[idx].speed() * speed_coeff[k + half];
						}

						// Update speed
						if (pool_.current().hasValue(TelemetryData::DataFix)) {
							if (!pool_.current().isPause()) {
								pool_.current().setSpeed(sum);

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

			// course
			//--------

			if (course_method == TelemetrySettings::SmoothWindowedMovingAverage) {
				if (course_window > 1) {
					// Add new point
					nextPoint = pool_.next(course_window/2 - 1);

					// Save point
					course_points.emplace_back(nextPoint);

					if (nextPoint.type() != TelemetryData::TypeUnknown) {
						if (nextPoint.hasValue(TelemetryData::DataFix)) {
							course_x_sum += std::cos(nextPoint.course() * M_PI / 180.0);
							course_y_sum += std::sin(nextPoint.course() * M_PI / 180.0);
							course_count += 1;
						}
					}

					// Remove old point
					if (course_points.size() > course_window) {
						previousPoint = course_points.front();
						course_points.pop_front();

						if (previousPoint.hasValue(TelemetryData::DataFix)) {
							course_x_sum -= std::cos(previousPoint.course() * M_PI / 180.0);
							course_y_sum -= std::sin(previousPoint.course() * M_PI / 180.0);
							course_count -= 1;
						}
					}

					// Compute 'course' smooth values
					if (pool_.current().hasValue(TelemetryData::DataFix)) {
						if (!pool_.current().isPause()) {
							double course = std::atan2(course_y_sum / course_count, course_x_sum / course_count) * 180.0 / M_PI;
							pool_.current().setCourse(course);
						}
					}
				}
			}
			else if (course_method == TelemetrySettings::SmoothButterworth) {
				if (pool_.tell() > 2) {
					double a = 4.0;
					double z = 0.7;

					if (pool_.current().hasValue(TelemetryData::DataFix)) {
						if (!pool_.current().isPause()) {
							double n0, n1, n2;

							double course_x, course_y;

							// x projection
							n0 = std::cos(pool_.current().course() * M_PI / 180.0);
							n1 = std::cos(pool_.previous().course() * M_PI / 180.0);
							n2 = std::cos(pool_.previous(1).course() * M_PI / 180.0);

							course_x = 
									(n0 + (n1 * (a + ((a * a) / (2 * z * z)))) \
									- (n2 * (a * a) / (4 * z * z))) / (1 + a + ((a * a) / (4 * z * z)));

							// y projection
							n0 = std::sin(pool_.current().course() * M_PI / 180.0);
							n1 = std::sin(pool_.previous().course() * M_PI / 180.0);
							n2 = std::sin(pool_.previous(1).course() * M_PI / 180.0);

							course_y = 
									(n0 + (n1 * (a + ((a * a) / (2 * z * z)))) \
									- (n2 * (a * a) / (4 * z * z))) / (1 + a + ((a * a) / (4 * z * z)));

							pool_.current().setCourse(
									std::atan2(course_y, course_x) * 180.0 / M_PI
							);
						}
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
						if (nextPoint.hasValue(TelemetryData::DataHeading)) {
							heading_x_sum += std::cos(nextPoint.heading() * M_PI / 180.0);
							heading_y_sum += std::sin(nextPoint.heading() * M_PI / 180.0);
							heading_count += 1;
						}
					}

					// Remove old point
					if (heading_points.size() > heading_window) {
						previousPoint = heading_points.front();
						heading_points.pop_front();

						if (previousPoint.hasValue(TelemetryData::DataHeading)) {
							heading_x_sum -= std::cos(previousPoint.heading() * M_PI / 180.0);
							heading_y_sum -= std::sin(previousPoint.heading() * M_PI / 180.0);
							heading_count -= 1;
						}
					}

					// Compute 'heading' smooth values
					if (pool_.current().hasValue(TelemetryData::DataHeading)) {
						if (!pool_.current().isPause()) {
							double heading = std::atan2(heading_y_sum / heading_count, heading_x_sum / heading_count) * 180.0 / M_PI;
							pool_.current().setHeading(heading);
						}
					}
				}
			}
			else if (heading_method == TelemetrySettings::SmoothButterworth) {
				if (pool_.tell() > 2) {
					double a = 4.0;
					double z = 0.7;

					if (pool_.current().hasValue(TelemetryData::DataHeading)) {
						if (!pool_.current().isPause()) {
							double n0, n1, n2;

							double heading_x, heading_y;

							// x projection
							n0 = std::cos(pool_.current().heading() * M_PI / 180.0);
							n1 = std::cos(pool_.previous().heading() * M_PI / 180.0);
							n2 = std::cos(pool_.previous(1).heading() * M_PI / 180.0);

							heading_x = 
									(n0 + (n1 * (a + ((a * a) / (2 * z * z)))) \
									- (n2 * (a * a) / (4 * z * z))) / (1 + a + ((a * a) / (4 * z * z)));

							// y projection
							n0 = std::sin(pool_.current().heading() * M_PI / 180.0);
							n1 = std::sin(pool_.previous().heading() * M_PI / 180.0);
							n2 = std::sin(pool_.previous(1).heading() * M_PI / 180.0);

							heading_y = 
									(n0 + (n1 * (a + ((a * a) / (2 * z * z)))) \
									- (n2 * (a * a) / (4 * z * z))) / (1 + a + ((a * a) / (4 * z * z)));

							// result
							pool_.current().setHeading(
									std::atan2(heading_y, heading_x) * 180.0 / M_PI
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
			else if (elevation_method == TelemetrySettings::SmoothSavitzkyGolay) {
				if ((elevation_window > 1) && (elevation_coeff.size() > 0)) {
					ssize_t n = pool_.count();
					ssize_t half = elevation_window / 2;

					// Add new point
					nextPoint = pool_.next(elevation_window/2 - 1);

					// Save point
					if (nextPoint.type() != TelemetryData::TypeUnknown)
						elevation_points.emplace_back(nextPoint);

					// Remove old point
					if (elevation_points.size() > elevation_window) {
						previousPoint = elevation_points.front();
						elevation_points.pop_front();
					}

					if (elevation_points.size() >= (size_t) half) {
						double sum = 0;

						for (ssize_t k=-half; k<=half; k++) {
							int idx = i + k;

							// Mirror padding
							if (idx < 0)
								idx = -idx;
							else if (idx >= n) 
								idx = (2*n - idx - 2) - (i - half);
							else if (i > half)
								idx -= i - half;

							sum += elevation_points[idx].elevation() * elevation_coeff[k + half];
						}

						// Update elevation
						if (pool_.current().hasValue(TelemetryData::DataFix)) {
							if (!pool_.current().isPause())
								pool_.current().setElevation(sum);
						}
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
			else if (acceleration_method == TelemetrySettings::SmoothSavitzkyGolay) {
				if ((acceleration_window > 1) && (acceleration_coeff.size() > 0)) {
					ssize_t n = pool_.count();
					ssize_t half = acceleration_window / 2;

					// Add new point
					nextPoint = pool_.next(acceleration_window/2 - 1);

					// Save point
					if (nextPoint.type() != TelemetryData::TypeUnknown)
						acceleration_points.emplace_back(nextPoint);

					// Remove old point
					if (acceleration_points.size() > acceleration_window) {
						previousPoint = acceleration_points.front();
						acceleration_points.pop_front();
					}

					if (acceleration_points.size() >= (size_t) half) {
						double sum = 0;

						for (ssize_t k=-half; k<=half; k++) {
							int idx = i + k;

							// Mirror padding
							if (idx < 0)
								idx = -idx;
							else if (idx >= n) 
								idx = (2*n - idx - 2) - (i - half);
							else if (i > half)
								idx -= i - half;

							sum += acceleration_points[idx].acceleration() * acceleration_coeff[k + half];
						}

						// Update acceleration
						if (pool_.current().hasValue(TelemetryData::DataFix)) {
							if (!pool_.current().isPause())
								pool_.current().setAcceleration(sum);
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

	size_t grade_order = 0;
	size_t verticalspeed_order = 0;

	double grade_sum = 0;
	double verticalspeed_sum = 0;

	std::vector<double> grade_coeff;
	std::vector<double> verticalspeed_coeff;

	TelemetrySettings::Smooth grade_method;
	TelemetrySettings::Smooth verticalspeed_method;

	std::deque<Point> grade_points;
	std::deque<Point> verticalspeed_points;

	TelemetrySource::Point nextPoint, previousPoint;

	log_call();

	grade_method = settings().telemetrySmoothMethod(TelemetryData::DataGrade);
	verticalspeed_method = settings().telemetrySmoothMethod(TelemetryData::DataVerticalSpeed);

	grade_window = (settings().telemetrySmoothPoints(TelemetryData::DataGrade) * 2) + 1;
	verticalspeed_window = (settings().telemetrySmoothPoints(TelemetryData::DataVerticalSpeed) * 2) + 1;

	grade_order = settings().telemetrySmoothOrder(TelemetryData::DataGrade);
	verticalspeed_order = settings().telemetrySmoothOrder(TelemetryData::DataVerticalSpeed);

	// Fill 'grade' window
	if ((grade_method == TelemetrySettings::SmoothWindowedMovingAverage) 
			|| (grade_method == TelemetrySettings::SmoothSavitzkyGolay)) {
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
	}

	// Fill 'verticalspeed' window
	if (verticalspeed_method == TelemetrySettings::SmoothWindowedMovingAverage) {
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
	}

	// Compute Savitzky Golay coefficients
	if (grade_method == TelemetrySettings::SmoothSavitzkyGolay)
	   	grade_coeff = SavitzkyGolay::coefficients(grade_window, grade_order);

	// Compute Savitzky Golay coefficients
	if (verticalspeed_method == TelemetrySettings::SmoothSavitzkyGolay)
	   	verticalspeed_coeff = SavitzkyGolay::coefficients(verticalspeed_window, verticalspeed_order);

	// Move to first point
	pool_.seek(1);

	// Compute average value for each point
	for (int i = 0; !pool_.empty(); i++) {
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
			else if (grade_method == TelemetrySettings::SmoothSavitzkyGolay) {
				if ((grade_window > 1) && (grade_coeff.size() > 0)) {
					ssize_t n = pool_.count();
					ssize_t half = grade_window / 2;

					// Add new point
					nextPoint = pool_.next(grade_window/2 - 1);

					// Save point
					if (nextPoint.type() != TelemetryData::TypeUnknown)
						grade_points.emplace_back(nextPoint);

					// Remove old point
					if (grade_points.size() > grade_window) {
						previousPoint = grade_points.front();
						grade_points.pop_front();
					}

					if (grade_points.size() >= (size_t) half) {
						double sum = 0;

						for (ssize_t k=-half; k<=half; k++) {
							int idx = i + k;

							// Mirror padding
							if (idx < 0)
								idx = -idx;
							else if (idx >= n) 
								idx = (2*n - idx - 2) - (i - half);
							else if (i > half)
								idx -= i - half;

							sum += grade_points[idx].grade() * grade_coeff[k + half];
						}

						// Update grade
						pool_.current().setGrade(sum);
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
			else if (verticalspeed_method == TelemetrySettings::SmoothSavitzkyGolay) {
				if ((verticalspeed_window > 1) && (verticalspeed_coeff.size() > 0)) {
					ssize_t n = pool_.count();
					ssize_t half = verticalspeed_window / 2;

					// Add new point
					nextPoint = pool_.next(verticalspeed_window/2 - 1);

					// Save point
					if (nextPoint.type() != TelemetryData::TypeUnknown)
						verticalspeed_points.emplace_back(nextPoint);

					// Remove old point
					if (verticalspeed_points.size() > verticalspeed_window) {
						previousPoint = verticalspeed_points.front();
						verticalspeed_points.pop_front();
					}

					if (verticalspeed_points.size() >= (size_t) half) {
						double sum = 0;

						for (ssize_t k=-half; k<=half; k++) {
							int idx = i + k;

							// Mirror padding
							if (idx < 0)
								idx = -idx;
							else if (idx >= n) 
								idx = (2*n - idx - 2) - (i - half);
							else if (i > half)
								idx -= i - half;

							sum += verticalspeed_points[idx].verticalspeed() * verticalspeed_coeff[k + half];
						}

						// Update verticalspeed
						pool_.current().setVerticalSpeed(sum);
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


void TelemetrySource::smooth_step_three(void) {
	int position_count = 0;

	size_t position_window = 0;

	size_t position_order = 0;

	double lat_sum = 0;
	double lon_sum = 0;

	std::vector<double> position_coeff;

	TelemetrySettings::Smooth position_method;

	std::deque<Point> position_points;

	TelemetrySource::Point nextPoint, previousPoint;

	log_call();

	position_method = settings().telemetrySmoothMethod(TelemetryData::DataPosition);

	position_window = (settings().telemetrySmoothPoints(TelemetryData::DataPosition) * 2) + 1;

	position_order = settings().telemetrySmoothOrder(TelemetryData::DataPosition);

	// Fill 'position' window
	if ((position_method == TelemetrySettings::SmoothWindowedMovingAverage) 
			|| (position_method == TelemetrySettings::SmoothSavitzkyGolay)) {
		for (size_t i=0; i<position_window/2; i++) {
			nextPoint = pool_.next(i);

			if (nextPoint.type() == TelemetryData::TypeUnknown)
				break;

			// Save point
			position_points.emplace_back(nextPoint);

			if (nextPoint.hasValue(TelemetryData::DataFix)) {
				lat_sum += nextPoint.latitude();
				lon_sum += nextPoint.longitude();

				position_count += 1;
			}
		}
	}

	// Compute Savitzky Golay coefficients
	if (position_method == TelemetrySettings::SmoothSavitzkyGolay)
	   	position_coeff = SavitzkyGolay::coefficients(position_window, position_order);

	// Move to first point
	pool_.seek(1);

	// Compute average value for each point
	for (ssize_t i = 0; !pool_.empty(); i++) {
		if (pool_.current().type() != TelemetryData::TypeError) {
			// position
			//----------

			if (position_method == TelemetrySettings::SmoothWindowedMovingAverage) {
				if (position_window > 1) {
					// Add new point
					nextPoint = pool_.next(position_window/2 - 1);

					// Save point
					position_points.emplace_back(nextPoint);

					if (nextPoint.type() != TelemetryData::TypeUnknown) {
						if (nextPoint.hasValue(TelemetryData::DataFix)) {
							lat_sum += nextPoint.latitude();
							lon_sum += nextPoint.longitude();
							position_count += 1;
						}
					}

					// Remove old point
					if (position_points.size() > position_window) {
						previousPoint = position_points.front();
						position_points.pop_front();

						if (previousPoint.hasValue(TelemetryData::DataFix)) {
							lat_sum -= previousPoint.latitude();
							lon_sum -= previousPoint.longitude();
							position_count -= 1;
						}
					}

					if (pool_.current().hasValue(TelemetryData::DataFix)) {
						// Compute 'position' smooth values
						pool_.current().setPosition(lat_sum / position_count, lon_sum / position_count);
					}
				}
			}
			else if (position_method == TelemetrySettings::SmoothSavitzkyGolay) {
				if ((position_window > 1) && (position_coeff.size() > 0)) {
					ssize_t n = pool_.count();
					ssize_t half = position_window / 2;

					// Add new point
					nextPoint = pool_.next(position_window/2 - 1);

					// Save point
					if (nextPoint.type() != TelemetryData::TypeUnknown)
						position_points.emplace_back(nextPoint);

					// Remove old point
					if (position_points.size() > position_window) {
						previousPoint = position_points.front();
						position_points.pop_front();
					}

					if (position_points.size() >= (size_t) half) {
						double x_sum = 0;
						double y_sum = 0;

						for (ssize_t k=-half; k<=half; k++) {
							int idx = i + k;

							// Mirror padding
							if (idx < 0)
								idx = -idx;
							else if (idx >= n) 
								idx = (2*n - idx - 2) - (i - half);
							else if (i > half)
								idx -= i - half;

							x_sum += position_points[idx].x() * position_coeff[k + half];
							y_sum += position_points[idx].y() * position_coeff[k + half];
						}

						// Update position
						if (pool_.current().hasValue(TelemetryData::DataFix)) {
							pool_.current().setXY(x_sum, y_sum);
						}
					}
				}
			}
			else if (position_method == TelemetrySettings::SmoothButterworth) {
				if (pool_.tell() > 2) {
					double a = 4.0;
					double z = 0.7;

					if (pool_.current().hasValue(TelemetryData::DataFix)) {
						// Compute 'position' smooth values
						double lat = (pool_.current().latitude() + (pool_.previous().latitude() * (a + ((a * a) / (2 * z * z)))) \
							- (pool_.previous(1).latitude() * (a * a) / (4 * z * z))) / (1 + a + ((a * a) / (4 * z * z)));
						double lon = (pool_.current().longitude() + (pool_.previous().longitude() * (a + ((a * a) / (2 * z * z)))) \
							- (pool_.previous(1).longitude() * (a * a) / (4 * z * z))) / (1 + a + ((a * a) / (4 * z * z)));

						pool_.current().setPosition(lat , lon);
					}
				}
			}
		}

		// Move to next
		pool_.seek(1);
	}

	// Reset
	pool_.reset();




//	int index = 0;
//
//	double lat, lon;
//
//	size_t samplerate = settings().telemetrySmoothPoints(TelemetryData::DataPosition);
//
//	TelemetrySource::Point curPoint;
//
//	// Move to first point
//	pool_.seek(1);
//
//	// Compute average value for each point
//	while ((samplerate > 0) && !pool_.empty()) {
//		// Next point
//		index++;
//		pool_.seek(1);
//
//		// Get current point
//		curPoint = pool_.current();
//
//		// Skip invalid point
//		if (curPoint.type() == TelemetryData::TypeError)
//			continue;
//
//		// Keep point of interest
//		if (index % samplerate == 0)
//			continue;
//
//		// Enough point
//		if (pool_.size() < samplerate)
//			break;
//
//		printf("prev: %d - next: %d\n", index % samplerate - 1, samplerate - (index % samplerate) - 1);
//
//		// Extract previsous & next point
//		previousPoint = pool_.previous(index % samplerate - 1);
//		nextPoint = pool_.next(samplerate - (index % samplerate) - 1);
//
//		// Compute position
//		lat = previousPoint.lat_ + ((int) (curPoint.ts_ - previousPoint.ts_)) * (nextPoint.lat_ - previousPoint.lat_) / ((int) (nextPoint.ts_ - previousPoint.ts_));
//		lon = previousPoint.lon_ + ((int) (curPoint.ts_ - previousPoint.ts_)) * (nextPoint.lon_ - previousPoint.lon_) / ((int) (nextPoint.ts_ - previousPoint.ts_));
//
//		// Save position
//		curPoint.setPosition(lat, lon);
//
//		// Save point
//		pool_.current() = curPoint;
//	}
//
//	// Reset
//	pool_.reset();
}


void TelemetrySource::smooth(void) {
	size_t order;
	size_t points;

	TelemetrySettings::Smooth method;

	log_call();

	if (!quiet_) {
		printf("%s: Apply smooth telemetry filter\n", name().c_str());

		// 1/ position
		method = settings().telemetrySmoothMethod(TelemetryData::DataPosition);
		points = settings().telemetrySmoothPoints(TelemetryData::DataPosition);
		order = settings().telemetrySmoothOrder(TelemetryData::DataPosition);

		if ((method == TelemetrySettings::SmoothWindowedMovingAverage) && (points > 1))
			printf("     - position: 'windowed moving average' filter - window size '%ld' (+/- %ld points)\n", 
					2 * points + 1, points);
		else if ((method == TelemetrySettings::SmoothSavitzkyGolay) && (points > 1))
			printf("     - position: 'Savitzky & Golay' filter - window size '%ld' (+/- %ld points) - order %ld\n",
					2 * points + 1, points, order);
		else if (method == TelemetrySettings::SmoothButterworth)
			printf("     - position: 'butterworth' filter - a = 4.0 / z = 0.7\n");
		else
			printf("     - position: no\n");

		// 2/ grade
		method = settings().telemetrySmoothMethod(TelemetryData::DataGrade);
		points = settings().telemetrySmoothPoints(TelemetryData::DataGrade);
		order = settings().telemetrySmoothOrder(TelemetryData::DataGrade);

		if ((method == TelemetrySettings::SmoothWindowedMovingAverage) && (points > 1))
			printf("     - grade: 'windowed moving average' filter - window size '%ld' (+/- %ld points)\n", 
					2 * points + 1, points);
		else if ((method == TelemetrySettings::SmoothSavitzkyGolay) && (points > 1))
			printf("     - grade: 'Savitzky & Golay' filter - window size '%ld' (+/- %ld points) - order %ld\n",
					2 * points + 1, points, order);
		else if (method == TelemetrySettings::SmoothButterworth)
			printf("     - grade: 'butterworth' filter - a = 4.0 / z = 0.7\n");
		else
			printf("     - grade: no\n");

		// 3/ speed
		method = settings().telemetrySmoothMethod(TelemetryData::DataSpeed);
		points = settings().telemetrySmoothPoints(TelemetryData::DataSpeed);
		order = settings().telemetrySmoothOrder(TelemetryData::DataSpeed);

		if ((method == TelemetrySettings::SmoothWindowedMovingAverage) && (points > 1))
			printf("     - speed: 'windowed moving average' filter - window size '%ld' (+/- %ld points)\n", 
					2 * points + 1, points);
		else if ((method == TelemetrySettings::SmoothSavitzkyGolay) && (points > 1))
			printf("     - speed: 'Savitzky & Golay' filter - window size '%ld' (+/- %ld points) - order %ld\n",
					2 * points + 1, points, order);
		else if (method == TelemetrySettings::SmoothButterworth)
			printf("     - speed: 'butterworth' filter - a = 4.0 / z = 0.7\n");
		else
			printf("     - speed: no\n");

		// 4/ course
		method = settings().telemetrySmoothMethod(TelemetryData::DataCourse);
		points = settings().telemetrySmoothPoints(TelemetryData::DataCourse);
		order = settings().telemetrySmoothOrder(TelemetryData::DataCourse);

		if ((method == TelemetrySettings::SmoothWindowedMovingAverage) && (points > 1))
			printf("     - course: 'windowed moving average' filter - window size '%ld' (+/- %ld points)\n", 
					2 * points + 1, points);
		else if ((method == TelemetrySettings::SmoothSavitzkyGolay) && (points > 1))
			printf("     - course: 'Savitzky & Golay' filter - window size '%ld' (+/- %ld points) - order %ld\n",
					2 * points + 1, points, order);
		else if (method == TelemetrySettings::SmoothButterworth)
			printf("     - course: 'butterworth' filter - a = 4.0 / z = 0.7\n");
		else
			printf("     - course: no\n");

		// 4/ heading
		method = settings().telemetrySmoothMethod(TelemetryData::DataHeading);
		points = settings().telemetrySmoothPoints(TelemetryData::DataHeading);
		order = settings().telemetrySmoothOrder(TelemetryData::DataHeading);

		if ((method == TelemetrySettings::SmoothWindowedMovingAverage) && (points > 1))
			printf("     - heading: 'windowed moving average' filter - window size '%ld' (+/- %ld points)\n", 
					2 * points + 1, points);
		else if ((method == TelemetrySettings::SmoothSavitzkyGolay) && (points > 1))
			printf("     - heading: 'Savitzky & Golay' filter - window size '%ld' (+/- %ld points) - order %ld\n",
					2 * points + 1, points, order);
		else if (method == TelemetrySettings::SmoothButterworth)
			printf("     - heading: 'butterworth' filter - a = 4.0 / z = 0.7\n");
		else
			printf("     - heading: no\n");

		// 5/ elevation
		method = settings().telemetrySmoothMethod(TelemetryData::DataElevation);
		points = settings().telemetrySmoothPoints(TelemetryData::DataElevation);
		order = settings().telemetrySmoothOrder(TelemetryData::DataElevation);

		if ((method == TelemetrySettings::SmoothWindowedMovingAverage) && (points > 1))
			printf("     - elevation: 'windowed moving average' filter - window size '%ld' (+/- %ld points)\n", 
					2 * points + 1, points);
		else if ((method == TelemetrySettings::SmoothSavitzkyGolay) && (points > 1))
			printf("     - elevation: 'Savitzky & Golay' filter - window size '%ld' (+/- %ld points) - order %ld\n",
					2 * points + 1, points, order);
		else if (method == TelemetrySettings::SmoothButterworth)
			printf("     - elevation: 'butterworth' filter - a = 4.0 / z = 0.7\n");
		else
			printf("     - elevation: no\n");

		// 6/ acceleration
		method = settings().telemetrySmoothMethod(TelemetryData::DataAcceleration);
		points = settings().telemetrySmoothPoints(TelemetryData::DataAcceleration);
		order = settings().telemetrySmoothOrder(TelemetryData::DataAcceleration);

		if ((method == TelemetrySettings::SmoothWindowedMovingAverage) && (points > 1))
			printf("     - acceleration: 'windowed moving average' filter - window size '%ld' (+/- %ld points)\n", 
					2 * points + 1, points);
		else if ((method == TelemetrySettings::SmoothSavitzkyGolay) && (points > 1))
			printf("     - acceleration: 'Savitzky & Golay' filter - window size '%ld' (+/- %ld points) - order %ld\n",
					2 * points + 1, points, order);
		else if (method == TelemetrySettings::SmoothButterworth)
			printf("     - acceleration: 'butterworth' filter - a = 4.0 / z = 0.7\n");
		else
			printf("     - acceleration: no\n");

		// 7/ verticalspeed
		method = settings().telemetrySmoothMethod(TelemetryData::DataVerticalSpeed);
		points = settings().telemetrySmoothPoints(TelemetryData::DataVerticalSpeed);
		order = settings().telemetrySmoothOrder(TelemetryData::DataVerticalSpeed);

		if ((method == TelemetrySettings::SmoothWindowedMovingAverage) && (points > 1))
			printf("     - verticalspeed: 'windowed moving average' filter - window size '%ld' (+/- %ld points)\n", 
					2 * points + 1, points);
		else if ((method == TelemetrySettings::SmoothSavitzkyGolay) && (points > 1))
			printf("     - verticalspeed: 'Savitzky & Golay' filter - window size '%ld' (+/- %ld points) - order %ld\n",
					2 * points + 1, points, order);
		else if (method == TelemetrySettings::SmoothButterworth)
			printf("     - verticalspeed: 'butterworth' filter - a = 4.0 / z = 0.7\n");
		else
			printf("     - verticalspeed: no\n");
	}

	smooth_step_one();
	smooth_step_two();
	smooth_step_three();
}


/**
 * Fix bad point by interpolation:
 *  - speed, avgspeed, avgridespeed
 *  - acceleration
 *  - distance
 *  - elevation
 * So compute:
 *  - grade
 *  - course
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

			if (currentPoint.hasValue(TelemetryData::DataCourse)) {
				double course = prevPoint.course_ + k * (nextPoint.course_ - prevPoint.course_);
				currentPoint.setCourse(course);
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


void TelemetrySource::bounds(void) {
	TelemetrySource::Point point;
	TelemetrySource::Point prevPoint;

	// Move to first point
	pool_.seek(1);

	if (pool_.empty())
		return;

	// Initialize
	point = pool_.current();

	point.ele_min_ = point.ele_;
	point.ele_max_ = point.ele_;

	point.distance_min_ = point.distance_;
	point.distance_max_ = point.distance_;

	// Save point
	pool_.current() = point;

	prevPoint = point;

	// Move to second point
	pool_.seek(1);

	// For each point
	while (!pool_.empty()) {
		point = pool_.current();

		point.ele_min_ = std::min(prevPoint.ele_min_, point.ele_);
		point.ele_max_ = std::max(prevPoint.ele_max_, point.ele_);

		point.distance_min_ = std::min(prevPoint.distance_min_, point.distance_);
		point.distance_max_ = std::max(prevPoint.distance_max_, point.distance_);

		// Save point
		pool_.current() = point;

		prevPoint = point;

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

	if (!quiet_)
		printf("%s: Load telemetry data.\n", name().c_str());

	reset();
	clear();
	config();
	load();
	range();
	filter();
	compute();
	fix();
	smooth();
	trim();
	bounds();

	return type;
}


enum TelemetrySource::Data TelemetrySource::loadData(bool verbose) {
	bool save = quiet_;

	enum TelemetrySource::Data result;

	setQuiet(!verbose);
	result = loadData();
	setQuiet(save);

	return result;
}


void TelemetrySource::insertData(uint64_t timestamp) {
	int flags;

	TelemetryData data;

	TelemetrySource::Point point;

	// Current data
	data = pool_.current();

	// Predict data by interpolation
	predictData(data, TelemetrySettings::MethodInterpolate, timestamp);

	// Use the last line 
	point.setLine(data.line());

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

		if (prevPoint.hasValue(TelemetryData::DataCourse) && nextPoint.hasValue(TelemetryData::DataCourse)) {
			double course;

			double prev_course = prevPoint.course_;
			double next_course = nextPoint.course_;

			if (std::abs(next_course - prev_course) > 180.0) {
				if (next_course > prev_course)
					prev_course += 360.0;
				else
					next_course += 360.0;
			}
			
			course = prev_course + k * (next_course - prev_course);

			if (course >= 360.0)
				course -= 360.0;

			point.setCourse(course);
		}

		if (prevPoint.hasValue(TelemetryData::DataHeading) && nextPoint.hasValue(TelemetryData::DataHeading)) {
			double heading;

			double prev_heading = prevPoint.heading_;
			double next_heading = nextPoint.heading_;

			if (std::abs(next_heading - prev_heading) > 180.0) {
				if (next_heading > prev_heading)
					prev_heading += 360.0;
				else
					next_heading += 360.0;
			}
			
			heading = prev_heading + k * (next_heading - prev_heading);

			if (heading >= 360.0)
				heading -= 360.0;

			point.setHeading(heading);
		}

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

		if (prevPoint.hasValue(TelemetryData::DataCourse) && curPoint.hasValue(TelemetryData::DataCourse)) {
			double course;

			double prev_course = prevPoint.course_;
			double cur_course = curPoint.course_;

			if (std::abs(cur_course - prev_course) > 180.0) {
				if (cur_course > prev_course)
					prev_course += 360.0;
				else
					cur_course += 360.0;
			}
			
			course = cur_course + k * (cur_course - prev_course);

			if (course >= 360.0)
				course -= 360.0;

			point.setCourse(course);
		}

		if (prevPoint.hasValue(TelemetryData::DataHeading) && curPoint.hasValue(TelemetryData::DataHeading)) {
			double heading;

			double prev_heading = prevPoint.heading_;
			double cur_heading = curPoint.heading_;

			if (std::abs(cur_heading - prev_heading) > 180.0) {
				if (cur_heading > prev_heading)
					prev_heading += 360.0;
				else
					cur_heading += 360.0;
			}
			
			heading = cur_heading + k * (cur_heading - prev_heading);

			if (heading >= 360.0)
				heading -= 360.0;

			point.setHeading(heading);
		}

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
			| TelemetryData::DataCourse
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
	else if (ext == ".tcx") {
		source = new TCX(filename);
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

