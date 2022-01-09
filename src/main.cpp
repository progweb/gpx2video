#include <iostream>
#include <cstdlib>
#include <string>

#include <string.h>
#include <getopt.h>

extern "C" {
#include <event2/event.h>
#include <libavcodec/avcodec.h>
}

#include "log.h"
#include "map.h"
#include "cache.h"
#include "renderer.h"
#include "timesync.h"
#include "extractor.h"
#include "gpx2video.h"


namespace gpx2video {

static const struct option options[] = {
	{ "help",           no_argument,       0, 'h' },
	{ "verbose",        no_argument,       0, 'v' },
	{ "quiet",          no_argument,       0, 'q' },
	{ "format",         required_argument, 0, 'f' },
	{ "duration",       required_argument, 0, 'd' },
	{ "media",          required_argument, 0, 'm' },
	{ "gpx",            required_argument, 0, 'g' },
	{ "layout",         required_argument, 0, 'l' },
	{ "output",         required_argument, 0, 'o' },
	{ "map-source",     required_argument, 0, 0 },
	{ "map-factor",     required_argument, 0, 0 },
	{ "map-zoom",       required_argument, 0, 0 },
	{ "map-list",       no_argument,       0, 0 },
	{ "extract-format", no_argument,       0, 0 },
	{ 0,                0,                 0, 0 }
};

static void print_usage(const std::string &name) {
	log_call();

	std::cout << "Usage: " << name << "%s [-v] -m=media -g=gpx -o=output command" << std::endl;
	std::cout << "       " << name << " -h" << std::endl;
	std::cout << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "\t- m, --media=file       : Input media file name" << std::endl;
	std::cout << "\t- g, --gpx=file         : GPX file name" << std::endl;
	std::cout << "\t- l, --layout=file      : Layout fie name" << std::endl;
	std::cout << "\t- o, --output=file      : Output file name" << std::endl;
	std::cout << "\t- d, --duration         : Duration (in ms)" << std::endl;
	std::cout << "\t- f, --format=name      : Extract format (dump, gpx)" << std::endl;
	std::cout << "\t-    --map-factor       : Map factor (default: 1.0)" << std::endl;
	std::cout << "\t-    --map-source       : Map source" << std::endl;
	std::cout << "\t-    --map-zoom         : Map zoom" << std::endl;
	std::cout << "\t-    --map-list         : Dump supported map list" << std::endl;
	std::cout << "\t-    --extract-format   : Dump extract format supported" << std::endl;
	std::cout << "\t- v, --verbose          : Show trace" << std::endl;
	std::cout << "\t- q, --quiet            : Quiet mode" << std::endl;
	std::cout << "\t- h, --help             : Show this help screen" << std::endl;
	std::cout << std::endl;
	std::cout << "Command:" << std::endl;
	std::cout << "\t extract: Extract GPS sensor data from media stream" << std::endl;
	std::cout << "\t sync   : Synchronize GoPro stream timestamp with embedded GPS" << std::endl;
	std::cout << "\t clear  : Clear cache" << std::endl;
	std::cout << "\t map    : Build map from gpx data" << std::endl;
	std::cout << "\t track  : Build map with track from gpx data" << std::endl;
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

}; // namespace gpx2video



GPX2Video::Command& GPX2Video::command(void) {
	return command_;
}


void GPX2Video::setCommand(const GPX2Video::Command &command) {
	command_ = command;
}


int GPX2Video::parseCommandLine(int argc, char *argv[]) {
	int index;
	int option;

	int verbose = 0;
	int map_zoom = 12;
	int max_duration_ms = 0; // By default process whole media

	double map_factor = 1.0;

	const char *s;

	MapSettings::Source map_source = MapSettings::SourceOpenStreetMap;

	std::string gpxfile;
	std::string mediafile;
	std::string layoutfile;
	std::string outputfile;

	ExtractorSettings::Format extract_format = ExtractorSettings::FormatDump;

	bool gpxfile_required = false;
	bool mediafile_required = false;
	bool layoutfile_required = false;
	bool outputfile_required = false;

	const std::string name(argv[0]);

	log_call();

	for (;;) {
		index = 0;
		option = getopt_long(argc, argv, "hqvd:m:g:o:f:s:z:l:", gpx2video::options, &index);

		if (option == -1) 
			break;

		switch (option) {
		case 0:
			s = gpx2video::options[index].name;
			if (s && !strcmp(s, "map-list")) {
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
			else if (s && !strcmp(s, "extract-format")) {
				setCommand(GPX2Video::CommandFormat);
				return 0;
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
		case 'f':
			extract_format = (ExtractorSettings::Format) atoi(optarg);
			break;
		case 'q':
//			GPX2Video::setLogQuiet(true);
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

	// Check command
	if (argc == 1) {
		if (!strcmp(argv[0], "extract")) {
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
	if (mediafile_required && mediafile.empty()) {
		std::cout << name << ": option '--media' is required" << std::endl;
		return -1;
	}

	if (gpxfile_required && gpxfile.empty()) {
		std::cout << name << ": option '--gpx' is required" << std::endl;
		return -1;
	}

	if (layoutfile_required && layoutfile.empty()) {
		std::cout << name << ": option '--layout' is required" << std::endl;
		return -1;
	}

	if (outputfile_required && outputfile.empty()) {
		std::cout << name << ": option '--output' is required" << std::endl;
		return -1;
	}

	// Save app settings
	setSettings(GPX2Video::Settings(
		gpxfile,
		mediafile,
		layoutfile,
		outputfile,
		map_factor,
		map_zoom,
		max_duration_ms,
		map_source,
		extract_format)
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
		gpx2video::print_usage(name);
		goto exit;
	}

	switch (app.command()) {
	case GPX2Video::CommandSource:
		gpx2video::print_map_list(name);
		goto exit;
		break;

	case GPX2Video::CommandFormat:
		gpx2video::print_format_supported(name);
		goto exit;
		break;

	case GPX2Video::CommandExtract:
		extractor = app.buildExtractor();
		app.append(extractor);
		break;

	case GPX2Video::CommandSync:
		// Create gpx2video timesync task
		timesync = TimeSync::create(app);
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
		map = app.buildMap();
		app.append(map);
		break;

	case GPX2Video::CommandTrack:
		// Create cache directories
		cache = Cache::create(app);
		app.append(cache);

		// Create gpx2video map task
		map = app.buildMap();
		app.append(map);
		break;

	case GPX2Video::CommandVideo:
		// Create cache directories
		cache = Cache::create(app);
		app.append(cache);

		// Create gpx2video timesync task
		timesync = TimeSync::create(app);
		app.append(timesync);

		// Create gpx2video map task
		map = app.buildMap();
		app.append(map);

		// Create gpx2video renderer task
		renderer = Renderer::create(app, map);

		app.append(renderer);
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

