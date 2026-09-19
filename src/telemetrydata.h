#ifndef __GPX2VIDEO__TELEMETRYDATA_H__
#define __GPX2VIDEO__TELEMETRYDATA_H__

#include <iostream>
#include <string>

#include "macros.h"


class TelemetrySource;


class TelemetryData {
public:
	friend class TelemetrySource;

	enum Type {
		TypeUnknown,
		TypeDummy,
		TypeMeasured,
		TypeFixed,
		TypePredicted,
		TypeUnchanged,
		TypeError
	};

	enum Range {
		RangeMin,
		RangeMax,
		RangeNone
	};

	enum Data {
		DataNone = 0,

		DataFix = 1,
		DataPosition = 1,
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
		DataCourse = (1 << 17),
		DataHeading = (1 << 18),
		DataHomeDistance = (1 << 19),
		DataBatteryLevel = (1 << 20),

		DataAll = (1 << 21) -1
	};

	enum Unit {
		UnitNone,

		UnitDefault,

		UnitPercent,
		UnitDegree,

		UnitMeterPerSec, // Meter / Sec
		UnitMilesPerSec, // Miles / Sec
		UnitFeetPerSec, // Feet / Sec

		UnitMeterPerMin, // Meter / Min
		UnitMilesPerMin, // Miles / Min
		UnitFeetPerMin, // Feet / Min

		UnitKmPerHour, // Km / Hour
		UnitMeterPerHour, // Meter / Hour
		UnitMilesPerHour, // Miles / Hour
		UnitFeetPerHour, // Feet / Hour

		UnitMinPerMile, // Min / Mile
		UnitMinPerKm, // Min / Km

		UnitKm,
		UnitMeter,
		UnitFeet,
		UnitMiles,

		UnitCelsius,
		UnitFarenheit,

		UnitBPM,
		UnitTrPerMin,
		UnitWatt,

		UnitG,
		UnitMeterPerSec2,

		UnitUnknown
	};

	TelemetryData();
	virtual ~TelemetryData();

	int line(void) const;

	int index(void) const;
	void setIndex(int index);

	const Type& type(void) const;
	const char * type2string(void) const;

	const uint64_t& datetime(void) const;
	void setDatetime(const uint64_t &datetime);

	const uint64_t& timestamp(void) const;
	const double& latitude(bool raw=false) const;
	const double& longitude(bool raw=false) const;

	double elevation(TelemetryData::Unit unit = TelemetryData::UnitDefault, TelemetryData::Range range = TelemetryData::RangeNone) const;
	int cadence(TelemetryData::Unit unit = TelemetryData::UnitDefault, TelemetryData::Range range = TelemetryData::RangeNone) const;
	int heartrate(TelemetryData::Unit unit = TelemetryData::UnitDefault, TelemetryData::Range range = TelemetryData::RangeNone) const;
	double temperature(TelemetryData::Unit unit = TelemetryData::UnitDefault, TelemetryData::Range range = TelemetryData::RangeNone) const;
	int power(TelemetryData::Unit unit = TelemetryData::UnitDefault, TelemetryData::Range range = TelemetryData::RangeNone) const;
	double duration(TelemetryData::Unit unit = TelemetryData::UnitDefault) const;
	double distance(TelemetryData::Unit unit = TelemetryData::UnitDefault, TelemetryData::Range range = TelemetryData::RangeNone) const;
	double course(TelemetryData::Unit unit = TelemetryData::UnitDefault, TelemetryData::Range range = TelemetryData::RangeNone) const;
	double heading(TelemetryData::Unit unit = TelemetryData::UnitDefault, TelemetryData::Range range = TelemetryData::RangeNone) const;
	double grade(TelemetryData::Range range = TelemetryData::RangeNone) const;
	double speed(TelemetryData::Unit unit = TelemetryData::UnitDefault, TelemetryData::Range range = TelemetryData::RangeNone) const;
	double maxspeed(TelemetryData::Unit unit = TelemetryData::UnitDefault, TelemetryData::Range range = TelemetryData::RangeNone) const;
	double acceleration(TelemetryData::Unit unit = TelemetryData::UnitDefault, TelemetryData::Range range = TelemetryData::RangeNone) const;
	double rideTime(TelemetryData::Unit unit = TelemetryData::UnitDefault) const;
	double elapsedTime(TelemetryData::Unit unit = TelemetryData::UnitDefault) const;
	double avgspeed(TelemetryData::Unit unit = TelemetryData::UnitDefault, TelemetryData::Range range = TelemetryData::RangeNone) const;
	double avgridespeed(TelemetryData::Unit unit = TelemetryData::UnitDefault, TelemetryData::Range range = TelemetryData::RangeNone) const;
	double verticalspeed(TelemetryData::Unit unit = TelemetryData::UnitDefault, TelemetryData::Range range = TelemetryData::RangeNone) const;
	double homedistance(TelemetryData::Unit unit = TelemetryData::UnitDefault, TelemetryData::Range range = TelemetryData::RangeNone) const;
	double batterylevel(TelemetryData::Range range = TelemetryData::RangeNone) const;
	int lap(void) const;

	bool inRange(void) const;
	bool isPause(void) const;

	bool hasValue(Data type = DataAll) const;

	void reset(bool all = false);

	void dump(void) const;

	static void writeHeader(void);
	void writeData(size_t index) const;

protected:
	int has_value_;

	bool in_range_;
	bool is_pause_;

	uint32_t line_;

	Type type_;

	int index_;

	uint64_t ts_;
	double lat_, lon_;
	double raw_lat_, raw_lon_;
	double ele_, ele_min_, ele_max_;
	double temperature_, temperature_min_, temperature_max_;
	int heartrate_, heartrate_min_, heartrate_max_;
	int cadence_, cadence_min_, cadence_max_;
	int power_, power_min_, power_max_;

	uint64_t datetime_;
	double distance_, distance_min_, distance_max_;
	double course_, course_min_, course_max_;
	double heading_, heading_min_, heading_max_;
	double duration_;
	double grade_, grade_min_, grade_max_;
	double speed_, speed_min_, speed_max_;
	double maxspeed_, maxspeed_min_, maxspeed_max_;
	double acceleration_, acceleration_min_, acceleration_max_;
	double ridetime_;
	double elapsedtime_;
	double avgspeed_, avgspeed_min_, avgspeed_max_;
	double avgridespeed_, avgridespeed_min_, avgridespeed_max_;
	double verticalspeed_, verticalspeed_min_, verticalspeed_max_;
	double homedistance_, homedistance_min_, homedistance_max_;
	double batterylevel_, batterylevel_min_, batterylevel_max_;

	int lap_;
	bool in_lap_;
};


#endif

