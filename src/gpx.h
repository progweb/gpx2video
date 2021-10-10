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
		struct tm time;
	};

	GPXData();
	virtual ~GPXData();

	void read(gpx::WPT *wpt);

	void dump(void);

	const bool& valid(void) const {
		return valid_;
	}

	const double& duration(void) const {
		return duration_;
	}

	const double& distance(void) const {
		return distance_;
	}

	const double& speed(void) const {
		return speed_;
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

protected:
	bool parse(void);

private:
	GPX(std::ifstream &stream, gpx::GPX *root);

    std::ifstream &stream_;

	gpx::GPX *root_;

	gpx::TRK *trk_;

	time_t start_time_;
};

#endif

