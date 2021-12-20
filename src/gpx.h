#ifndef __GPX2VIDEO__GPX_H__
#define __GPX2VIDEO__GPX_H__

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "gpxlib/Parser.h"
#include "gpxlib/ReportCerr.h"


class GPXData {
public:
	struct point {
		bool valid;
		double lat, lon;
		double x, y;
		double ele;
		time_t time;
	};

	GPXData();
	virtual ~GPXData();

	void read(gpx::WPT *wpt);

	void dump(void);

	const struct point& position(void) const {
		return cur_pt_;
	}

	const bool& valid(void) const {
		return valid_;
	}

	const double& duration(void) const {
		return duration_;
	}

	const double& distance(void) const {
		return distance_;
	}

	const double& elevation(void) const {
		return cur_pt_.ele;
	}

	const double& grade(void) const {
		return grade_;
	}

	const double& speed(void) const {
		return speed_;
	}

	const double& temperature(void) const {
		return temperature_;
	}

	const int& cadence(void) const {
		return cadence_;
	}

	const int& heartrate(void) const {
		return heartrate_;
	}

	static void convert(struct point *pt, gpx::WPT *wpt);

protected:
	bool compute(void);

	int nbr_points_;
	struct point cur_pt_;
	struct point prev_pt_;

	bool valid_;
	double duration_;
	double distance_;
	double speed_;
	double grade_;
	double temperature_;
	int heartrate_;
	int cadence_;
};


class GPX {
public:
	virtual ~GPX();

	static GPX * open(const std::string &filename);

	void dump(void);

	void setStartTime(char *start_time);
	void setStartTime(time_t start_time);
	void setStartTime(struct tm *start_time);
	const GPXData retrieveData(const int64_t &timecode);
	bool getBoundingBox(GPXData::point *p1, GPXData::point *p2);
	double getMaxSpeed(void);

	void retrieveFirst(GPXData &data);
	void retrieveNext(GPXData &data);

protected:
	bool parse(void);

private:
	GPX(std::ifstream &stream, gpx::GPX *root);

    std::ifstream &stream_;

	gpx::GPX *root_;

	gpx::TRK *trk_;

	std::list<gpx::WPT*>::iterator iter_pts_;
	std::list<gpx::TRKSeg*>::iterator iter_seg_;

	time_t start_time_;
};

#endif

