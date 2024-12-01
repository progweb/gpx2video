#ifndef __GPX2VIDEO__TELEMETRYMEDIA_H__
#define __GPX2VIDEO__TELEMETRYMEDIA_H__

#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <deque>

#include "log.h"
#include "kalman.h"
#include "telemetrydata.h"
#include "telemetrysettings.h"


class TelemetryMedia;
class TelemetrySource;


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
			else if (type == "F") 
				type_ = TypeFixed;
			else if (type == "P") 
				type_ = TypePredicted;
			else if (type == "C")
				type_ = TypeUnchanged;
			else
				type_ = TypeError;
		}

		void setComputed(bool computed) {
			in_range_ = computed;
		}

		void setPause(bool pause) {
			is_pause_ = pause;
		}

		void setTimestamp(uint64_t ts) {
			ts_ = ts;
		}

		void setPosition(uint64_t ts, double lat, double lon) {
			ts_ = ts;
			raw_lat_ = lat;
			raw_lon_ = lon;

			setPosition(lat, lon);

			setValue(Data::DataFix);
		}

		void setPosition(double lat, double lon) {
			lat_ = lat;
			lon_ = lon;
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

		void setPower(int power) {
			power_ = power;

			addValue(Data::DataPower);
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

		void setAcceleration(double acceleration) {
			acceleration_ = acceleration;

			addValue(Data::DataAcceleration);
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

		void clearValue(int type) {
			has_value_ = has_value_ & ~type;
		}

		void restore(Point point, bool flags=false) {
			int mask = DataNone;

			// Restore previous data & flags:
			//  distance, grade, speed, maxspeed, 
			//  duration, ridetime, elaspedtime,
			//  avgspeed, avgridespeed
			//  lap, in_lap
			if (!hasValue(TelemetryData::DataFix)) {
				lat_ = point.lat_;
				lon_ = point.lon_;
				mask |= (point.has_value_ & DataFix);
			}

			if (!hasValue(TelemetryData::DataElevation)) {
				ele_ = point.ele_;
				mask |= (point.has_value_ & DataElevation);
			}

			if (!hasValue(TelemetryData::DataCadence)) {
				cadence_ = point.cadence_;
				mask |= (point.has_value_ & DataCadence);
			}

			if (!hasValue(TelemetryData::DataHeartrate)) {
				heartrate_ = point.heartrate_;
				mask |= (point.has_value_ & DataHeartrate);
			}

			if (!hasValue(TelemetryData::DataTemperature)) {
				temperature_ = point.temperature_;
				mask |= (point.has_value_ & DataTemperature);
			}

			if (!hasValue(TelemetryData::DataPower)) {
				power_ = point.power_;
				mask |= (point.has_value_ & DataPower);
			}

			if (!hasValue(TelemetryData::DataGrade)) {
				grade_ = point.grade_;
				mask |= (point.has_value_ & DataGrade);
			}

			if (!hasValue(TelemetryData::DataDistance)) {
				distance_ = point.distance_;
				mask |= (point.has_value_ & DataDistance);
			}

			if (!hasValue(TelemetryData::DataSpeed)) {
				speed_ = point.speed_;
				mask |= (point.has_value_ & DataSpeed);
			}

			if (!hasValue(TelemetryData::DataMaxSpeed)) {
				maxspeed_ = point.maxspeed_;
				mask |= (point.has_value_ & DataMaxSpeed);
			}

			if (!hasValue(TelemetryData::DataAcceleration)) {
				speed_ = point.acceleration_;
				mask |= (point.has_value_ & DataAcceleration);
			}

			if (!hasValue(TelemetryData::DataDuration)) {
				duration_ = point.duration_;
				mask |= (point.has_value_ & DataDuration);
			}

			if (!hasValue(TelemetryData::DataRideTime)) {
				ridetime_ = point.ridetime_;
				mask |= (point.has_value_ & DataRideTime);
			}

			if (!hasValue(TelemetryData::DataElapsedTime)) {
				elapsedtime_ = point.elapsedtime_;
				mask |= (point.has_value_ & DataElapsedTime);
			}

			if (!hasValue(TelemetryData::DataAverageSpeed)) {
				avgspeed_ = point.avgspeed_;
				mask |= (point.has_value_ & DataAverageSpeed);
			}

			if (!hasValue(TelemetryData::DataAverageRideSpeed)) {
				avgridespeed_ = point.avgridespeed_;
				mask |= (point.has_value_ & DataAverageRideSpeed);
			}

			lap_ = point.lap_;
			in_lap_ = point.in_lap_;

			if (flags)
				setValue(has_value_ | (point.has_value_ & mask));
		}

		double distanceTo(const Point &to);
	};

	class PointPool {
	public:
		PointPool()
			: nbr_points_max_(0)
			, index_(-1) {
		}

		virtual ~PointPool() {
		}

		void setNumberOfPoints(const unsigned long number) {
			log_warn("PointPool::setNumberOfPoints not yet supported");

			(void) number;
//			nbr_points_max_ = number;
		}

		bool empty(void) {
			return (index_ == (int) points_.size());
		}

		size_t count(void) {
			return points_.size();
		}

		size_t size(void) {
			ssize_t size = points_.size();

			size -= index_ + 1;

			return MAX(0, size);
		}

		size_t backlog(void) {
			return (index_ > 0) ? index_ : 0;
		}

		int tell(void) {
			return index_;
		}

		void seek(int offset) {
			index_ += offset;

			if (index_ < 0)
				index_ = -1;
		}

		void clear(void) {
			index_ = -1;

			points_.clear();
		}

		void insert(Point &point) {
			auto pos = points_.begin();
			points_.insert(std::next(pos, index_ + 1), point);
		}

		void push(Point &point) {
			points_.emplace_back(point);

			if (nbr_points_max_ > 0) {
				while (points_.size() > nbr_points_max_) {
					index_--;
					points_.pop_front();
				}
			}

			if (index_ < 0)
				index_ = -1;
		}

		void remove(void) {
			auto pos = points_.begin();
			points_.erase(std::next(pos, index_), std::next(pos, index_ + 1));
		}

		void reset(void) {
			seek(-backlog() - 1);
		}

		void dump(bool content=false) {
			TelemetrySource::Point point;

			std::cout << "Pool info:" << std::endl;
			std::cout << "  index: " << index_ << std::endl;
			std::cout << "  backlog + size / count: " << backlog() << " + " << size() << " / " << count() << std::endl;

			if (!content)
				return;

			TelemetryData::writeHeader();

			for (size_t i=0; i<points_.size(); i++) {
				point = points_[i];

				point.writeData(i);
			}
		}

		Point& first(void) {
			return points_.front();
		}

		Point& last(void) {
			return points_.back();
		}

		Point& previous(size_t index = 0) {
			size_t n = 0;

			for (int i=1; (size_t) i<=backlog(); i++) {
				if (points_[index_ - i].type() == TelemetryData::TypeError)
					continue;

				if (index > n++)
					continue;

				return points_[index_ - i];
			}

			return default_;
		}

		Point& current() {
			return points_[index_];
		}

		Point& next(size_t index=0) {
			size_t n = 0;

			for (size_t i=index_ + 1; i<points_.size(); i++) {
				if (points_[i].type() == TelemetryData::TypeError)
					continue;

				if (index > n++)
					continue;

				return points_[i];
			}

			return default_;
		}

		Point& operator [](int index) {
			return points_.at(index_ + index);
		}

		Point operator [](int index) const {
			return points_.at(index_ + index);
		}

	private:
		size_t nbr_points_max_;

		int index_;

		std::deque<Point> points_;

		Point default_;
	};

	TelemetrySource(const std::string &filename);
	virtual ~TelemetrySource();

	// Setter
	void setQuiet(const bool quiet);

	void setNumberOfPoints(const unsigned long number);

	void skipBadPoint(bool check);

	void setFilter(enum TelemetrySettings::Filter method=TelemetrySettings::FilterNone);
	void setMethod(enum TelemetrySettings::Method method=TelemetrySettings::MethodNone);

	void setSmoothMethod(enum TelemetryData::Data type, enum TelemetrySettings::Smooth method);
	void setSmoothPoints(enum TelemetryData::Data type, int number);

	bool setDataRange(std::string begin, std::string end);
	bool setComputeRange(std::string from, std::string to);

	int64_t timeOffset(void) const;
	void setTimeOffset(const int64_t& offset);

	// Getter
	bool inRange(uint64_t timestamp) const;

	uint64_t beginTimestamp(void) const;
	uint64_t endTimestamp(void) const;

	bool getBoundingBox(TelemetryData *p1, TelemetryData *p2);

	// Data api
	enum Data loadData(void);

	enum Data retrieveFirst(TelemetryData &data);
	enum Data retrieveFrom(TelemetryData &data);
	enum Data retrieveNext(TelemetryData &data, uint64_t timestamp=-1);
	enum Data retrieveData(TelemetryData &data);
	enum Data retrieveLast(TelemetryData &data);
	enum Data retrieveTo(TelemetryData &data);

	void dump(bool content);

	// Parser implementation
	virtual std::string name(void) = 0;
	virtual void reset(void) = 0;
	virtual enum Data read(Point &point) = 0;

private:
	void push(Point &pt);

	void clear(void);
	bool load(void);
	void range(void);
	void filter(void);
	void compute_i(TelemetryData &data, bool force=false);
	void compute(void);
	void smooth_step_one(void);
	void smooth_step_two(void);
	void smooth(void);
	void fix(void);
	void trim(void);

	void insertData(uint64_t timestamp);
	void updateData(TelemetryData &data);
	void predictData(TelemetryData &data, TelemetrySettings::Method method, uint64_t timestamp);
	void cleanData(TelemetryData &data, uint64_t timestamp);

protected:
	std::ifstream stream_;

	Point start_;

	PointPool pool_;

	bool quiet_;

	bool eof_;
	bool check_;

	int rate_;
	int64_t offset_;

	uint64_t begin_, end_;
	uint64_t from_, to_;

	enum TelemetrySettings::Filter filter_;
	enum TelemetrySettings::Method method_;

	enum TelemetrySettings::Smooth smooth_grade_method_;
	int smooth_grade_points_;

	enum TelemetrySettings::Smooth smooth_speed_method_;
	int smooth_speed_points_;

	enum TelemetrySettings::Smooth smooth_elevation_method_;
	int smooth_elevation_points_;

	enum TelemetrySettings::Smooth smooth_acceleration_method_;
	int smooth_acceleration_points_;

	KalmanFilter kalman_;
};


class TelemetryMedia {
public:
	static TelemetrySource * open(const std::string &filename, const TelemetrySettings &settings, bool quiet=true);

	//void dump(void);
};

#endif

