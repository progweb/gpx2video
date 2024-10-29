#ifndef __GPX2VIDEO__GPX2VIDEO_H__
#define __GPX2VIDEO__GPX2VIDEO_H__

#include <iostream>
#include <cstdlib>
#include <string>
#include <list>

#include <unistd.h>

#include "log.h"
#include "media.h"
#include "exportcodec.h"
#include "mapsettings.h"
#include "extractorsettings.h"
#include "telemetrysettings.h"
#include "application.h"


class Map;
class Extractor;


class GPX2Video : public GPXApplication {
public:
	class Settings : public GPXApplication::Settings, public TelemetrySettings, public RendererSettings {
	public:
		Settings(std::string gpx_file="", 
			std::string media_file="", 
			std::string layout_file="",
			std::string output_file="",
			int rate=0,
			int offset=0,
			std::string start_time="",
			bool time_factor_auto=false,
			double time_factor_value=1.0,
			double map_factor=1.0,
			int map_zoom=8, 
			int max_duration_ms=0,
			MapSettings::Source map_source=MapSettings::SourceOpenStreetMap,
			double path_thick=3.0,
			double path_border=1.4,
			std::string from="",
			std::string to="",
			ExtractorSettings::Format extract_format=ExtractorSettings::FormatDump,
			bool telemetry_check=false,
			TelemetrySettings::Filter telemetry_filter=TelemetrySettings::FilterNone,
			TelemetrySettings::Method telemetry_method=TelemetrySettings::MethodNone,
			int telemetry_rate=0,
			ExportCodec::Codec video_codec=ExportCodec::CodecH264,
			std::string video_hw_device="",
			std::string video_preset="medium",
			int32_t video_crf=-1,
			int64_t video_bit_rate=0,
			int64_t video_min_bit_rate=0,
			int64_t video_max_bit_rate=0)
			: GPXApplication::Settings(
					gpx_file, output_file,
					from, to, 
					offset,
					max_duration_ms)
			, TelemetrySettings(
					telemetry_check,
					telemetry_method, 
					telemetry_rate)
			, RendererSettings(
					media_file, layout_file,
					time_factor_auto,
					time_factor_value,
					video_codec,
					video_hw_device,
					video_preset,
					video_crf,
					video_bit_rate,
					video_min_bit_rate,
					video_max_bit_rate)
			, rate_(rate)
			, start_time_(start_time)
			, map_factor_(map_factor)
			, map_zoom_(map_zoom)
			, map_source_(map_source)
			, path_thick_(path_thick)
			, path_border_(path_border)
	   		, extract_format_(extract_format) {
			TelemetrySettings::setFilter(telemetry_filter);
		}

		const std::string& gpxfile(void) const {
			return inputfile();
		}
		
		const int& rate(void) const {
			return rate_;
		}

		const std::string& startTime(void) const {
			return start_time_;
		}

		const MapSettings::Source& mapsource(void) const {
			return map_source_;
		}

		const double& paththick(void) const {
			return path_thick_;
		}

		const double& pathborder(void) const {
			return path_border_;
		}

		const ExtractorSettings::Format& extractFormat(void) const {
			return extract_format_;
		}

		const double& mapfactor(void) const {
			return map_factor_;
		}

		const int& mapzoom(void) const {
			return map_zoom_;
		}

	private:
		int rate_;
		std::string start_time_;

		double map_factor_;
		int map_zoom_;
		MapSettings::Source map_source_;

		double path_thick_;
		double path_border_;

		ExtractorSettings::Format extract_format_;
	};

	GPX2Video(struct event_base *evbase);
	~GPX2Video();

	void setLogLevel(int level);
//	void setLogQuiet(bool enable);

	Settings& settings(void);
	void setSettings(const Settings &settings);

	int parseCommandLine(int argc, char *argv[]);

	MediaContainer * media(void);
	int setDefaultStartTime(void);
	Map * buildMap(void);
	Extractor * buildExtractor(void);

private:
	Settings settings_;

	MediaContainer *container_;
};

#endif
