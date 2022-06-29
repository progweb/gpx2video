#ifndef __GPX2VIDEO__GPX_H__
#define __GPX2VIDEO__GPX_H__

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "kalman.h"
#include "gpxlib/Parser.h"
#include "gpxlib/ReportCerr.h"
#include "telemetrysettings.h"


class GPXData {
public:
	struct point {
		bool valid;
		double lat, lon;
		double x, y;
		double ele;
		time_t time;
	};

	enum Position {
		PositionCurrent,	// Interpolated position in the GPX stream
		PositionPrevious,	// Last read position in the GPX stream
		PositionNext		// Next read position in the GPX stream
	};

	GPXData();
	virtual ~GPXData();

	void init(void);
	void predict(enum TelemetrySettings::Filter filter=TelemetrySettings::FilterNone);
	void update(enum TelemetrySettings::Filter filter=TelemetrySettings::FilterNone);

	void read(gpx::WPT *wpt);

	bool compute(void);

	void dump(void);

	int line(void) const {
		return line_;
	}

	const time_t& time(Position p = PositionCurrent) const {
		if (p == PositionCurrent)
			return cur_pt_.time;
		if (p == PositionPrevious)
			return prev_pt_.time;

		return next_pt_.time;
	}

	const struct point& position(Position p = PositionCurrent) const {
		if (p == PositionCurrent)
			return cur_pt_;
		if (p == PositionPrevious)
			return prev_pt_;

		return next_pt_;
	}

	const bool& valid(void) const {
		return valid_;
	}

	const int& elapsedTime(void) const {
		return elapsedtime_;
	}

	void setElapsedTime(const int &time) {
		elapsedtime_ = time;
	}

	const double& duration(void) const {
		return duration_;
	}

	const double& distance(void) const {
		return distance_;
	}

	const double& elevation(Position p = PositionCurrent) const {
		if (p == PositionCurrent)
			return cur_pt_.ele;
		if (p == PositionPrevious)
			return prev_pt_.ele;

		return next_pt_.ele;
	}

	const double& grade(void) const {
		return grade_;
	}

	const double& speed(void) const {
		return speed_;
	}

	const double& maxspeed(void) const {
		return maxspeed_;
	}

	const double& avgspeed(void) const {
		return avgspeed_;
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
	int nbr_points_;
	struct point cur_pt_;
	struct point prev_pt_;
	struct point next_pt_;

	int nbr_predictions_;
	KalmanFilter filter_;

	int line_;
	bool valid_;
	int elapsedtime_;
	double duration_;
	double distance_;
	double speed_;
	double maxspeed_;
	double avgspeed_;
	double grade_;
	double temperature_;
	int heartrate_;
	int cadence_;
};


class GPX {
public:
	enum Data {
		DataMeasured,
		DataPredicted,
		DataUnchanged,
		DataEof
	};

	virtual ~GPX();

	static GPX * open(const std::string &filename, enum TelemetrySettings::Filter filter=TelemetrySettings::FilterNone);

	void dump(void);

	void setStartTime(char *start_time);
	void setStartTime(time_t start_time);
	void setStartTime(struct tm *start_time);

	int timeOffset(void) const;
	void setTimeOffset(const int& offset);

//	const GPXData retrieveData(const int64_t &timecode);
	bool getBoundingBox(GPXData::point *p1, GPXData::point *p2);
	double getMaxSpeed(void);

	enum Data retrieveFirst(GPXData &data);
	enum Data retrieveNext(GPXData &data, int timestamp=-1);
	enum Data retrieveData(GPXData &data);
	enum Data retrieveLast(GPXData &data);

protected:
	bool parse(void);

private:
	GPX(std::ifstream &stream, gpx::GPX *root, enum TelemetrySettings::Filter filter);

    std::ifstream &stream_;

	gpx::GPX *root_;

	gpx::TRK *trk_;

	std::list<gpx::WPT*>::iterator iter_pts_;
	std::list<gpx::TRKSeg*>::iterator iter_seg_;

	int offset_;
	time_t start_time_;
	time_t start_activity_;

	enum TelemetrySettings::Filter filter_;
};

#endif

