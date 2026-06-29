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
	int cadence(TelemetryData::Unit unit = TelemetryData::UnitDefault) const;
	int heartrate(TelemetryData::Unit unit = TelemetryData::UnitDefault) const;
	double temperature(TelemetryData::Unit unit = TelemetryData::UnitDefault) const;
	int power(TelemetryData::Unit unit = TelemetryData::UnitDefault) const;
	double duration(TelemetryData::Unit unit = TelemetryData::UnitDefault) const;
	double distance(TelemetryData::Unit unit = TelemetryData::UnitDefault, TelemetryData::Range range = TelemetryData::RangeNone) const;
	double course(TelemetryData::Unit unit = TelemetryData::UnitDefault) const;
	double heading(TelemetryData::Unit unit = TelemetryData::UnitDefault) const;
	double grade(void) const;
	double speed(TelemetryData::Unit unit = TelemetryData::UnitDefault) const;
	double maxspeed(TelemetryData::Unit unit = TelemetryData::UnitDefault) const;
	double acceleration(TelemetryData::Unit unit = TelemetryData::UnitDefault) const;
	double rideTime(TelemetryData::Unit unit = TelemetryData::UnitDefault) const;
	double elapsedTime(TelemetryData::Unit unit = TelemetryData::UnitDefault) const;
	double avgspeed(TelemetryData::Unit unit = TelemetryData::UnitDefault) const;
	double avgridespeed(TelemetryData::Unit unit = TelemetryData::UnitDefault) const;
	double verticalspeed(TelemetryData::Unit unit = TelemetryData::UnitDefault) const;
	double homedistance(TelemetryData::Unit unit = TelemetryData::UnitDefault) const;
	double batterylevel(void) const;
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
	double temperature_;
	int heartrate_;
	int cadence_;
	int power_;

	uint64_t datetime_;
	double distance_, distance_min_, distance_max_;
	double course_;
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
	double homedistance_;
	double batterylevel_;

	int lap_;
	bool in_lap_;
};


#endif

