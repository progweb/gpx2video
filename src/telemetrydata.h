#ifndef __GPX2VIDEO__TELEMETRYDATA_H__
#define __GPX2VIDEO__TELEMETRYDATA_H__

#include <iostream>
#include <string>


class TelemetrySource;


class TelemetryData {
public:
	friend class TelemetrySource;

	enum Type {
		TypeUnknown,
		TypeMeasured,
		TypeFixed,
		TypePredicted,
		TypeUnchanged,
		TypeError,
	};

	enum Data {
		DataNone = 0,

		DataFix = 1,
		DataElevation = (1 << 1),
		DataCadence = (1 << 2),
		DataHeartrate = (1 << 3),
		DataTemperature = (1 << 4),
		DataPower = (1 << 5),

		DataDuration = (1 << 6),          // activity duration between from & to
		DataDistance = (1 << 7),
		DataGrade = (1 << 8),
		DataSpeed = (1 << 9),
		DataMaxSpeed = (1 << 10),
		DataAcceleration = (1 << 11),
		DataRideTime = (1 << 12),         // duration in moving between from & to
		DataElapsedTime = (1 << 13),      // activity duration between begin & end 
		DataAverageSpeed = (1 << 14),     // average computed for duration time
		DataAverageRideSpeed = (1 << 15), // average computed for ride time
		DataVerticalSpeed = (1 << 16),
		DataHeading = (1 << 17),

		DataAll = (1 << 17) -1
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
			"F", // Fix
			"P", // Predict
			"C", // Unchanged
			"E", // Error
		};

		if (type_ > ARRAY_SIZE(types))
			return "";

		return types[type_];
	}


	const uint64_t& datetime(void) const {
		return datetime_;
	}

	void setDatetime(const uint64_t &datetime) {
		datetime_ = datetime;
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

	const int& power(void) const {
		return power_;
	}

	const double &duration(void) const {
		return duration_;
	}

	const double& distance(void) const {
		return distance_;
	}

	const double& heading(void) const {
		return heading_;
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

	const double& acceleration(void) const {
		return acceleration_;
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

	const double& verticalspeed(void) const {
		return verticalspeed_;
	}

	const int& lap(void) const {
		return lap_;
	}

	bool inRange(void) const {
		return in_range_;
	}

	bool isPause(void) const {
		return is_pause_;
	}

	bool hasValue(Data type = DataAll) const {
		return ((has_value_ & type) != 0);
	}

	void reset(bool all = false);

	void dump(void);

	static void writeHeader(void);
	void writeData(size_t index) const;

protected:
	int has_value_;

	bool in_range_;
	bool is_pause_;

	uint32_t line_;

	Type type_;

	uint64_t ts_;
	double lat_, lon_;
	double raw_lat_, raw_lon_;
	double ele_;
	double temperature_;
	int heartrate_;
	int cadence_;
	int power_;

	uint64_t datetime_;
	double distance_;
	double heading_;
	double duration_;
	double grade_;
	double speed_;
	double maxspeed_;
	double acceleration_;
	double ridetime_;
	double elapsedtime_;
	double avgspeed_;
	double avgridespeed_;
	double verticalspeed_;

	int lap_;
	bool in_lap_;
};


#endif

