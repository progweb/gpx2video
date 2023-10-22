#include <fstream>
#include <iostream>
#include <string>

#include <string.h>
//#define __USE_XOPEN  // For strptime
#include <time.h>

#include "unistd.h"

#include "utmconvert/utmconvert.h"
#include <GeographicLib/Geodesic.hpp>


#include "gpxlib/Parser.h"
#include "gpxlib/ReportCerr.h"

#include "log.h"
#include "gpx.h"


void show(gpx::Node &node, unsigned width)
{
	std::string value(width, ' ');

  value = node.getValue();

  if (value.length() > width)
  {
    value = value.substr(0, width);
  }
  else if (value.length() < width)
  {
    value.insert(0, width - value.length(), ' ');
  }

  std::cout << value << ' ';
}



#define NOISE 0.1


// GPX Data Point
//----------------

GPXData::GPXData() 
	: enable_(false)
	, has_value_(GPXData::DataNone)
	, nbr_points_(0)
	, line_(0)
	, valid_(false)
	, type_(GPXData::TypeUnknown)
	, ridetime_(0)
	, elapsedtime_(0)
	, duration_(0)
	, distance_(0)
	, speed_(0) 
	, maxspeed_(0)
	, avgspeed_(0)
	, avgridespeed_(0)
	, grade_(0) 
	, cadence_(0)
	, lap_(1)
	, in_lap_(false) {

	nbr_predictions_ = 0;

	filter_ = alloc_filter_velocity2d(10.0);
}


GPXData::~GPXData() {
//	free_filter(filter_);
}


void GPXData::dump(bool debug) {
	char s[128];

	struct tm tm;

	time_t time = cur_pt_.ts / 1000;

	localtime_r(&time, &tm);

	strftime(s, sizeof(s), "%Y-%m-%d %H:%M:%S", &tm);

	printf("  [%d] GPX Time: %s Distance: %.3f km in %d seconds, current speed is %.3f (valid: %s)\n",
		line_,
		s,
		distance_/1000.0, (int) round(elapsedtime_), speed_, 
		valid() ? "true" : "false");  
	
	if (debug) {
		printf("  - lon: %f\n", position().lon);
		printf("  - lat: %f\n", position().lat);
	}
}


void GPXData::convert(struct GPXData::point *pt, gpx::WPT *wpt) {
	char buf[128];

	const char *s;

	struct tm time;

	struct Utm_val utm;

	pt->valid = true;

	// Convert time - GPX file contains UTC time
	s = wpt->time().getValue().c_str();

	// Try format: "2020:12:13 08:55:48.215"
	memset(&time, 0, sizeof(time));
	if (strptime(s, "%Y:%m:%d %H:%M:%S.", &time) != NULL)
		pt->ts = timegm(&time) * 1000;
	// Try format: "2020-07-28T07:04:43.000Z"
	else if (strptime(s, "%Y-%m-%dT%H:%M:%S.", &time) != NULL)
		pt->ts = timegm(&time) * 1000;
	// Try format: "2020-07-28T07:04:43Z"
	else if (strptime(s, "%Y-%m-%dT%H:%M:%SZ", &time) != NULL)
		pt->ts = timegm(&time) * 1000;
	// Try format: "2020-07-28T07:04:43+0200"
	else if (strptime(s, "%Y-%m-%dT%H:%M:%S+", &time) != NULL)
		pt->ts = timegm(&time) * 1000;
	else
		pt->valid = false;

	// Parse ms precision
	if (pt->valid) {
		char *ms;

		strcpy(buf, s);

		if ((ms = ::strchr(buf, '.')) != NULL) {
			ms += 1; // skip '.' char

			if (strlen(ms) >= 3) {
				ms[3] = '\0';

				pt->ts += atoi(ms);
			}
		}
	}

	// Convert lat, lon & ele
	pt->lat = (double) wpt->lat();
	pt->lon = (double) wpt->lon();
	pt->ele = (double) wpt->ele();

	utm = to_utm(pt->lat, pt->lon);

	// Return result
	pt->x = utm.x;
	pt->y = utm.y;
}


bool GPXData::compute(void) {
//	double dx = (cur_pt_.x - prev_pt_.x);
//	double dy = (cur_pt_.y - prev_pt_.y);
	double dz = (cur_pt_.ele - prev_pt_.ele);

//	double dc = sqrt(dx*dx + dy*dy + dz*dz);

	double dc;

	double speed = 0;

	GeographicLib::Math::real lat1 = cur_pt_.lat;
	GeographicLib::Math::real lon1 = cur_pt_.lon;
	GeographicLib::Math::real lat2 = prev_pt_.lat;
	GeographicLib::Math::real lon2 = prev_pt_.lon;
	GeographicLib::Math::real d;

//	GeographicLib::Geodesic gsic(6378388, 1/297.0);
	GeographicLib::Geodesic gsic(6378137.0, 1.0/298.2572);
	gsic.Inverse(lat1, lon1, lat2, lon2, d);

	dc = d;

//	double dt = difftime(cur_pt_.time, prev_pt_.time);
	double dt = cur_pt_.ts - prev_pt_.ts;

//	dc = sqrt(dx*dx + dy*dy);

	// grade elevation
	if (floor(dc) > 0) 
		grade_ = 100.0 * dz / dc;

	// speed & maxspeed
	if (dt > 0) {
		speed = (3600.0 * dc) / (1.0 * dt);
		if (abs((int) (speed - speed_)) < 50)
			speed_ = speed;
		if (enable_ && (speed_ > maxspeed_))
			maxspeed_ = speed_;
	}

	if (enable_) {
		// duration_ in second
		duration_ += (dt / 1000.0);
	
		// distance_ in meter
		distance_ += dc;

		// average speed
		if (duration_ > 0)
			avgspeed_ = (3600.0 * distance_) / (1000.0 * duration_);

		// average ride speed
		if (ridetime_ > 0)
			avgridespeed_ = (3600.0 * distance_) / (1000.0 * ridetime_);

		// Determine current lap
		lat1 = cur_pt_.lat;
		lon1 = cur_pt_.lon;
		lat2 = start_pt_.lat;
		lon2 = start_pt_.lon;
//		GeographicLib::Math::real d;

//		GeographicLib::Geodesic gsic(6378388, 1/297.0);
		gsic.Inverse(lat1, lon1, lat2, lon2, d);
	
		if (in_lap_ && (d < 8)) {
			lap_++;
			in_lap_ = false;
		}
		else if (!in_lap_ && (d > 15)) {
			in_lap_ = true;
		}
	}

	return true;
}
	

bool GPXData::smooth(GPXData &prev, GPXData &next) {
	speed_ = (prev.speed_ + speed_ + next.speed_) / 3;
	grade_ = (prev.grade_ + grade_ + next.grade_) / 3;

	return true;
}


void GPXData::init(void) {
	memcpy(&last_pt_, &next_pt_, sizeof(last_pt_));
	memcpy(&cur_pt_, &next_pt_, sizeof(cur_pt_));
	memcpy(&prev_pt_, &cur_pt_, sizeof(prev_pt_));

	memcpy(&start_pt_, &cur_pt_, sizeof(prev_pt_));

	reset();
}


void GPXData::reset(void) {
	enable_ = false;
	has_value_ = false;

	ridetime_ = 0;
	elapsedtime_ = 0;
	duration_ = 0.0;
	distance_ = 0.0;
	speed_ = 0.0;
	maxspeed_ = 0.0;
	avgspeed_ = 0.0;
	avgridespeed_ = 0.0;
	grade_ = 0.0;
	temperature_ = 0.0;
	heartrate_ = 0;
	cadence_ = 0;

	lap_ = 1;
	in_lap_ = false;
}


bool GPXData::unchanged(void) {
	type_ = GPXData::TypeUnchanged;

	return true;
}


bool GPXData::predict(enum TelemetrySettings::Filter filter) {
	struct Utm_val utm;

	struct point prev_pt;

	bool result = false;

	log_call();

	if (filter == TelemetrySettings::FilterNone)
		return result;

	// By default, no changes
	type_ = GPXData::TypeUnchanged;

	// New prediction
	nbr_predictions_++;

	// Save previous point
	memcpy(&prev_pt, &prev_pt_, sizeof(prev_pt));
	memcpy(&prev_pt_, &cur_pt_, sizeof(prev_pt_));

	cur_pt_.ts += 1000; // Prediction should occur each 1 second

	switch (filter) {
	case TelemetrySettings::FilterKalman:
		// Predict
		::update(filter_);
		::get_lat_long(filter_, &cur_pt_.lat, &cur_pt_.lon);

		result = true;
		break;

	case TelemetrySettings::FilterInterpolate:
		cur_pt_.lat += 1000.0 * (next_pt_.lat - prev_pt_.lat) / (next_pt_.ts - prev_pt_.ts);
		cur_pt_.lon += 1000.0 * (next_pt_.lon - prev_pt_.lon) / (next_pt_.ts - prev_pt_.ts);
		cur_pt_.ele += 1000.0 * (next_pt_.ele - prev_pt_.ele) / (next_pt_.ts - prev_pt_.ts);

		result = true;
		break;

	case TelemetrySettings::FilterLinear:
		cur_pt_.lat += (prev_pt_.lat - prev_pt.lat);
		cur_pt_.lon += (prev_pt_.lon - prev_pt.lon);
		cur_pt_.ele += (prev_pt_.ele - prev_pt.ele);

		result = true;
		break;

	case TelemetrySettings::FilterSample:
	case TelemetrySettings::FilterNone:
	default:
		break;
	}

	if (filter == TelemetrySettings::FilterSample) {
		duration_ += 1.0;
	}
	else if (filter != TelemetrySettings::FilterNone) {
		utm = to_utm(cur_pt_.lat, cur_pt_.lon);

		cur_pt_.x = utm.x;
		cur_pt_.y = utm.y;

		compute();
	}

	if (enable_) {
		elapsedtime_ += (cur_pt_.ts - prev_pt_.ts) / 1000.0;

		if (speed_ >= 4.0)
			ridetime_ += (cur_pt_.ts - prev_pt_.ts) / 1000.0;
	}

	if (result)
		type_ = GPXData::TypePredicted;

	return result;
}


bool GPXData::update(enum TelemetrySettings::Filter filter) {
	bool result = false;

	log_call();

	memcpy(&prev_pt_, &cur_pt_, sizeof(prev_pt_));

	switch (filter) {
	case TelemetrySettings::FilterKalman:
		// Measure
		memcpy(&cur_pt_, &next_pt_, sizeof(cur_pt_));

		// Correct
		::update_velocity2d(filter_, next_pt_.lat, next_pt_.lon, 1.0);

		// Filter
		::get_lat_long(filter_, &cur_pt_.lat, &cur_pt_.lon);

		result = true;
		break;
	
	case TelemetrySettings::FilterInterpolate:
		memcpy(&cur_pt_, &next_pt_, sizeof(cur_pt_));

		result = true;

		break;

	case TelemetrySettings::FilterLinear:
		memcpy(&cur_pt_, &next_pt_, sizeof(cur_pt_));

		result = true;

		break;

	case TelemetrySettings::FilterSample:
		memcpy(&cur_pt_, &next_pt_, sizeof(cur_pt_));

		result = true;

		break;

	case TelemetrySettings::FilterNone:
	default:
		memcpy(&cur_pt_, &next_pt_, sizeof(cur_pt_));

		result = true;

		break;
	}

	if (filter == TelemetrySettings::FilterSample) {
		duration_ -= 1.0 * nbr_predictions_;
		prev_pt_.ts -= 1000 * nbr_predictions_;
	}

	compute();

	if (filter == TelemetrySettings::FilterSample) {
		prev_pt_.ts += 1000 * nbr_predictions_;
	}

	if (enable_) {
		elapsedtime_ += (cur_pt_.ts - prev_pt_.ts) / 1000.0;

		if (speed_ >= 4.0)
			ridetime_ += (cur_pt_.ts - prev_pt_.ts) / 1000.0;
	}

	nbr_predictions_ = 0;

	if (result)
		type_ = GPXData::TypeMeasured;
	else
		type_ = GPXData::TypeUnchanged;

	return result;
}


/**
 * Garmin Connect format:
 *   <extensions>
 *     <ns3:TrackPointExtension>
 *       <ns3:atemp>30.0</ns3:atemp>
 *       <ns3:hr>120</ns3:hr>
 *       <ns3:cad>85</ns3:cad>
 *     </ns3:TrackPointExtension>
 *   </extensions>
 *
 * RideWithGPS format:
 *   <extensions>
 *     <gpxdata:hr>120</gpxdata:hr>
 *     <gpxdata:cadence>85</gpxdata:cadence>
 *   </extensions>
 */
void GPXData::read(gpx::WPT *wpt) {
	struct point pt;

	std::string name;

	setValue(GPXData::DataNone);

	convert(&pt, wpt);

	// Skip point if not valid
	if (!pt.valid)
		return;

	// lat & lon & ele ok
	// TODO: do better
	addValue(GPXData::DataFix);
	addValue(GPXData::DataElevation);

	// Line
	line_ = wpt->line();

	// Extensions
	gpx::Node *extensions = wpt->extensions().getElements().front();

	if (extensions) {
		name = extensions->getName();

		if (name.find("TrackPointExtension") == std::string::npos)
			extensions = &wpt->extensions();

		if (extensions) {
			for (std::list<gpx::Node*>::const_iterator iter = extensions->getElements().begin(); 
				iter != extensions->getElements().end(); ++iter) {
				gpx::Node *node = (*iter);

				name = node->getName();

				if (name.find("atemp") != std::string::npos) {
					temperature_ = std::stod(node->getValue());
					addValue(GPXData::DataTemperature);
				}
				else if (name.find("cad") != std::string::npos) {
					cadence_ = std::stoi(node->getValue());
					addValue(GPXData::DataCadence);
				}
				else if (name.find("hr") != std::string::npos) {
					heartrate_ = std::stoi(node->getValue());
					addValue(GPXData::DataHeartrate);
				}
			}
		}
	}

	// Save point
	memcpy(&last_pt_, &next_pt_, sizeof(last_pt_));
	memcpy(&next_pt_, &pt, sizeof(next_pt_));

	valid_ = true;
	type_ = GPXData::TypeMeasured;
}


// GPX File Reader
//-----------------

GPX::GPX(std::ifstream &stream, gpx::GPX *root, enum TelemetrySettings::Filter filter) 
	: stream_(stream)
	, root_(root)
	, eof_(false)
	, trk_(NULL)
	, offset_(0) 
	, from_(0)
	, to_(0)
	, start_time_(0)
	, start_activity_(0)
	, filter_(filter) {
}


GPX::~GPX() {
}


GPX * GPX::open(const std::string &filename, enum TelemetrySettings::Filter filter) {
	GPX *gpx = NULL;

	GPXData data;

	gpx::GPX *root;
//	gpx::ReportCerr report;
	gpx::Parser parser(NULL); //&report);

    std::ifstream stream = std::ifstream(filename);

	if (!stream.is_open()) {
		log_error("Open '%s' GPX file failure, please check that file is readable", filename.c_str());
		goto failure;
	}

	root = parser.parse(stream);

	if (root == NULL) {
		log_error("Parsing of '%s' failed due to %s on line %d and column %d", 
			filename.c_str(), parser.errorText().c_str(),
			parser.errorLineNumber(), parser.errorColumnNumber());
		goto failure;
	}

	gpx = new GPX(stream, root, filter);

	// Parse activity start time
	gpx->parse();
	gpx->retrieveFirst(data);

	gpx->start_activity_ = data.time();

failure:
	return gpx;
}


void GPX::dump(void) {
	std::cout << "Track info:" << std::endl;
	std::cout << "  Name        : " << trk_->name()  .getValue() << std::endl;
	std::cout << "  Comment     : " << trk_->cmt()   .getValue() << std::endl;
	std::cout << "  Description : " << trk_->desc()  .getValue() << std::endl;
	std::cout << "  Source      : " << trk_->src()   .getValue() << std::endl;
	std::cout << "  Type        : " << trk_->type()  .getValue() << std::endl;
	std::cout << "  Number      : " << trk_->number().getValue() << std::endl;
	std::cout << "  Segments:   : " << trk_->trksegs().list().size()  << std::endl;
}


bool GPX::parse(void) {
	std::list<gpx::TRK*> &trks = root_->trks().list();

	for (std::list<gpx::TRK *>::iterator node = trks.begin(); node != trks.end(); ++node) {
		gpx::TRK *trk = (*node);
         
		if (trk == nullptr)
 			continue;

		trk_ = trk;

		// Parse only the first track
		break;
	}

	return true;
}


bool GPX::setFrom(std::string from) {
	struct tm time;

	log_call();

	memset(&time, 0, sizeof(time));

	if (from.empty())
		goto skip;

	if (::strptime(from.c_str(), "%Y-%m-%d %H:%M:%S", &time) == NULL) {
		log_error("Parse GPX begin date failure");
		return false;
	}

	time.tm_isdst = -1;

	// Convert race start time in UTC time
	from_ = timelocal(&time);

skip:
	return true;
}


bool GPX::setTo(std::string to) {
	struct tm time;

	log_call();

	memset(&time, 0, sizeof(time));

	if (to.empty())
		goto skip;

	if (::strptime(to.c_str(), "%Y-%m-%d %H:%M:%S", &time) == NULL) {
		log_error("Parse GPX end date failure");
		return false;
	}

	time.tm_isdst = -1;

	// Convert race start time in UTC time
	to_ = timelocal(&time);

skip:
	return true;
}


void GPX::setStartTime(time_t start_time) {
	start_time_ = start_time;
}


void GPX::setStartTime(struct tm *start_time) {
	time_t t = timegm(start_time);

	setStartTime(t);
}


void GPX::setStartTime(char *start_time) {
	struct tm time;

	memset(&time, 0, sizeof(time));

	strptime(start_time, "%Y:%m:%d %H:%M:%S.", &time);

	setStartTime(&time);
}


int64_t GPX::timeOffset(void) const {
	return offset_;
}


void GPX::setTimeOffset(const int64_t& offset) {
	offset_ = offset;
}


enum GPX::Data GPX::retrieveFirst_i(GPXData &data) {
	gpx::WPT *wpt;
	std::list<gpx::TRKSeg*> &trksegs = trk_->trksegs().list();

	log_call();

	data = GPXData();

	// Tweak
	data_list_.clear();

	// Retrieve first segment & first track
	iter_seg_ = trksegs.begin();
	
	if (iter_seg_ != trksegs.end()) {
		gpx::TRKSeg *trkseg = (*iter_seg_);

		std::list<gpx::WPT*> &trkpts = trkseg->trkpts().list();

		iter_pts_ = trkpts.begin();

		if (iter_pts_ != trkpts.end()) {
			wpt = (*iter_pts_);

			data.read(wpt); // Init previous & current point with first point
			data.init();	// Init counter

			if (start_time_ == 0)
				setStartTime(data.time());			

			return GPX::DataAgain;
		}
	}

	return GPX::DataEof;
}


enum GPX::Data GPX::retrieveFirst(GPXData &data) {
	enum GPX::Data result;

	log_call();

	eof_ = false;

	// Read first waypoint
	result = retrieveFirst_i(data);

	// Read second waypoint
	retrieveData(data);

//	// Search start waypoint
//	if (from_ != 0) {
//		int64_t timecode_ms = (((int64_t) from_ - start_time_) * 1000); // - offset_;
//
//		result = retrieveNext(data, timecode_ms - 1000);
//
//		data.init();
//
//		data_list_.clear();
//	}

	return result;
}


enum GPX::Data GPX::retrieveFrom(GPXData &data) {
	int64_t timecode_ms;

	enum GPX::Data result;

	result = retrieveFirst(data);

	if (from_ != 0) {
		timecode_ms = (((int64_t) from_ - start_time_) * 1000); // - offset_;

		result = retrieveNext(data, timecode_ms);

		data.init();
	}

	return result;
}


enum GPX::Data GPX::retrieveNext_i(GPXData &data, int64_t timecode_ms) {
	enum GPX::Data result = GPX::DataAgain;

	int64_t timestamp = start_time_ + ((offset_ + timecode_ms) / 1000);

	log_call();

	log_debug("retrieve next timestamp: %ld (filter: %d)", timestamp, filter_);
	log_debug(" curr time: %ld", data.time(GPXData::PositionCurrent));
	log_debug(" next time: %ld", data.time(GPXData::PositionNext));

	do {
		if (timecode_ms == -1) {
//			printf(" <no timecode> ");
			data.update(filter_);

			if (this->retrieveData(data) == GPX::DataEof)
				goto eof;
		}
		else {
			if ((from_ != 0) && (timestamp < (from_ + (offset_ / 1000)))) {
//				printf(" <from> ");
				data.disableCompute();
			}
			else if ((to_ != 0) && (timestamp > (to_ + (offset_ / 1000)))) {
//				printf(" <to>");
				data.disableCompute();
			}

			if (timestamp <= data.time(GPXData::PositionCurrent)) {
//				printf(" <unchanged> ");
				data.unchanged();
			}
			else if (timestamp < data.time(GPXData::PositionNext)) {
//				printf(" <predict> ");
				if (filter_ == TelemetrySettings::FilterNone) {
					data.update(filter_);
			
					if (this->retrieveData(data) == GPX::DataEof)
						goto eof;
				}
				else
					data.predict(filter_);
			}
			else if (timestamp == data.time(GPXData::PositionNext)) {
//				printf(" <update> ");
				data.update(filter_);

				if (this->retrieveData(data) == GPX::DataEof)
					goto eof;
			}
			else if (timestamp > data.time(GPXData::PositionNext)) {
//				printf(" <next> ");
				time_t next_time = data.time(GPXData::PositionCurrent) + 1;

				if (next_time >= data.time(GPXData::PositionNext)) {
					data.update(filter_);
				
					if (this->retrieveData(data) == GPX::DataEof)
						goto eof;
				}
				else {
					if (filter_ == TelemetrySettings::FilterNone) {
						data.update(filter_);
				
						if (this->retrieveData(data) == GPX::DataEof)
							goto eof;
					}
					else
						data.predict(filter_);
				}
			}
		}

		if ((from_ == 0) || ((from_ + (offset_ / 1000)) < data.time(GPXData::PositionCurrent)))
			data.enableCompute();

		if ((to_ != 0) && (data.time(GPXData::PositionCurrent) > (to_ + (offset_ / 1000))))
			goto eof;
	} while (data.time(GPXData::PositionCurrent) < timestamp);
//printf(" <end>\n");
	return result;

eof:
	data.unvalid();
	data.disableCompute();

	return GPX::DataEof;
}


enum GPX::Data GPX::retrieveNext(GPXData &data, int64_t timecode_ms) {
	enum GPX::Data type;

	GPXData next, prev;
	GPXData result;

	log_call();

	if (timecode_ms == -1)
		return this->retrieveNext_i(data, timecode_ms);

	// First call... so extract 2 points
	if (data_list_.size() == 0) {
		type = this->retrieveNext_i(data, timecode_ms);
		if (type != GPX::DataEof)
			data_list_.push_back(data);

		next = data;

		type = this->retrieveNext_i(next, timecode_ms + 1000);
		if (type != GPX::DataEof)
			data_list_.push_back(next);
	}
//	else if ((timecode_ms % 1000) > 0)  {
//		data.unchanged();
//
//		return GPX::DataAgain;
//	}
	else {
		next = data_list_.back();

		type = this->retrieveNext_i(next, timecode_ms + 1000);
		if ((type != GPX::DataEof) && (next.type() != GPXData::TypeUnchanged)) {
			data = data_list_.back();

			data_list_.push_back(next);
		}
		else if (type != GPX::DataEof) {
			data.unchanged();

			return GPX::DataAgain;
		}

//		data = data_list_.back();
//
//		next = data;
//
//		type = this->retrieveNext_i(next, timecode_ms + 1000);
//		if (type != GPX::DataEof)
//			data_list_.push_back(next);
	}

	// Compute
	size_t size = data_list_.size();

	if (type == GPX::DataEof) {
		result = data_list_[size - 1];
	}
	else if (size > 2) {
		prev = data_list_[size - 3];
		next = data_list_[size - 1];
		result = data_list_[size - 2];

		result.smooth(prev, next);
	}
	else {
		result = data_list_[0];
	}

	data = result;

	return type;
}


enum GPX::Data GPX::retrieveData(GPXData &data) {
	gpx::WPT *wpt;
	gpx::TRKSeg *trkseg = (*iter_seg_);

	std::list<gpx::WPT*> &trkpts = trkseg->trkpts().list();
	std::list<gpx::TRKSeg*> &trksegs = trk_->trksegs().list();

	log_call();

	if (eof_)
		goto done;

	// Next point
	iter_pts_++;

	if (iter_pts_ == trkpts.end()) {
		iter_seg_++;

		if (iter_seg_ == trksegs.end())
			goto done;

		trkseg = (*iter_seg_);

		trkpts = trkseg->trkpts().list();
		iter_pts_ = trkpts.begin();
	}

	if (iter_pts_ == trkpts.end())
		goto done;

	wpt = (*iter_pts_);

	data.read(wpt);

	return GPX::DataAgain;

done:
	eof_ = true;

	data = GPXData();

	return GPX::DataEof;
}


enum GPX::Data GPX::retrieveLast(GPXData &data) {
	gpx::WPT *wpt;
	std::list<gpx::TRKSeg*> &trksegs = trk_->trksegs().list();

	log_call();

	data = GPXData();

	iter_seg_ = trksegs.end();

	if (!trksegs.empty()) {
		do {
			iter_seg_--;
		
			gpx::TRKSeg *trkseg = (*iter_seg_);

			std::list<gpx::WPT*> &trkpts = trkseg->trkpts().list();
			iter_pts_ = trkpts.end();

			if (!trkpts.empty()) {
				do {
					iter_pts_--;

					wpt = (*iter_pts_);

					data.read(wpt);
					data.update();

					if (to_ != 0) {
						if (data.time(GPXData::PositionCurrent) <= (to_ + (offset_ / 1000)))
							return GPX::DataAgain;
						else
							continue;
					}

					return GPX::DataAgain;
				} while (iter_pts_ != trkpts.begin());
			}
		} while (iter_seg_ != trksegs.begin());
	}

	eof_ = true;

	return GPX::DataEof;
}


bool GPX::getBoundingBox(GPXData::point *p1, GPXData::point *p2) {
	GPXData::point p;

	std::list<gpx::TRKSeg*> &trksegs = trk_->trksegs().list();

	log_call();

	p1->valid = false;
	p2->valid = false;

	for (std::list<gpx::TRKSeg*>::iterator iter2 = trksegs.begin(); iter2 != trksegs.end(); ++iter2) {
		gpx::TRKSeg *seg = (*iter2);

		std::list<gpx::WPT*> &trkpts = seg->trkpts().list();

		for (std::list<gpx::WPT*>::iterator iter3 = trkpts.begin(); iter3 != trkpts.end(); ++iter3) {
			gpx::WPT *wpt = (*iter3);

			GPXData::convert(&p, wpt);

			if (!p.valid)
				continue;

			if ((from_ != 0) && ((p.ts / 1000) < from_))
				continue;

			if (!p1->valid)
				*p1 = p;
			if (!p2->valid)
				*p2 = p;

			// top-left bounding box
			if (p.lon < p1->lon)
				p1->lon = p.lon;
			if (p.lat > p1->lat)
				p1->lat = p.lat;

			// bottom-right bounding box
			if (p.lon > p2->lon)
				p2->lon = p.lon;
			if (p.lat < p2->lat)
				p2->lat = p.lat;

			if ((to_ != 0) && ((p.ts / 1000) > to_))
				break;
		}
	}

	return (p1->valid && p2->valid);
}


double GPX::getMaxSpeed(void) {
	GPXData data;

	double max_speed = 0.0;
	double last_speed = 0.0;

	std::list<gpx::TRKSeg*> &trksegs = trk_->trksegs().list();

	log_call();

	for (std::list<gpx::TRKSeg*>::iterator iter2 = trksegs.begin(); iter2 != trksegs.end(); ++iter2) {
		gpx::TRKSeg *seg = (*iter2);

		std::list<gpx::WPT*> &trkpts = seg->trkpts().list();

		for (std::list<gpx::WPT*>::iterator iter3 = trkpts.begin(); iter3 != trkpts.end(); ++iter3) {
			gpx::WPT *wpt = (*iter3);

			data.read(wpt);

			if (!data.valid())
				continue;

			// Skip incoherent values
			if (abs(last_speed - data.speed()) > 10)
				continue;

			if (data.speed() > max_speed)
				max_speed = data.speed();

			last_speed = data.speed();
		}
	}

	return max_speed;
}

