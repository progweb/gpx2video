#ifndef __GPX2VIDEO__CSV_H__
#define __GPX2VIDEO__CSV_H__

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "log.h"
#include "telemetrymedia.h"


class CSV : public TelemetrySource {
public:
	CSV(const std::string &filename)
		: TelemetrySource(filename) {
		line_ = 0;

		sep_ = ',';

		// Column index
		index_timestamp_ = -1;
		index_total_duration_ = -1;
		index_partial_duration_ = -1;

		index_data_ = -1;
		index_ridetime_ = -1;

		index_latitude_ = -1;
		index_longitude_ = -1;
		index_elevation_ = -1;

		index_grade_ = -1;
		index_distance_ = -1;
		index_speed_ = -1;
		index_maxspeed_ = -1;
		index_avgspeed_ = -1;
		index_avgridespeed_ = -1;

		index_cadence_ = -1;
		index_heartrate_ = -1;
		index_temperature_ = -1;
		index_power_ = -1;
		index_lap_ = -1;

		if (!stream_.is_open()) {
			log_error("Open '%s' CSV file failure, please check that file is readable", filename.c_str());
			goto failure;
		}

failure:
		return;
	}

	virtual ~CSV() {
	}
	
	std::string name(void) {
		return std::string("CSV");
	}

	void reset(void) {
		std::string line;

		log_call();

		stream_.seekg(0, stream_.beg);

		line_ = 0;

		sep_ = ',';

		// Read & parse header
		readAndParseHeader();
	}

	enum TelemetrySource::Data read(TelemetrySource::Point &point) {
		std::string line;

		std::vector<std::string> columns;

		enum TelemetrySource::Data type = TelemetrySource::DataUnknown;

		log_call();

		type = readLine(line);
		parseLine(columns, line);

		if (type == TelemetrySource::DataEof)
			goto eof;

		writePoint(columns, point);

eof:
		return type;
	}

	bool readAndParseHeader() {
		std::string line;

		std::vector<std::string> columns;

		enum TelemetrySource::Data type = TelemetrySource::DataUnknown;

		log_call();

		type = readLine(line);
		parseFormat(line);
		parseLine(columns, line);

		if (type == TelemetrySource::DataEof)
			goto eof;

		// Timestamp, Time, Total duration, Partial duration, RideTime, 
		// Data, 
		// Lat, Lon, Ele, 
		// Grade, Distance, Speed, MaxSpeed, Average, Ride Average, 
		// Cadence, Heartrate, Power, Lap
		for (size_t i=0; i<columns.size(); i++) {
			if (columns[i] == "Timestamp")
				index_timestamp_ = i;
			else if (columns[i] == "Total duration")
				index_total_duration_ = i;
			else if (columns[i] == "Partial duration")
				index_partial_duration_ = i;
			else if (columns[i] == "RideTime")
				index_ridetime_ = i;
			else if (columns[i] == "Data")
				index_data_ = i;
			else if ((columns[i] == "Lat") || (columns[i] == "Latitude"))
				index_latitude_ = i;
			else if ((columns[i] == "Lon") || (columns[i] == "Longitude"))
				index_longitude_ = i;
			else if ((columns[i] == "Ele") || (columns[i] == "Elevation"))
				index_elevation_ = i;
			else if (columns[i] == "Grade")
				index_grade_ = i;
			else if (columns[i] == "Distance")
				index_distance_ = i;
			else if (columns[i] == "Speed")
				index_speed_ = i;
			else if (columns[i] == "MaxSpeed")
				index_maxspeed_ = i;
			else if (columns[i] == "Average")
				index_avgspeed_ = i;
			else if (columns[i] == "Ride Average")
				index_avgridespeed_ = i;
			else if (columns[i] == "Cadence")
				index_cadence_ = i;
			else if (columns[i] == "Heartrate")
				index_heartrate_ = i;
			else if (columns[i] == "Temperature")
				index_temperature_ = i;
			else if (columns[i] == "Power")
				index_power_ = i;
			else if (columns[i] == "Lap")
				index_lap_ = i;
		}

		return true;

eof:
		return false;
	}

	enum TelemetrySource::Data readLine(std::string &line) {
		log_call();

		if (!std::getline(stream_, line))
			return TelemetrySource::DataEof;

		return TelemetrySource::DataAgain;
	}

	bool parseFormat(std::string line) {
		auto pos = line.find(";");

		// ';' as column separator
		if (pos < line.length()) {
			sep_ = ';';
			return true;
		}

		// ',' as column separator
		pos = line.find(",");

		if (pos < line.length()) {
			sep_ = ',';
			return true;
		}

		return false;
	}

	bool parseLine(std::vector<std::string> &columns, std::string line) {
		std::string column;

		bool loop = true;
		bool withQ = false;
		std::string part{""};

		std::stringstream ss(line);

		log_call();

		line_++;

		while ((loop == true) && !ss.eof()) {
			while (std::getline(ss, column, sep_)) {
				for (; column.length() > 0;) {
					auto pos = column.find("\"");

					// " found
					if (pos < column.length()) {
						part += column.substr(0, pos);

						column = column.substr(pos + 1, column.length());

						if (withQ && (column[0] == '"')) {
							part += '"';
							column = column.substr(1, column.length());
						}
						else 
							withQ = !withQ;
					}

					if (!withQ) {
						column = trim(part + column);
						columns.emplace_back(std::move(column));
						part = "";
						loop = false;
					} 
					else if (column.find("\"") < column.length()) {
					}
					else {
						part += column + ",";
						column = "";
					}
				}
			}
		}

		return true;
	}

	void writePoint(std::vector<std::string> &columns, TelemetrySource::Point &point) {
		// 0: Timestamp, 1: Time, 2: Total duration, 3: Partial duration, 4: RideTime, 
		// 5: Data, 
		// 6: Lat, 7: Lon, 8: Ele, 
		// 9:Grade, 10: Distance, 11: Speed, 12: MaxSpeed, 13: Average, 14: Ride Average, 
		// 15: Cadence, 16: Heartrate, 17: Power, 18: Lap
		point.setLine(line_);

		if (index_data_ != -1)
			point.setType(columns[index_data_]);

		if ((index_timestamp_ != -1) && (index_latitude_ != -1) && (index_longitude_ != -1))
			point.setPosition(
				strtoul(columns[index_timestamp_].c_str(), NULL, 0),
				strtod(columns[index_latitude_].c_str(), NULL),
				strtod(columns[index_longitude_].c_str(), NULL)
			);

		if (index_elevation_ != -1)
			point.setElevation(str2double(columns[index_elevation_]));

		if (index_total_duration_ != -1)
			point.setDuration(strtod(columns[index_total_duration_].c_str(), NULL));

		if (index_partial_duration_ != -1)
			point.setElapsedTime(strtod(columns[index_partial_duration_].c_str(), NULL));

		if (index_grade_ != -1)
			point.setGrade(strtod(columns[index_grade_].c_str(), NULL));

		if (index_distance_ != -1)
			point.setDistance(strtod(columns[index_distance_].c_str(), NULL));

		if (index_speed_ != -1)
			point.setSpeed(strtod(columns[index_speed_].c_str(), NULL));

		if (index_maxspeed_ != -1)
			point.setMaxSpeed(strtod(columns[index_maxspeed_].c_str(), NULL));

		if (index_avgspeed_ != -1)
			point.setAverageSpeed(strtod(columns[index_avgspeed_].c_str(), NULL));

		if (index_avgridespeed_ != -1)
			point.setAverageRideSpeed(strtod(columns[index_avgridespeed_].c_str(), NULL));

		if (index_cadence_ != -1)
			point.setCadence(std::stoi(columns[index_cadence_]));

		if (index_heartrate_ != -1)
			point.setHeartrate(std::stoi(columns[index_heartrate_]));

		if (index_temperature_ != -1)
			point.setTemperature(std::stoi(columns[index_temperature_]));

		if (index_power_ != -1)
			point.setPower(std::stoi(columns[index_power_]));

		if (index_lap_ != -1)
			point.setLap(std::stoi(columns[index_lap_]));
	}

private:
	uint32_t line_;

	char sep_;

	int index_timestamp_;
	int index_total_duration_;
	int index_partial_duration_;

	int index_data_;
	int index_ridetime_;

	int index_latitude_;
	int index_longitude_;
	int index_elevation_;

	int index_grade_;
	int index_distance_;
	int index_speed_;
	int index_maxspeed_;
	int index_avgspeed_;
	int index_avgridespeed_;

	int index_cadence_;
	int index_heartrate_;
	int index_temperature_;
	int index_power_;
	int index_lap_;

	std::string trim(std::string str) {
		size_t position;

		const std::string whitespaces = " \t\n\t\f\v";

		position = str.find_first_not_of(whitespaces);
		str.erase(0, position);

		position = str.find_last_not_of(whitespaces);
		str.erase(position + 1);

		return str;
	}

	double str2double(std::string str) {
		auto pos = str.find(",");

		if (pos < str.length())
			str.replace(pos, 1, ".");

		return strtod(str.c_str(), NULL);
	}
};

#endif

