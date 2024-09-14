#ifndef __GPX2VIDEO__TELEMETRYMEDIA_H__
#define __GPX2VIDEO__TELEMETRYMEDIA_H__

#include <fstream>
#include <iostream>
#include <string>
#include <deque>

#include "kalman.h"
#include "telemetrysettings.h"


std::string timestamp2string(uint64_t value);


class TelemetryData;
class TelemetryMedia;
class TelemetrySource;


class TelemetryData {
public:
	friend class TelemetrySource;

	enum Type {
		TypeUnknown,
		TypeMeasured,
		TypePredicted,
		TypeUnchanged,
	};

	enum Data {
		DataNone = 0,

		DataFix = 1,
		DataElevation = (1 << 1),
		DataCadence = (1 << 2),
		DataHeartrate = (1 << 3),
		DataTemperature = (1 << 4),

		DataDuration = (1 << 5),
		DataDistance = (1 << 6),
		DataGrade = (1 << 7),
		DataSpeed = (1 << 8),
		DataMaxSpeed = (1 << 8),
		DataRideTime = (1 << 10),
		DataElapsedTime = (1 << 11),
		DataAverageSpeed = (1 << 12),
		DataAverageRideSpeed = (1 << 13),

		DataAll = (1 << 14) -1
	};

	TelemetryData();
	virtual ~TelemetryData();

	int line(void) const {
		return line_;
	}

	const Type& type(void) const {
		return type_;
	}

	const char * type2string(void) const {
		const char *types[] = {
			"U", // Unknown
			"M", // Measured
			"P", // Predict
			"C", // Unchanged
		};

		return types[type_];
	}

	const time_t& time(void) const {
		static time_t result;

		result = ts_ / 1000;

		return result;
	}

	const uint64_t& timestamp(void) const {
		return ts_;
	}

	const double& latitude(void) const {
		return lat_;
	}

	const double& longitude(void) const {
		return lon_;
	}

	const double& elevation(void) const {
		return ele_;
	}

	const int& cadence(void) const {
		return cadence_;
	}

	const int& heartrate(void) const {
		return heartrate_;
	}

	const double& temperature(void) const {
		return temperature_;
	}

	const double &duration(void) const {
		return duration_;
	}

	const double& distance(void) const {
		return distance_;
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

	const double& rideTime(void) const {
		return ridetime_;
	}

	const double& elapsedTime(void) const {
		return elapsedtime_;
	}

	const double& avgspeed(void) const {
		return avgspeed_;
	}

	const double& avgridespeed(void) const {
		return avgridespeed_;
	}

	const int& lap(void) const {
		return lap_;
	}

	bool hasValue(Data type = DataAll) const {
		return ((has_value_ & type) == type);
	}

	void reset(bool all = false);
	void dump(bool debug = false);

protected:
	int has_value_;

	uint32_t line_;

	Type type_;

	uint64_t ts_;
	double lat_, lon_;
	double ele_;
	double temperature_;
	int heartrate_;
	int cadence_;

	double distance_;
	double duration_;
	double grade_;
	double speed_;
	double maxspeed_;
	double ridetime_;
	double elapsedtime_;
	double avgspeed_;
	double avgridespeed_;

	int lap_;
	bool in_lap_;
};


class TelemetrySource {
public:
	enum Data {
		DataUnknown,
		DataAgain,
		DataEof
	};

	class Point : public TelemetryData {
	public:
		Point()
			: TelemetryData() {
		};

		Point(uint64_t ts, double lat, double lon)
			: TelemetryData() {
			setPosition(ts, lat, lon);
		};

		~Point() {
		}

		void setLine(uint32_t line) {
			line_ = line;
		}

		void setType(Type type) {
			type_ = type;
		}

		void setType(std::string type) {
			if (type == "U")
				type_ = TypeUnknown;
			else if (type == "M")
				type_ = TypeMeasured;
			else if (type == "P") 
				type_ = TypePredicted;
			else if (type == "C")
				type_ = TypeUnchanged;
		}

		void setPosition(uint64_t ts, double lat, double lon) {
			ts_ = ts;
			lat_ = lat;
			lon_ = lon;

			setValue(Data::DataFix);
		}

		void setElevation(double ele) {
			ele_ = ele;

			addValue(Data::DataElevation);
		}

		void setCadence(int cadence) {
			cadence_ = cadence;

			addValue(Data::DataCadence);
		}

		void setHeartrate(int heartrate) {
			heartrate_ = heartrate;

			addValue(Data::DataHeartrate);
		}

		void setTemperature(int temperature) {
			temperature_ = temperature;

			addValue(Data::DataTemperature);
		}

		void setDuration(double duration) {
			duration_ = duration;

			addValue(Data::DataDuration);
		}

		void setDistance(double distance) {
			distance_ = distance;

			addValue(Data::DataDistance);
		}

		void setGrade(double grade) {
			grade_ = grade;

			addValue(Data::DataGrade);
		}

		void setSpeed(double speed) {
			speed_ = speed;

			addValue(Data::DataSpeed);
		}

		void setMaxSpeed(double speed) {
			maxspeed_ = speed;

			addValue(Data::DataMaxSpeed);
		}

		void setRideTime(double ridetime) {
			ridetime_ = ridetime;

			addValue(Data::DataRideTime);
		}

		void setElapsedTime(double elapsedtime) {
			elapsedtime_ = elapsedtime;

			addValue(Data::DataElapsedTime);
		}

		void setAverageSpeed(double speed) {
			avgspeed_ = speed;

			addValue(Data::DataAverageSpeed);
		}

		void setAverageRideSpeed(double speed) {
			avgridespeed_ = speed;

			addValue(Data::DataAverageRideSpeed);
		}

		void setLap(int lap) {
			lap_ = lap;
		}

		void setValue(int type) {
			has_value_ = type;
		}

		void addValue(Data type) {
			has_value_ |= type;
		}

		void restore(Point point) {
			int mask = DataNone;

			// Restore previous data & flags:
			//  distance, grade, speed, maxspeed, 
			//  duration, ridetime, elaspedtime,
			//  avgspeed, avgridespeed
			//  lap, in_lap
			if (!hasValue(TelemetryData::DataDistance)) {
				distance_ = point.distance_;
				mask |= DataDistance;
			}

			if (!hasValue(TelemetryData::DataGrade)) {
				grade_ = point.grade_;
				mask |= DataGrade;
			}

			if (!hasValue(TelemetryData::DataSpeed)) {
				speed_ = point.speed_;
				mask |= DataSpeed;
			}

			if (!hasValue(TelemetryData::DataMaxSpeed)) {
				maxspeed_ = point.maxspeed_;
				mask |= DataMaxSpeed;
			}

			if (!hasValue(TelemetryData::DataDuration)) {
				duration_ = point.duration_;
				mask |= DataDuration;
			}

			if (!hasValue(TelemetryData::DataRideTime)) {
				ridetime_ = point.ridetime_;
				mask |= DataRideTime;
			}

			if (!hasValue(TelemetryData::DataElapsedTime)) {
				elapsedtime_ = point.elapsedtime_;
				mask |= DataElapsedTime;
			}

			if (!hasValue(TelemetryData::DataAverageSpeed)) {
				avgspeed_ = point.avgspeed_;
				mask |= DataAverageSpeed;
			}

			if (!hasValue(TelemetryData::DataAverageRideSpeed)) {
				avgridespeed_ = point.avgridespeed_;
				mask |= DataAverageRideSpeed;
			}

			lap_ = point.lap_;
			in_lap_ = point.in_lap_;

//			setValue(has_value_ | (point.has_value_ & mask));
		}
	};

	TelemetrySource(const std::string &filename);
	virtual ~TelemetrySource();

	void setNumberOfPoints(const unsigned long number);

	void setMethod(enum TelemetrySettings::Method method=TelemetrySettings::MethodNone);

	bool setFrom(std::string from);
	bool setTo(std::string to);

	void setStartTime(char *start_time);
	void setStartTime(time_t start_time);
	void setStartTime(struct tm *start_time);

	int64_t timeOffset(void) const;
	void setTimeOffset(const int64_t& offset);

	bool getBoundingBox(TelemetryData *p1, TelemetryData *p2);

	enum Data retrieveFirst(TelemetryData &data);
	enum Data retrieveFrom(TelemetryData &data);
	enum Data retrieveNext(TelemetryData &data, uint64_t timestamp=-1);
	enum Data retrieveData(TelemetryData &data);
	enum Data retrieveLast(TelemetryData &data);

	virtual void reset() = 0;
	virtual enum Data read(Point &point) = 0;

private:
	void push(Point &pt);
	void compute(TelemetryData &data);
	void update(TelemetryData &data);
	void predict(TelemetryData &data);

	void enableCompute(void) {
		enable_ = true;
	}

	void disableCompute(void) {
		enable_ = false;
	}

protected:
	std::ifstream stream_;

	TelemetryData start_;

	std::deque<Point> points_;

	unsigned long nbr_points_max_;

	bool enable_;

	int64_t offset_;

	uint64_t from_;
	uint64_t to_;

	enum TelemetrySettings::Method method_;

	KalmanFilter kalman_;
};


class TelemetryMedia {
public:
	static TelemetrySource * open(const std::string &filename, enum TelemetrySettings::Method method=TelemetrySettings::MethodNone);

	//void dump(void);
};

#endif

