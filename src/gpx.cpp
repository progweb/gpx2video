#include <fstream>
#include <iostream>
#include <string>

#include <string.h>
//#define __USE_XOPEN  // For strptime
#include <time.h>
#include <math.h>

#include "utmconvert/utmconvert.h"

#include "gpxlib/Parser.h"
#include "gpxlib/ReportCerr.h"

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




GPXData::GPXData() 
	: nbr_points_(0)
	, valid_(false)
	, duration_(0)
	, distance_(0)
	, speed_(0) 
	, grade_(0) {
}


GPXData::~GPXData() {
}


void GPXData::dump(void) {
	char s[128];

	struct tm time;

	localtime_r(&cur_pt_.time, &time);

	strftime(s, sizeof(s), "%Y-%m-%d %H:%M:%S", &time);

	printf("  Time: %s. Distance: %.3f km in %.3f seconds, current speed is %.3f (valid: %s)\n",
		s,
		distance_/1000.0, duration_, speed_, 
		valid() ? "true" : "false");  
}


void GPXData::convert(struct GPXData::point *pt, gpx::WPT *wpt) {
	char **end;
	const char *s;

	struct tm time;

	struct Utm_val utm;

	pt->valid = true;

	// Convert time - GPX file contains UTC time
	s = wpt->time().getValue().c_str();

	// Try format: "2020:12:13 08:55:48.215"
	memset(&time, 0, sizeof(time));
	if (strptime(s, "%Y:%m:%d %H:%M:%S.", &time) != NULL)
		pt->time = timegm(&time);
	// Try format: "2020-07-28T07:04:43.000Z"
	else if (strptime(s, "%Y-%m-%dT%H:%M:%S.", &time) != NULL)
		pt->time = timegm(&time);
	else
		pt->valid = false;

	// Convert lat, lon & ele
	pt->lat = strtod(wpt->lat().getValue().c_str(), (char**)&end);
	pt->lon = strtod(wpt->lon().getValue().c_str(), (char**)&end);
	pt->ele = strtod(wpt->ele().getValue().c_str(), (char**)&end);

	utm = to_utm(pt->lat, pt->lon);

	// Return result
	pt->x = utm.x;
	pt->y = utm.y;
}


bool GPXData::compute(void) {
	double dx = (cur_pt_.x - prev_pt_.x);
	double dy = (cur_pt_.y - prev_pt_.y);
	double dz = (cur_pt_.ele - prev_pt_.ele);

	double dc = sqrt(dx*dx + dy*dy + dz*dz);
	double dt = difftime(cur_pt_.time, prev_pt_.time);

	duration_ += dt;
	distance_ += dc;
	speed_ = (3600 * dc) / (1000 * dt);

	dc = sqrt(dx*dx + dy*dy);

	grade_ = 100 * dz / dc;

	return true;
}


void GPXData::read(gpx::WPT *wpt) {
	struct point pt;

	convert(&pt, wpt);

	// Skip point if not valid
	if (!pt.valid)
		return;

	memcpy(&prev_pt_, &cur_pt_, sizeof(prev_pt_));
	memcpy(&cur_pt_, &pt, sizeof(cur_pt_));

	nbr_points_++;

	if (nbr_points_ > 1)
		compute();

	valid_ = true;
}


GPX::GPX(std::ifstream &stream, gpx::GPX *root) 
	: stream_(stream)
	, root_(root) {
}


GPX::~GPX() {
	if (stream_.is_open())
		stream_.close();
}


GPX * GPX::open(const std::string &filename) {
	GPX *gpx = NULL;

	gpx::GPX *root;
//	gpx::ReportCerr report;
	gpx::Parser parser(NULL); //&report);

    std::ifstream stream = std::ifstream(filename);

	if (!stream.is_open())
		goto failure;

	root = parser.parse(stream);

	if (root == NULL) {
		std::cerr << "Parsing of " << filename << " failed due to " << parser.errorText() << " on line " << parser.errorLineNumber() << " and column " << parser.errorColumnNumber() << std::endl;
		goto failure;
	}

	gpx = new GPX(stream, root);

	gpx->parse();

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

	for (std::list<gpx::TRK*>::iterator node = trks.begin(); node != trks.end(); ++node) {
		gpx::TRK *trk = (*node);
         
		if (trk == nullptr)
 			continue;

		trk_ = trk;

		// Parse only the first track
		break;
	}

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


void GPX::retrieveFirst(GPXData &data) {
	gpx::WPT *wpt;
	std::list<gpx::TRKSeg*> &trksegs = trk_->trksegs().list();

	data = GPXData();

	iter_seg_ = trksegs.begin();
	
	if (iter_seg_ != trksegs.end()) {
		gpx::TRKSeg *trkseg = (*iter_seg_);

		std::list<gpx::WPT*> &trkpts = trkseg->trkpts().list();
		iter_pts_ = trkpts.begin();

		if (iter_pts_ != trkpts.end()) {
			wpt = (*iter_pts_);

			data.read(wpt);
		}
	}
}


void GPX::retrieveNext(GPXData &data) {
	gpx::WPT *wpt;
	gpx::TRKSeg *trkseg = (*iter_seg_);

	std::list<gpx::WPT*> &trkpts = trkseg->trkpts().list();
	std::list<gpx::TRKSeg*> &trksegs = trk_->trksegs().list();

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

	return;

done:
	data = GPXData();
	return;
}


const GPXData GPX::retrieveData(const int64_t &timecode) {
	GPXData data;

	std::list<gpx::TRKSeg*> &trksegs = trk_->trksegs().list();

	for (std::list<gpx::TRKSeg*>::iterator iter2 = trksegs.begin(); iter2 != trksegs.end(); ++iter2) {
		int n = 0;
		gpx::TRKSeg *seg = (*iter2);

		std::list<gpx::WPT*> &trkpts = seg->trkpts().list();

		for (std::list<gpx::WPT*>::iterator iter3 = trkpts.begin(); iter3 != trkpts.end(); ++iter3) {
			const char *s;

			time_t t;
			struct tm time;

			gpx::WPT *wpt = (*iter3);

			n++;
			data.read(wpt);

			// Convert time
			s = wpt->time().getValue().c_str();

			// Try format: "2020:12:13 08:55:48.215"
			memset(&time, 0, sizeof(time));
			if (strptime(s, "%Y:%m:%d %H:%M:%S.", &time) != NULL)
				t = timegm(&time);
			// Try format: "2020-07-28T07:04:43.000Z"
			else if (strptime(s, "%Y-%m-%dT%H:%M:%S.", &time) != NULL)
				t = timegm(&time);
			else
				continue;

			// Search timecode in the GPX stream
			if (t < (start_time_ + (timecode / 1000))) {
//				printf("t = %ld vs %ld\n", t, start_time_ + (timecode / 1000));
				continue;	   
			}

//			printf("OK %d\n", n);

			return data;
		}
	}

	printf("FAILURE\n");

	return data;
}


bool GPX::getBoundingBox(GPXData::point *p1, GPXData::point *p2) {
	GPXData::point p;

	std::list<gpx::TRKSeg*> &trksegs = trk_->trksegs().list();

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
		}
	}

	return (p1->valid && p2->valid);
}

double GPX::getMaxSpeed(void) {
	GPXData data;

	double max_speed = 0.0;
	double last_speed = 0.0;

	std::list<gpx::TRKSeg*> &trksegs = trk_->trksegs().list();

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

