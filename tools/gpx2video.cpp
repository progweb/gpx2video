#include <iostream>
#include <cstdlib>
#include <string>
#include <filesystem>

#include <string.h>
#include <getopt.h>
#include <sys/stat.h>

extern "C" {
#include <event2/event.h>
#include <libavcodec/avcodec.h>
}

#include "log.h"
#include "version.h"
#include "utils.h"
#include "evcurl.h"
#include "datetime.h"
#include "cache.h"
#include "map.h"
#include "track.h"
#include "decoder.h"
#include "encoder.h"
#include "extractor.h"
#include "timesync.h"
#include "telemetry.h"
#include "imagerenderer.h"
#include "videorenderer.h"
#include "gpx2video.h"


namespace gpx2video {

static const struct option options[] = {
	{ "help",                       no_argument,       0, 'h' },
	{ "verbose",                    no_argument,       0, 'v' },
	{ "quiet",                      no_argument,       0, 'q' },
	{ "duration",                   required_argument, 0, 'd' },
	{ "trim",                       required_argument, 0, 0 },
	{ "media",                      required_argument, 0, 'm' },
	{ "gpx",                        required_argument, 0, 'g' },
	{ "layout",                     required_argument, 0, 'l' },
	{ "output",                     required_argument, 0, 'o' },
	{ "rate",                       required_argument, 0, 'r' },
	{ "start-time",                 required_argument, 0, 0 },
	{ "time-factor",                required_argument, 0, 0 },
	{ "map-source",                 required_argument, 0, 0 },
	{ "map-factor",                 required_argument, 0, 0 },
	{ "map-zoom",                   required_argument, 0, 0 },
	{ "map-source-list",            no_argument,       0, 0 },
	{ "extract-format",             required_argument, 0, 0 },
	{ "extract-format-list",        no_argument,       0, 0 },
	{ "gpx-begin",                  required_argument, 0, 0 },
	{ "gpx-end",                    required_argument, 0, 0 },
	{ "gpx-from",                   required_argument, 0, 0 },
	{ "gpx-to",                     required_argument, 0, 0 },
	{ "telemetry-offset",           required_argument, 0, 0 },
	{ "telemetry-check",            required_argument, 0, 0 },
	{ "telemetry-pause-detection",  required_argument, 0, 0 },
	{ "telemetry-filter",           required_argument, 0, 0 },
	{ "telemetry-filter-list",      no_argument,       0, 0 },
	{ "telemetry-method",           required_argument, 0, 0 },
	{ "telemetry-method-list",      no_argument,       0, 0 },
	{ "telemetry-rate",             required_argument, 0, 0 },
	{ "telemetry-smooth",           required_argument, 0, 0 },
	{ "telemetry-smooth-list",      no_argument,       0, 0 },
	{ "video-codec",                optional_argument, 0, 0 },
	{ "video-hwdevice",             optional_argument, 0, 0 },
	{ "video-preset",               required_argument, 0, 0 },
	{ "video-crf",                  required_argument, 0, 0 },
	{ "video-bitrate",              required_argument, 0, 0 },
	{ "video-min-bitrate",          required_argument, 0, 0 },
	{ "video-max-bitrate",          required_argument, 0, 0 },
	{ 0,                            0,                 0, 0 }
};

static void print_usage(const std::string &name) {
	log_call();

	std::cout << "Usage: " << name << " [-v] -m=media -g=gpx -o=output command" << std::endl;
	std::cout << "       " << name << " -h" << std::endl;
	std::cout << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "\t- m, --media=file                      : Input media file name" << std::endl;
	std::cout << "\t- g, --gpx=file                        : GPX file name" << std::endl;
	std::cout << "\t-    --gpx-begin                       : Drop data before datetime (format: yyyy-mm-dd hh:mm:ss) (not required)" << std::endl;
	std::cout << "\t-    --gpx-end                         : Drop data after datetime (format: yyyy-mm-dd hh:mm:ss) (not required)" << std::endl;
	std::cout << "\t-    --gpx-from                        : Compute data after datetime (format: yyyy-mm-dd hh:mm:ss) (not required)" << std::endl;
	std::cout << "\t-    --gpx-to                          : Compute data before datetime (format: yyyy-mm-dd hh:mm:ss) (not required)" << std::endl;
	std::cout << "\t- l, --layout=file                     : Layout file name" << std::endl;
	std::cout << "\t- o, --output=file                     : Output file name" << std::endl;
	std::cout << "\t- d, --duration                        : Duration (in ms) (not required)" << std::endl;
	std::cout << "\t-    --trim                            : Left trim crop (in ms) (not required)" << std::endl;
	std::cout << "\t-    --start-time                      : Overwrite or set creation_time field" << std::endl;
	std::cout << "\t-    --time-factor                     : Time factor - To read video timelapse (default: 1.0)" << std::endl;
	std::cout << "\t-    --extract-format=name             : Extract format (dump, gpx)" << std::endl;
	std::cout << "\t-    --telemetry-offset=value          : Apply time offset as data reading (value in ms)" << std::endl;
	std::cout << "\t-    --telemetry-check=bool            : Check & skip bad point (default: false)" << std::endl;
	std::cout << "\t-    --telemetry-pause-detection=bool  : Detect move & pause (default: true)" << std::endl;
	std::cout << "\t-    --telemetry-filter=filter         : Telemetry filter" << std::endl;
	std::cout << "\t-    --telemetry-method=method         : Telemetry interpolate method (none, sample, linear...)" << std::endl;
	std::cout << "\t-    --telemetry-rate                  : Telemetry rate (refresh each ms) (default: 250 ms))" << std::endl;
	std::cout << "\t-    --telemetry-smooth=value          : Number of points to smooth data (default '2', to disable set '0'))" << std::endl;
//	std::cout << "\t- r, --rate                            : Frame per second (not implemented" << std::endl;
	std::cout << "\t-    --map-factor                      : Map factor (default: 1.0)" << std::endl;
	std::cout << "\t-    --map-source                      : Map source" << std::endl;
	std::cout << "\t-    --map-zoom                        : Map zoom" << std::endl;
	std::cout << "\t-    --path-thick                      : Path thick (default: 3.0)" << std::endl;
	std::cout << "\t-    --path-border                     : Path border (default: 1.4)" << std::endl;
	std::cout << "\t- v, --verbose                         : Show trace" << std::endl;
	std::cout << "\t- q, --quiet                           : Quiet mode" << std::endl;
	std::cout << "\t- h, --help                            : Show this help screen" << std::endl;
	std::cout << std::endl;
	std::cout << "Option format:" << std::endl;
	std::cout << "\t-    --extract-format-list             : Dump extract format supported" << std::endl;
	std::cout << "\t-    --map-source-list                 : Dump supported map providers list" << std::endl;
	std::cout << "\t-    --telemetry-filter-list           : Dump telemetry filter supported" << std::endl;
	std::cout << "\t-    --telemetry-method-list           : Dump telemetry method supported" << std::endl;
	std::cout << "\t-    --telemetry-smooth-list           : Dump telemetry smooth supported" << std::endl;
	std::cout << std::endl;
	std::cout << "Encoder options:" << std::endl;
	std::cout << "\t-    --video-codec                     : Video encoder codec name" << std::endl;
	std::cout << "\t-    --video-hwdevice                  : Video encoder hardware device" << std::endl;
	std::cout << "\t-    --video-preset                    : Video encoder preset settings" << std::endl;
	std::cout << "\t-    --video-crf                       : Video encoder constant rate factor" << std::endl;
	std::cout << "\t-    --video-bitrate                   : Video encoder bitrate" << std::endl;
	std::cout << "\t-    --video-min-bitrate               : Video encoder min bitrate" << std::endl;
	std::cout << "\t-    --video-max-bitrate               : Video encoder max bitrate" << std::endl;
	std::cout << std::endl;
	std::cout << "Command:" << std::endl;
	std::cout << "\t extract: Extract GPS sensor data from media stream" << std::endl;
	std::cout << "\t sync   : Synchronize GoPro stream timestamp with embedded GPS" << std::endl;
	std::cout << "\t clear  : Clear cache" << std::endl;
	std::cout << "\t map    : Build map from gpx data" << std::endl;
	std::cout << "\t track  : Build map with track from gpx data" << std::endl;
	std::cout << "\t compute: Compute telemetry data from gpx, csv... data" << std::endl;
	std::cout << "\t image  : Process alpha image each second" << std::endl;
	std::cout << "\t video  : Process video" << std::endl;

	return;
}

static void print_map_list(const std::string &name) {
	int i;

	log_call();

	std::cout << "Map list: " << name << std::endl;

	for (i=MapSettings::SourceNull; i != MapSettings::SourceCount; i++) {
		std::string name = MapSettings::getFriendlyName((MapSettings::Source) i);
		std::string copyright = MapSettings::getCopyright((MapSettings::Source) i);
		std::string uri = MapSettings::getRepoURI((MapSettings::Source) i);

		if (uri == "")
			continue;

		std::cout << "\t- " << i << ":\t" << name << " " << copyright << std::endl;
	}
}

static void print_format_supported(const std::string &name) {
	int i;

	log_call();

	std::cout << "Extract format supported: " << name << std::endl;

	for (i=ExtractorSettings::FormatDump; i != ExtractorSettings::FormatCount; i++) {
		std::string name = ExtractorSettings::getFriendlyName((ExtractorSettings::Format) i);

		std::cout << "\t- " << i << ":\t" << name << std::endl;
	}
}

static void print_filter_supported(const std::string &name) {
	int i;

	log_call();

	std::cout << "Telemetry filter supported: " << name << std::endl;

	for (i=TelemetrySettings::FilterNone; i != TelemetrySettings::FilterCount; i++) {
		std::string name = TelemetrySettings::getFriendlyFilterName((TelemetrySettings::Filter) i);

		std::cout << "\t- " << i << ":\t" << name << std::endl;
	}
}

static void print_method_supported(const std::string &name) {
	int i;

	log_call();

	std::cout << "Telemetry method supported: " << name << std::endl;

	for (i=TelemetrySettings::MethodNone; i != TelemetrySettings::MethodCount; i++) {
		std::string name = TelemetrySettings::getFriendlyMethodName((TelemetrySettings::Method) i);

		std::cout << "\t- " << i << ":\t" << name << std::endl;
	}
}

static void print_smooth_supported(const std::string &name) {
	int i;

	log_call();

	std::cout << "Telemetry smooth supported: " << name << std::endl;

	for (i=TelemetrySettings::SmoothNone; i != TelemetrySettings::SmoothCount; i++) {
		std::string name = TelemetrySettings::getFriendlySmoothName((TelemetrySettings::Smooth) i);

		std::cout << "\t- " << i << ":\t" << name << std::endl;
	}
}

}; // namespace gpx2video


GPX2Video::GPX2Video(struct event_base *evbase) 
	: GPXApplication(evbase)
	, container_(NULL) {
	log_call();

//	setLogLevel(AV_LOG_INFO);
//	setProgressInfo(false);

	// Register FFmpeg codecs and filters (deprecated in 4.0+)
#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(58, 9, 100)
	av_register_all();
#endif
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(58, 9, 100)
	avcodec_register_all();
#endif
//#if LIBAVFILTER_VERSION_INT < AV_VERSION_INT(7, 14, 100)
//	avfilter_register_all();
//#endif

//	init();
	listen();
}


GPX2Video::~GPX2Video() {
	log_call();
}


void GPX2Video::setLogLevel(int level) {
	log_call();

	GPXApplication::setLogLevel(level);

	av_log_set_level(level);
}


GPX2Video::Settings& GPX2Video::settings(void) {
	return settings_;
}


void GPX2Video::setSettings(const GPX2Video::Settings &settings) {
	GPXApplication::setSettings(settings);

	settings_ = settings;
}


MediaContainer * GPX2Video::media(void) {
	std::string mediafile = settings().mediafile();

	std::string start_time = settings().startTime();
	
	// Probe input media
	if (container_ == NULL) {
		container_ = Decoder::probe(mediafile);

		if (container_ == NULL) {
			log_error("Media '%s' file read error", mediafile.c_str());
		}
		else if (!start_time.empty()) {
			log_notice("Overwrite video create time with: %s", start_time.c_str());

			container_->setStartTime(start_time);
		}
		else if (container_->startTime() == 0) {
			log_info("Creation time not found, you can set manually value in using '--start-time' option");

			this->setDefaultStartTime();
		}
	}

	return container_;
}


int GPX2Video::setDefaultStartTime(void) {
	TelemetryData data;

	log_call();

	// Open telemetry data file
	TelemetrySource *source = TelemetryMedia::open(settings().gpxfile(), settings(), true);

	if (source == NULL) {
		log_warn("Can't read telemetry data, set default start time failure");
		return -1;
	}

	// Start time activity
	source->retrieveFirst(data);
	
	// Use first point as start time
	container_->setStartTime(data.timestamp());

	// Dump result
	log_notice("Use default creation time: %s", Datetime::timestamp2string(data.timestamp()).c_str());

	// Free
	delete source;
	
	return 0;
}


Map * GPX2Video::buildMap(void) {
	// Telemetry data input file
	TelemetryData data;

	log_call();

	// Open telemetry data file
	TelemetrySource *source = TelemetryMedia::open(settings().gpxfile(), settings(), true);

	if (source == NULL) {
		log_warn("Can't read telemetry data, none telemetry file found");
		return NULL;
	}

	// Create map bounding box
	TelemetryData p1, p2;
	source->getBoundingBox(&p1, &p2);

	// Free
	delete source;

	MapSettings mapSettings;
	mapSettings.setSource(settings().mapsource());
	mapSettings.setZoom(settings().mapzoom());
	mapSettings.setDivider(settings().mapfactor());
	mapSettings.setBoundingBox(p1.latitude(), p1.longitude(), p2.latitude(), p2.longitude());
	mapSettings.setPathThick(settings().paththick());
	mapSettings.setPathBorder(settings().pathborder());

	Map *map = Map::create(*this, settings(), mapSettings);

	return map;
}


Extractor * GPX2Video::buildExtractor(void) {
	ExtractorSettings extractorSettings;
	extractorSettings.setFormat(settings().extractFormat());

	Extractor *extractor = Extractor::create(*this, extractorSettings, media());

	return extractor;
}


int GPX2Video::parseTelemetrySmoothArg(char *arg, 
		TelemetryData::Data &type, TelemetrySettings::Smooth &method, int &number) {
	int result = 0;

	int value;
	std::string name;

	char *subopt;
	char *subopts = arg;

	enum {
		TELEMETRY_SMOOTH_DATA = 0,
		TELEMETRY_SMOOTH_METHOD,
		TELEMETRY_SMOOTH_POINTS
	};

	const char * const token[] = {
		[TELEMETRY_SMOOTH_DATA] = "data",
		[TELEMETRY_SMOOTH_METHOD] = "method",
		[TELEMETRY_SMOOTH_POINTS] = "points",
		NULL
	};

	// Default values
	type = TelemetryData::DataNone;
	method = TelemetrySettings::SmoothNone;
	number = 0;

	// Parse args
	while (*subopts != '\0' && (result != -1)) {
		switch (getsubopt(&subopts, (char * const *) token, &subopt)) {
		case TELEMETRY_SMOOTH_DATA:
			name = std::string(subopt);

			if (name == "all")
				type = TelemetryData::DataAll;
			else if (name == "grade")
				type = TelemetryData::DataGrade;
			else if (name == "speed")
				type = TelemetryData::DataSpeed;
			else if (name == "elevation")
				type = TelemetryData::DataElevation;
			else if (name == "acceleration")
				type = TelemetryData::DataAcceleration;
			else if (name == "verticalspeed")
				type = TelemetryData::DataVerticalSpeed;
			else 
				result = -1;
			break;

		case TELEMETRY_SMOOTH_METHOD:
			value = std::stoi(subopt);

			if ((value >= 0) && (value < TelemetrySettings::SmoothCount))
				method = (TelemetrySettings::Smooth) value;
			else
				result = -1;
			break;

		case TELEMETRY_SMOOTH_POINTS:
			number = std::stoi(subopt);
			break;

		default:
			log_error("No match found for telemetry smooth token: '%s'\n", subopt);
			result = -1;
			break;
		}
	}

	return result;
}


int GPX2Video::parseCommandLine(int argc, char *argv[]) {
	int index;
	int option;

	int rate = 0; // Video fps - by default, no change
	int verbose = 0;
	int map_zoom = 12;
	int max_duration_ms = 0; // By default process whole media

	std::string start_time;

	bool time_factor_auto = false;
	double time_factor_value = 1.0;

	double map_factor = 1.0;

	double path_thick = 3.0;
	double path_border = 1.4;

	int telemetry_rate = 250; // By default, update each 250 milliseconds

	// By default, enable grade smooth filter
	TelemetrySettings::Smooth telemetry_smooth_grade_method = TelemetrySettings::SmoothWindowedMovingAverage;
	int telemetry_smooth_grade_points = 2; 
	// By default, enable speed smooth filter
	TelemetrySettings::Smooth telemetry_smooth_speed_method = TelemetrySettings::SmoothWindowedMovingAverage;
	int telemetry_smooth_speed_points = 2;
	// By default, enable elevation smooth filter
	TelemetrySettings::Smooth telemetry_smooth_elevation_method = TelemetrySettings::SmoothWindowedMovingAverage;
	int telemetry_smooth_elevation_points = 2;
	// By default, enable acceleration smooth filter
	TelemetrySettings::Smooth telemetry_smooth_acceleration_method = TelemetrySettings::SmoothWindowedMovingAverage;
	int telemetry_smooth_acceleration_points = 2;
	// By default, enable verticalspeed smooth filter
	TelemetrySettings::Smooth telemetry_smooth_verticalspeed_method = TelemetrySettings::SmoothWindowedMovingAverage;
	int telemetry_smooth_verticalspeed_points = 2;

	// Video encoder settings
	ExportCodec::Codec video_codec = ExportCodec::CodecH264;
	int32_t video_crf = -2; // Valid value are -1 and positive value
	std::string video_hw_device = "";
	std::string video_preset = "medium";
	int64_t video_bit_rate = 2 * 1000 * 1000 * 8;		// 16MB
	int64_t video_min_bit_rate = 0;						// 0
	int64_t video_max_bit_rate = 2 * 1000 * 1000 * 16;	// 32MB

	const char *s;

	MapSettings::Source map_source = MapSettings::SourceNull;

	std::string gpxfile;
	std::string mediafile;
	std::string layoutfile;
	std::string outputfile;

	std::string gpx_begin, gpx_end;
	std::string gpx_from, gpx_to;

	ExtractorSettings::Format extract_format = ExtractorSettings::FormatDump;

	int64_t telemetry_offset = 0;
	bool telemetry_check = false;
	bool telemetry_pause_detection = true;
	TelemetrySettings::Filter telemetry_filter = TelemetrySettings::FilterOutlier;
	TelemetrySettings::Method telemetry_method = TelemetrySettings::MethodInterpolate;

	bool gpxfile_required = false;
	bool mediafile_required = false;
	bool layoutfile_required = false;
	bool outputfile_required = false;

	const std::string name(argv[0]);

	log_call();

	for (;;) {
		index = 0;
		option = getopt_long(argc, argv, "hqvd:m:g:o:r:s:z:l:", gpx2video::options, &index);

		if (option == -1) 
			break;

		switch (option) {
		case 0:
			s = gpx2video::options[index].name;
			if (s && !strcmp(s, "start-time")) {
				start_time = std::string(optarg);
			}
			else if (s && !strcmp(s, "time-factor")) {
				if (!strcmp(optarg, "auto")) {
					time_factor_auto = true;
					time_factor_value = 1.0;
				}
				else {
					time_factor_auto = false;
					time_factor_value = strtod(optarg, NULL);
				}
			}
			else if (s && !strcmp(s, "trim")) {
				// TODO
				printf("Not yet implemented\n");
				return 0;
			}
			else if (s && !strcmp(s, "map-source-list")) {
				setCommand(GPX2Video::CommandSource);
				return 0;
			}
			else if (s && !strcmp(s, "map-factor")) {
				map_factor = strtod(optarg, NULL);
			}
			else if (s && !strcmp(s, "map-zoom")) {
				map_zoom = atoi(optarg);
			}
			else if (s && !strcmp(s, "map-source")) {
				map_source = (MapSettings::Source) atoi(optarg);
			}
			else if (s && !strcmp(s, "path-thick")) {
				path_thick = strtod(optarg, NULL);
			}
			else if (s && !strcmp(s, "path-border")) {
				path_border = strtod(optarg, NULL);
			}
			else if (s && !strcmp(s, "gpx-begin")) {
				gpx_begin = std::string(optarg);
			}
			else if (s && !strcmp(s, "gpx-end")) {
				gpx_end = std::string(optarg);
			}
			else if (s && !strcmp(s, "gpx-from")) {
				gpx_from = std::string(optarg);
			}
			else if (s && !strcmp(s, "gpx-to")) {
				gpx_to = std::string(optarg);
			}
			else if (s && !strcmp(s, "extract-format")) {
				extract_format = (ExtractorSettings::Format) atoi(optarg);
			}
			else if (s && !strcmp(s, "extract-format-list")) {
				setCommand(GPX2Video::CommandFormat);
				return 0;
			}
			else if (s && !strcmp(s, "telemetry-offset")) {
				telemetry_offset = atoll(optarg);
			}
			else if (s && !strcmp(s, "telemetry-check")) {
				telemetry_check = (std::string(optarg) == "true");
			}
			else if (s && !strcmp(s, "telemetry-pause-detection")) {
				telemetry_pause_detection = (std::string(optarg) == "true");
			}
			else if (s && !strcmp(s, "telemetry-filter")) {
				telemetry_filter = (TelemetrySettings::Filter) atoi(optarg);
			}
			else if (s && !strcmp(s, "telemetry-filter-list")) {
				setCommand(GPX2Video::CommandFilter);
				return 0;
			}
			else if (s && !strcmp(s, "telemetry-method")) {
				telemetry_method = (TelemetrySettings::Method) atoi(optarg);
			}
			else if (s && !strcmp(s, "telemetry-method-list")) {
				setCommand(GPX2Video::CommandMethod);
				return 0;
			}
			else if (s && !strcmp(s, "telemetry-rate")) {
				telemetry_rate = atoi(optarg);
			}
			else if (s && !strcmp(s, "telemetry-smooth")) {
				int number;
				TelemetryData::Data type;
				TelemetrySettings::Smooth method;

				// Format : "data:method:points"
				parseTelemetrySmoothArg(optarg, type, method, number);

				switch (type) {
				case TelemetryData::DataGrade:
					telemetry_smooth_grade_method = method;
					telemetry_smooth_grade_points = number;
					break;

				case TelemetryData::DataSpeed:
					telemetry_smooth_speed_method = method;
					telemetry_smooth_speed_points = number;
					break;

				case TelemetryData::DataElevation:
					telemetry_smooth_elevation_method = method;
					telemetry_smooth_elevation_points = number;
					break;

				case TelemetryData::DataAcceleration:
					telemetry_smooth_acceleration_method = method;
					telemetry_smooth_acceleration_points = number;
					break;

				case TelemetryData::DataVerticalSpeed:
					telemetry_smooth_verticalspeed_method = method;
					telemetry_smooth_verticalspeed_points = number;
					break;

				case TelemetryData::DataAll:
					telemetry_smooth_grade_method = method;
					telemetry_smooth_grade_points = number;

					telemetry_smooth_speed_method = method;
					telemetry_smooth_speed_points = number;

					telemetry_smooth_elevation_method = method;
					telemetry_smooth_elevation_points = number;

					telemetry_smooth_acceleration_method = method;
					telemetry_smooth_acceleration_points = number;

					telemetry_smooth_verticalspeed_method = method;
					telemetry_smooth_verticalspeed_points = number;
					break;

				default:
					std::cout << "'telemetry-smooth' option malformed!" << std::endl;
					return -1;
				}
			}
			else if (s && !strcmp(s, "telemetry-smooth-list")) {
				setCommand(GPX2Video::CommandSmooth);
				return 0;
			}
			else if (s && !strcmp(s, "video-codec")) {
				if (optarg == NULL) {
					std::cout << std::endl;
					std::cout << "Video codecs list:" << std::endl;
					std::cout << "\t- h264" << std::endl;
					std::cout << "\t- hevc" << std::endl;
					std::cout << std::endl;
					std::cout << "NVidia video codecs list:" << std::endl;
					std::cout << "\t- h264_nvenc" << std::endl;
					std::cout << "\t- hevc_nvenc" << std::endl;
					std::cout << std::endl;
					std::cout << "Intel Quick Sync Video codecs list:" << std::endl;
					std::cout << "\t- h264_qsv" << std::endl;
					std::cout << "\t- hevc_qsv" << std::endl;
					std::cout << std::endl;
					std::cout << "VAAPI video codecs list:" << std::endl;
					std::cout << "\t- h264_vaapi" << std::endl;
					std::cout << std::endl;
					std::cout << "VAAPI video codec required video-hwdevice option." << std::endl;
					return -2;
				}
				else if (!strcasecmp(optarg, "h264") || !strcasecmp(optarg, "x264")) {
					video_codec = ExportCodec::CodecH264;

					if (video_crf == -2) // Undefined
						video_crf = 27;
				}
				else if (!strcasecmp(optarg, "h265") || !strcasecmp(optarg, "hevc")) {
					video_codec = ExportCodec::CodecHEVC;

					if (video_crf == -2) // Undefined
						video_crf = 31;
				}
				else if (!strcasecmp(optarg, "h264_nvenc") || !strcasecmp(optarg, "x264_nvenc")) {
					video_codec = ExportCodec::CodecNVEncH264;
				}
				else if (!strcasecmp(optarg, "h265_nvenc") || !strcasecmp(optarg, "hevc_nvenc")) {
					video_codec = ExportCodec::CodecNVEncHEVC;
				}
				else if (!strcasecmp(optarg, "h264_vaapi") || !strcasecmp(optarg, "x264_vaapi")) {
					video_codec = ExportCodec::CodecVAAPIH264;
				}
				else if (!strcasecmp(optarg, "h264_qsv") || !strcasecmp(optarg, "x264_qsv")) {
					video_codec = ExportCodec::CodecQSVH264;
				}
				else if (!strcasecmp(optarg, "h265_qsv") || !strcasecmp(optarg, "hevc_qsv")) {
					video_codec = ExportCodec::CodecQSVHEVC;
				}
				else {
					std::cout << "Video codec not supported!" << std::endl;
					return -1;
				}
			}
			else if (s && !strcmp(s, "video-hwdevice")) {
				if (optarg == NULL) {
					std::string path = "/dev/dri";

					std::cout << std::endl;
					std::cout << "Video hardware device list:" << std::endl;

					for (const auto& entry : std::filesystem::directory_iterator(path)) {
						struct stat sb;
						std::string name = entry.path().string();

						if ((::stat(name.c_str(), &sb) != 0) || !(sb.st_mode & S_IFCHR))
							continue;

						std::cout << "\t- " << name << std::endl;
					}

					return -2;
				}
				else {
					video_hw_device = std::string(optarg);
				}
			}
			else if (s && !strcmp(s, "video-preset")) {
				video_preset = std::string(optarg);
			}
			else if (s && !strcmp(s, "video-crf")) {
				video_crf = atoi(optarg);
			}
			else if (s && !strcmp(s, "video-bitrate")) {
				video_bit_rate = atoll(optarg);
			}
			else if (s && !strcmp(s, "video-min-bitrate")) {
				video_min_bit_rate = atoll(optarg);
			}
			else if (s && !strcmp(s, "video-max-bitrate")) {
				video_max_bit_rate = atoll(optarg);
			}
			else {
				std::cout << "option " << s;
				if (optarg)
					std::cout << " with arg " << optarg;
				std::cout << std::endl;
			}
			break;
		case 'h':
			return -1;
			break;
		case 'r':
			rate = atoi(optarg);
			break;
		case 'q':
			GPX2Video::setLogQuiet(true);
			break;
		case 'v':
			verbose++;
			break;
		case 'd':
			max_duration_ms = atoi(optarg);
			break;
		case 'm':
			if (!mediafile.empty()) {
				std::cout << "'media' option is already set!" << std::endl;
				return -1;
			}
			mediafile = std::string(optarg);
			break;
		case 'g':
			if (!gpxfile.empty()) {
				std::cout << "'gpx' option is already set!" << std::endl;
				return -1;
			}
			gpxfile = std::string(optarg);
			break;
		case 'l':
			if (!layoutfile.empty()) {
				std::cout << "'layout' option is already set!" << std::endl;
				return -1;
			}
			layoutfile = std::string(optarg);
			break;
		case 'o':
			if (!outputfile.empty()) {
				std::cout << "'output' option is already set!" << std::endl;
				return -1;
			}
			outputfile = std::string(optarg);
			break;
		default:
			return -1;
			break;
		}
	}

	// getopt has consumed
	argc -= optind;
	argv += optind;
	optind = 0;

	// Debug
	setLogLevel((verbose > 1) ? AV_LOG_DEBUG : AV_LOG_INFO);
	gpx2video_log_debug_enable((verbose > 2));

	// Check command
	if (argc == 1) {
		if (!strcmp(argv[0], "method")) {
			setCommand(GPX2Video::CommandMethod);
		}
		else if (!strcmp(argv[0], "smooth")) {
			setCommand(GPX2Video::CommandSmooth);
		}
		else if (!strcmp(argv[0], "extract")) {
			setCommand(GPX2Video::CommandExtract);

			mediafile_required = true;
			outputfile_required = true;
		}
		else if (!strcmp(argv[0], "sync")) {
			setCommand(GPX2Video::CommandSync);

			mediafile_required = true;
		}
		else if (!strcmp(argv[0], "clear")) {
			setCommand(GPX2Video::CommandClear);
		}
		else if (!strcmp(argv[0], "map")) {
			setCommand(GPX2Video::CommandMap);

			gpxfile_required = true;
			outputfile_required = true;
		}
		else if (!strcmp(argv[0], "track")) {
			setCommand(GPX2Video::CommandTrack);

			gpxfile_required = true;
			outputfile_required = true;
		}
		else if (!strcmp(argv[0], "compute")) {
			setCommand(GPX2Video::CommandCompute);
			
			gpxfile_required = true;
		}
		else if (!strcmp(argv[0], "image")) {
			setCommand(GPX2Video::CommandImage);
			
			gpxfile_required = true;
			mediafile_required = true;
			outputfile_required = true;
		}
		else if (!strcmp(argv[0], "video")) {
			setCommand(GPX2Video::CommandVideo);
			
			gpxfile_required = true;
			mediafile_required = true;
			outputfile_required = true;

		}
		else {
			std::cout << name << ": command '" << argv[0] << "' unknown" << std::endl;
			return -1;
		}
	}
	else {
		setCommand(GPX2Video::CommandVideo);
			
		gpxfile_required = true;
		mediafile_required = true;
		outputfile_required = true;
	}

	// Check required options
	if ((video_codec == ExportCodec::CodecVAAPIH264) && (video_hw_device == "")) {
		std::cout << name << ": option '--video-hwdevice' is required with VAAPI video codec" << std::endl;
		std::cout << std::endl;
		return -1;
	}

	if (mediafile_required && mediafile.empty()) {
		std::cout << name << ": option '--media' is required" << std::endl;
		std::cout << std::endl;
		return -1;
	}

	if (gpxfile_required && gpxfile.empty()) {
		std::cout << name << ": option '--gpx' is required" << std::endl;
		std::cout << std::endl;
		return -1;
	}

	if (layoutfile_required && layoutfile.empty()) {
		std::cout << name << ": option '--layout' is required" << std::endl;
		std::cout << std::endl;
		return -1;
	}

	if (outputfile_required && outputfile.empty()) {
		std::cout << name << ": option '--output' is required" << std::endl;
		std::cout << std::endl;
		return -1;
	}

	if ((command() == GPX2Video::CommandImage) || (command() == GPX2Video::CommandVideo)) {
		if ((telemetry_method != TelemetrySettings::MethodNone) && (telemetry_rate == 0)) {
			std::cout << "Invalid telemetry rate value!" << std::endl;
			std::cout << std::endl;
			return -1;
		}
	}

	setProgressInfo((verbose > 0));

	// CRF defined by user ?
	if (video_crf == -2) // Undefined
		video_crf = -1; // Disable

	// Save app settings
	setSettings(GPX2Video::Settings(
		gpxfile,
		mediafile,
		layoutfile,
		outputfile,
		rate,
		start_time,
		time_factor_auto,
		time_factor_value,
		map_factor,
		map_zoom,
		max_duration_ms,
		map_source,
		path_thick,
		path_border,
		gpx_begin,
		gpx_end,
		gpx_from,
		gpx_to,
		extract_format,
		telemetry_offset,
		telemetry_check,
		telemetry_pause_detection,
		telemetry_filter,
		telemetry_method,
		telemetry_rate,
		telemetry_smooth_grade_method,
		telemetry_smooth_grade_points,
		telemetry_smooth_speed_method,
		telemetry_smooth_speed_points,
		telemetry_smooth_elevation_method,
		telemetry_smooth_elevation_points,
		telemetry_smooth_acceleration_method,
		telemetry_smooth_acceleration_points,
		telemetry_smooth_verticalspeed_method,
		telemetry_smooth_verticalspeed_points,
		video_codec,
		video_hw_device,
		video_preset,
		video_crf,
		video_bit_rate,
		video_min_bit_rate,
		video_max_bit_rate)
	);

	return 0;
}


int main(int argc, char *argv[], char *envp[]) {
	int result;

	Map *map = NULL;
	Cache *cache = NULL;
	Renderer *renderer = NULL;
	TimeSync *timesync = NULL;
	Extractor *extractor = NULL;
	Telemetry *telemetry = NULL;

	TelemetrySettings telemetrySettings;
	RendererSettings rendererSettings;

	struct event_base *evbase;

	const std::string name(argv[0]);

	(void) envp;

	// Event loop
	evbase = event_base_new();

	// Baner info
	log_notice("gpx2video v%s", GPX2Video::version().c_str());

	// Init
	GPX2Video app(evbase);

	// Logs
	app.setLogLevel(AV_LOG_INFO);

	// Parse args
	result = app.parseCommandLine(argc, argv);
	if (result < 0) {
		if (result == -1)
			gpx2video::print_usage(name);
		goto exit;
	}

	// ::TMP:: Check assets directory 
	{
    	std::ifstream stream = std::ifstream("./assets/marker/position.png");

		if (!stream.is_open()) {
			log_error("Can't read assets directory");
			log_error("Please ready to build & use gpx2video");
			log_error("Don't forget to create assets link");
			goto exit;
		}
	}

	// Process
	switch (app.command()) {
	case GPX2Video::CommandSource:
		gpx2video::print_map_list(name);
		goto exit;
		break;

	case GPX2Video::CommandFormat:
		gpx2video::print_format_supported(name);
		goto exit;
		break;

	case GPX2Video::CommandFilter:
		gpx2video::print_filter_supported(name);
		goto exit;
		break;

	case GPX2Video::CommandMethod:
		gpx2video::print_method_supported(name);
		goto exit;
		break;

	case GPX2Video::CommandSmooth:
		gpx2video::print_smooth_supported(name);
		goto exit;
		break;

	case GPX2Video::CommandExtract:
		extractor = app.buildExtractor();
		app.append(extractor);
		break;

	case GPX2Video::CommandSync:
		// Create gpx2video timesync task
		timesync = TimeSync::create(app, app.media());
		app.append(timesync);
		break;

	case GPX2Video::CommandClear:
		// Create cache task
		cache = Cache::create(app);
		app.append(cache);
		break;

	case GPX2Video::CommandMap:
		// Create cache directories
		cache = Cache::create(app);
		app.append(cache);

		// Create gpx2video map task
		if (!app.settings().gpxfile().empty()) {
			if (app.settings().mapsource() != MapSettings::SourceNull) {
				map = app.buildMap();
				if (map == NULL) {
					log_error("Build map failure.");
					goto exit;
				}
				app.append(map);
			}
			else {
				log_error("Please choose map source.");
				goto exit;
			}
		}
		else {
			log_error("Please provide GPX data file.");
			goto exit;
		}
		break;

	case GPX2Video::CommandTrack:
		// Create cache directories
		cache = Cache::create(app);
		app.append(cache);

		// Create gpx2video map task
		if (!app.settings().gpxfile().empty()) {
			if (app.settings().mapsource() != MapSettings::SourceNull) {
				map = app.buildMap();
				if (map == NULL) {
					log_error("Build map failure.");
					goto exit;
				}
				app.append(map);
			}
			else {
				log_error("Please choose map source.");
				goto exit;
			}
		}
		else {
			log_error("Please provide GPX data file.");
			goto exit;
		}
		break;

	case GPX2Video::CommandCompute: {
			// Telemetry settings
			telemetrySettings = TelemetrySettings(
					app.settings().telemetryOffset(),
					app.settings().telemetryCheck(),
					app.settings().telemetryPauseDetection(),
					app.settings().telemetryMethod(),
					app.settings().telemetryRate(),
					TelemetrySettings::FormatCSV);

			telemetrySettings.setDataRange(
					app.settings().telemetryBegin(),
					app.settings().telemetryEnd());
			telemetrySettings.setComputeRange(
					app.settings().telemetryFrom(),
					app.settings().telemetryTo());
			telemetrySettings.setFilter(app.settings().telemetryFilter());

			telemetrySettings.setTelemetrySmoothMethod(TelemetryData::DataGrade, app.settings().telemetrySmoothMethod(TelemetryData::DataGrade));
			telemetrySettings.setTelemetrySmoothPoints(TelemetryData::DataGrade, app.settings().telemetrySmoothPoints(TelemetryData::DataGrade));

			telemetrySettings.setTelemetrySmoothMethod(TelemetryData::DataSpeed, app.settings().telemetrySmoothMethod(TelemetryData::DataSpeed));
			telemetrySettings.setTelemetrySmoothPoints(TelemetryData::DataSpeed, app.settings().telemetrySmoothPoints(TelemetryData::DataSpeed));

			telemetrySettings.setTelemetrySmoothMethod(TelemetryData::DataElevation, app.settings().telemetrySmoothMethod(TelemetryData::DataElevation));
			telemetrySettings.setTelemetrySmoothPoints(TelemetryData::DataElevation, app.settings().telemetrySmoothPoints(TelemetryData::DataElevation));

			telemetrySettings.setTelemetrySmoothMethod(TelemetryData::DataAcceleration, app.settings().telemetrySmoothMethod(TelemetryData::DataAcceleration));
			telemetrySettings.setTelemetrySmoothPoints(TelemetryData::DataAcceleration, app.settings().telemetrySmoothPoints(TelemetryData::DataAcceleration));

			telemetrySettings.setTelemetrySmoothMethod(TelemetryData::DataVerticalSpeed, app.settings().telemetrySmoothMethod(TelemetryData::DataVerticalSpeed));
			telemetrySettings.setTelemetrySmoothPoints(TelemetryData::DataVerticalSpeed, app.settings().telemetrySmoothPoints(TelemetryData::DataVerticalSpeed));

			telemetry = Telemetry::create(app, telemetrySettings);
			app.append(telemetry);
		}
		break;

	case GPX2Video::CommandImage: {
			// Renderer settings
			rendererSettings = RendererSettings(
					app.settings().mediafile(),
					app.settings().layoutfile(),
					app.settings().isTimeFactorAuto(),
					app.settings().timeFactor());

			// Telemetry settings
			telemetrySettings = TelemetrySettings(
					app.settings().telemetryOffset(),
					app.settings().telemetryCheck(),
					app.settings().telemetryPauseDetection(),
					app.settings().telemetryMethod(),
					app.settings().telemetryRate());

			telemetrySettings.setDataRange(
					app.settings().telemetryBegin(),
					app.settings().telemetryEnd());
			telemetrySettings.setComputeRange(
					app.settings().telemetryFrom(),
					app.settings().telemetryTo());
			telemetrySettings.setFilter(app.settings().telemetryFilter());

			telemetrySettings.setTelemetrySmoothMethod(TelemetryData::DataGrade, app.settings().telemetrySmoothMethod(TelemetryData::DataGrade));
			telemetrySettings.setTelemetrySmoothPoints(TelemetryData::DataGrade, app.settings().telemetrySmoothPoints(TelemetryData::DataGrade));

			telemetrySettings.setTelemetrySmoothMethod(TelemetryData::DataSpeed, app.settings().telemetrySmoothMethod(TelemetryData::DataSpeed));
			telemetrySettings.setTelemetrySmoothPoints(TelemetryData::DataSpeed, app.settings().telemetrySmoothPoints(TelemetryData::DataSpeed));

			telemetrySettings.setTelemetrySmoothMethod(TelemetryData::DataElevation, app.settings().telemetrySmoothMethod(TelemetryData::DataElevation));
			telemetrySettings.setTelemetrySmoothPoints(TelemetryData::DataElevation, app.settings().telemetrySmoothPoints(TelemetryData::DataElevation));

			telemetrySettings.setTelemetrySmoothMethod(TelemetryData::DataAcceleration, app.settings().telemetrySmoothMethod(TelemetryData::DataAcceleration));
			telemetrySettings.setTelemetrySmoothPoints(TelemetryData::DataAcceleration, app.settings().telemetrySmoothPoints(TelemetryData::DataAcceleration));

			telemetrySettings.setTelemetrySmoothMethod(TelemetryData::DataVerticalSpeed, app.settings().telemetrySmoothMethod(TelemetryData::DataVerticalSpeed));
			telemetrySettings.setTelemetrySmoothPoints(TelemetryData::DataVerticalSpeed, app.settings().telemetrySmoothPoints(TelemetryData::DataVerticalSpeed));

			// Create cache directories
			cache = Cache::create(app);
			app.append(cache);

			// Create gpx2video timesync task
			if (app.settings().startTime().empty()) {
				timesync = TimeSync::create(app, app.media());
				app.append(timesync);
			}

			// Create gpx2video image renderer task
			if ((renderer = ImageRenderer::create(app, rendererSettings, telemetrySettings, app.media())) == NULL) {
				log_error("Image renderer initialization failure!");
				goto exit;
			}
			app.append(renderer);
		}
		break;

	case GPX2Video::CommandVideo: {
			// Renderer settings
			rendererSettings = RendererSettings(
					app.settings().mediafile(),
					app.settings().layoutfile(),
					app.settings().isTimeFactorAuto(),
					app.settings().timeFactor(),
					app.settings().videoCodec(),
					app.settings().videoHardwareDevice(),
					app.settings().videoPreset(),
					app.settings().videoCRF(),
					app.settings().videoBitrate(),
					app.settings().videoMinBitrate(),
					app.settings().videoMaxBitrate());

			// Telemetry settings
			telemetrySettings = TelemetrySettings(
					app.settings().telemetryOffset(),
					app.settings().telemetryCheck(),
					app.settings().telemetryPauseDetection(),
					app.settings().telemetryMethod(),
					app.settings().telemetryRate());

			telemetrySettings.setDataRange(
					app.settings().telemetryBegin(),
					app.settings().telemetryEnd());
			telemetrySettings.setComputeRange(
					app.settings().telemetryFrom(),
					app.settings().telemetryTo());
			telemetrySettings.setFilter(app.settings().telemetryFilter());

			telemetrySettings.setTelemetrySmoothMethod(TelemetryData::DataGrade, app.settings().telemetrySmoothMethod(TelemetryData::DataGrade));
			telemetrySettings.setTelemetrySmoothPoints(TelemetryData::DataGrade, app.settings().telemetrySmoothPoints(TelemetryData::DataGrade));

			telemetrySettings.setTelemetrySmoothMethod(TelemetryData::DataSpeed, app.settings().telemetrySmoothMethod(TelemetryData::DataSpeed));
			telemetrySettings.setTelemetrySmoothPoints(TelemetryData::DataSpeed, app.settings().telemetrySmoothPoints(TelemetryData::DataSpeed));

			telemetrySettings.setTelemetrySmoothMethod(TelemetryData::DataElevation, app.settings().telemetrySmoothMethod(TelemetryData::DataElevation));
			telemetrySettings.setTelemetrySmoothPoints(TelemetryData::DataElevation, app.settings().telemetrySmoothPoints(TelemetryData::DataElevation));

			telemetrySettings.setTelemetrySmoothMethod(TelemetryData::DataAcceleration, app.settings().telemetrySmoothMethod(TelemetryData::DataAcceleration));
			telemetrySettings.setTelemetrySmoothPoints(TelemetryData::DataAcceleration, app.settings().telemetrySmoothPoints(TelemetryData::DataAcceleration));

			telemetrySettings.setTelemetrySmoothMethod(TelemetryData::DataVerticalSpeed, app.settings().telemetrySmoothMethod(TelemetryData::DataVerticalSpeed));
			telemetrySettings.setTelemetrySmoothPoints(TelemetryData::DataVerticalSpeed, app.settings().telemetrySmoothPoints(TelemetryData::DataVerticalSpeed));

			// Create cache directories
			cache = Cache::create(app);
			app.append(cache);

			// Create gpx2video timesync task
			if (app.settings().startTime().empty()) {
				timesync = TimeSync::create(app, app.media());
				app.append(timesync);
			}

			// Create gpx2video video renderer task
			if ((renderer = VideoRenderer::create(app, rendererSettings, telemetrySettings, app.media())) == NULL) {
				log_error("Video renderer initialization failure!");
				goto exit;
			}
			app.append(renderer);
		}
		break;

	default:
		log_notice("Command not supported");
		goto exit;
		break;
	}

	// Infinite loop
	app.exec();

exit:
	if (map)
		delete map;
	if (cache)
		delete cache;
	if (renderer)
		delete renderer;
	if (timesync)
		delete timesync;
	if (extractor)
		delete extractor;

	event_base_free(evbase);

	exit(EXIT_SUCCESS);
}

