#include <iostream>
#include <cstdlib>
#include <string>
#include <filesystem>
#include <fstream>
#include <sstream>

#include <string.h>
#include <getopt.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

extern "C" {
#include <event2/event.h>
}

#include "log.h"
#include "version.h"
#include "telemetry.h"
#include "test.h"
#include "gpxtools.h"


namespace gpxtools {

static const struct option options[] = {
	{ "help",                        no_argument,       0, 'h' },
	{ "verbose",                     no_argument,       0, 'v' },
	{ "quiet",                       no_argument,       0, 'q' },
	{ "input",                       required_argument, 0, 'i' },
	{ "output",                      required_argument, 0, 'o' },
	{ "begin",                       required_argument, 0, 0 },
	{ "end",                         required_argument, 0, 0 },
	{ "from",                        required_argument, 0, 0 },
	{ "to",                          required_argument, 0, 0 },
	{ "telemetry-offset",            required_argument, 0, 0 },
	{ "telemetry-check",             required_argument, 0, 0 },
	{ "telemetry-pause-detection",   required_argument, 0, 0 },
	{ "telemetry-filter",            required_argument, 0, 0 },
	{ "telemetry-filter-list",       no_argument,       0, 0 },
	{ "telemetry-method",            required_argument, 0, 0 },
	{ "telemetry-method-list",       no_argument,       0, 0 },
	{ "telemetry-rate",              required_argument, 0, 'r' },
	{ "telemetry-smooth",            required_argument, 0, 0 },
	{ "telemetry-smooth-list",       no_argument,       0, 0 },
	{ 0,                             0,                 0, 0 }
};


static void print_usage(const std::string &name) {
	log_call();

	std::cout << "Usage: " << name << " [-v] -i input-file -o output-file command" << std::endl;
	std::cout << "       " << name << " -h" << std::endl;
	std::cout << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "\t- i, --input=file                      : Input telemetry file name" << std::endl;
	std::cout << "\t- o, --output=file                     : Output telemetry file name" << std::endl;
	std::cout << "\t-    --begin                           : Drop data before datetime (format: yyyy-mm-dd hh:mm:ss) (not required)" << std::endl;
	std::cout << "\t-    --end                             : Drop data after datetime (format: yyyy-mm-dd hh:mm:ss) (not required)" << std::endl;
	std::cout << "\t-    --from                            : Compute data after datetime (format: yyyy-mm-dd hh:mm:ss) (not required)" << std::endl;
	std::cout << "\t-    --to                              : Compute data before datetime (format: yyyy-mm-dd hh:mm:ss) (not required)" << std::endl;
	std::cout << "\t-    --telemetry-offset=value          : Apply time offset as data reading (value in ms)" << std::endl;
	std::cout << "\t-    --telemetry-check=bool            : Check & skip bad point (default: false)" << std::endl;
	std::cout << "\t-    --telemetry-pause-detection=bool  : Detect move & pause (default: true)" << std::endl;
	std::cout << "\t-    --telemetry-filter=filter         : Telemetry filter" << std::endl;
	std::cout << "\t-    --telemetry-method=method         : Interpolate method (none, sample, linear...)" << std::endl;
	std::cout << "\t-    --telemetry-rate=value            : Telemetry rate (refresh each ms) (default 'no change': 0))" << std::endl;
	std::cout << "\t-    --telemetry-smooth=value          : Number of points to smooth data (default 'disable': 0))" << std::endl;
	std::cout << "\t- v, --verbose                         : Show trace" << std::endl;
	std::cout << "\t- q, --quiet                           : Quiet mode" << std::endl;
	std::cout << "\t- h, --help                            : Show this help screen" << std::endl;
	std::cout << std::endl;
	std::cout << "Option format:" << std::endl;
	std::cout << "\t-    --telemetry-filter-list           : Dump telemetry filter supported" << std::endl;
	std::cout << "\t-    --telemetry-method-list           : Dump telemetry method supported" << std::endl;
	std::cout << "\t-    --telemetry-smooth-list           : Dump telemetry smooth supported" << std::endl;
	std::cout << std::endl;
	std::cout << "Command:" << std::endl;
	std::cout << "\t convert: Convert telemetry data file" << std::endl;
	std::cout << "\t compute: Compute telemetry data from gpx, csv... data" << std::endl;

	return;
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

}; // namespace gpxtools


GPXTools::GPXTools(struct event_base *evbase) 
	: GPXApplication(evbase) {
	log_call();
	listen();
}


GPXTools::~GPXTools() {
	log_call();
}


GPXTools::Settings& GPXTools::settings(void) {
	return settings_;
}


void GPXTools::setSettings(const GPXTools::Settings &settings) {
	GPXApplication::setSettings(settings);

	settings_ = settings;
}


int GPXTools::parseTelemetrySmoothArg(char *arg,
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


int GPXTools::parseCommandLine(int argc, char *argv[]) {
	int index;
	int option;

	bool check = false;
	bool pause_detection = true;

	int rate = 0; // By default, no change
	int offset = 0; // By default, no offset
	int verbose = 0; // By default, not verbose

	// By default, disable grade smooth filter
	TelemetrySettings::Smooth telemetry_smooth_grade_method = TelemetrySettings::SmoothNone;
	int telemetry_smooth_grade_points = 2; 
	// By default, disable speed smooth filter
	TelemetrySettings::Smooth telemetry_smooth_speed_method = TelemetrySettings::SmoothNone;
	int telemetry_smooth_speed_points = 2;
	// By default, disable elevation smooth filter
	TelemetrySettings::Smooth telemetry_smooth_elevation_method = TelemetrySettings::SmoothNone;
	int telemetry_smooth_elevation_points = 2;
	// By default, disable acceleration smooth filter
	TelemetrySettings::Smooth telemetry_smooth_acceleration_method = TelemetrySettings::SmoothNone;
	int telemetry_smooth_acceleration_points = 2;
	// By default, enable verticalspeed smooth filter
	TelemetrySettings::Smooth telemetry_smooth_verticalspeed_method = TelemetrySettings::SmoothNone;
	int telemetry_smooth_verticalspeed_points = 2;

	const char *s;

	std::string inputfile;
	std::string outputfile;

	std::string begin, end; // Telemetry range export data
	std::string to, from; 	// Telmetry range compute

	TelemetrySettings::Format format = TelemetrySettings::FormatAuto;
	TelemetrySettings::Filter filter = TelemetrySettings::FilterNone;
	TelemetrySettings::Method method = TelemetrySettings::MethodNone;

	const std::string name(argv[0]);

	log_call();

	for (;;) {
		index = 0;
		option = getopt_long(argc, argv, "hqvi:o:", gpxtools::options, &index);

		if (option == -1) 
			break;

		switch (option) {
		case 0:
			s = gpxtools::options[index].name;
			if (s && !strcmp(s, "begin")) {
				begin = std::string(optarg);
			}
			else if (s && !strcmp(s, "end")) {
				end = std::string(optarg);
			}
			else if (s && !strcmp(s, "from")) {
				from = std::string(optarg);
			}
			else if (s && !strcmp(s, "to")) {
				to = std::string(optarg);
			}
			else if (s && !strcmp(s, "telemetry-offset")) {
				offset = atoi(optarg);
			}
			else if (s && !strcmp(s, "telemetry-check")) {
				check = (std::string(optarg) == "true");
			}
			else if (s && !strcmp(s, "telemetry-pause-detection")) {
				pause_detection = (std::string(optarg) == "true");
			}
			else if (s && !strcmp(s, "telemetry-filter")) {
				filter = (TelemetrySettings::Filter) atoi(optarg);
			}
			else if (s && !strcmp(s, "telemetry-filter-list")) {
				setCommand(GPXTools::CommandFilter);
				return 0;
			}
			else if (s && !strcmp(s, "telemetry-method")) {
				method = (TelemetrySettings::Method) atoi(optarg);
			}
			else if (s && !strcmp(s, "telemetry-method-list")) {
				setCommand(GPXTools::CommandMethod);
				return 0;
			}
			else if (s && !strcmp(s, "telemetry-rate")) {
				rate = atoi(optarg);
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
				setCommand(GPXTools::CommandSmooth);
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
		case 'q':
			GPXTools::setLogQuiet(true);
			break;
		case 'v':
			verbose++;
			break;
		case 'i':
			if (!inputfile.empty()) {
				std::cout << "'input' option is already set!" << std::endl;
				return -1;
			}
			inputfile = std::string(optarg);
			break;
		case 'o':
			if (!outputfile.empty()) {
				std::cout << "'output' option is already set!" << std::endl;
				return -1;
			}
			outputfile = std::string(optarg);
			break;
		case 'r':
			rate = atoi(optarg);
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
	gpx2video_log_debug_enable((verbose > 1));

	// Check command
	if (argc == 1) {
		if (!strcmp(argv[0], "method")) {
			setCommand(GPXTools::CommandMethod);
		}
		else if (!strcmp(argv[0], "smooth")) {
			setCommand(GPXTools::CommandSmooth);
		}
		else if (!strcmp(argv[0], "convert")) {
			setCommand(GPXTools::CommandConvert);
		}
		else if (!strcmp(argv[0], "compute")) {
			setCommand(GPXTools::CommandCompute);
		}
		else if (!strcmp(argv[0], "test")) {
			setCommand(GPXTools::CommandTest);
		}
		else {
			std::cout << name << ": command '" << argv[0] << "' unknown" << std::endl;
			return -1;
		}
	}
	else {
		setCommand(GPXTools::CommandConvert);
	}

	// Override some settings
	if (command() == GPXTools::CommandConvert) {
		from = "";
		to = "";
		offset = 0; // Don't change timestamp
		check = false; // Keep each point
		pause_detection = false; // Disable pause detection
		filter = TelemetrySettings::FilterNone;	// Don't filter data
		method = TelemetrySettings::MethodNone; // None interpolation
	}

	if (outputfile.empty())
		format = TelemetrySettings::FormatDump;

	// Save app settings
	setSettings(GPXTools::Settings(
		inputfile,
		outputfile,
		0, // max duration not used
		begin,
		end,
		from,
		to,
		offset,
		check,
		pause_detection,
		filter,
		method,
		rate,
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
		format)
	);

	return 0;
}


int main(int argc, char *argv[], char *envp[]) {
	int result;

	Test *test = NULL;
	Telemetry *telemetry = NULL;

	TelemetrySettings settings;

	struct event_base *evbase;

	const std::string name(argv[0]);

	(void) envp;

	// Event loop
	evbase = event_base_new();

	// Baner info
	log_notice("gpxtools v%s", GPXTools::version().c_str());

	// Init
	GPXTools app(evbase);

	// Parse args
	result = app.parseCommandLine(argc, argv);
	if (result < 0) {
		if (result == -1)
			gpxtools::print_usage(name);
		goto exit;
	}

	// Process
	switch (app.command()) {
	case GPXTools::CommandFilter:
		gpxtools::print_filter_supported(name);
		goto exit;
		break;

	case GPXTools::CommandMethod:
		gpxtools::print_method_supported(name);
		goto exit;
		break;

	case GPXTools::CommandSmooth:
		gpxtools::print_smooth_supported(name);
		goto exit;
		break;

	case GPXTools::CommandConvert: {
			// Telemetry settings
			settings = TelemetrySettings(
					app.settings().telemetryOffset(),
					app.settings().telemetryCheck(),
					app.settings().telemetryPauseDetection(),
					app.settings().telemetryMethod(),
					app.settings().telemetryRate(),
					app.settings().telemetryFormat());

			settings.setDataRange(
					app.settings().telemetryBegin(),
					app.settings().telemetryEnd());
			settings.setFilter(TelemetrySettings::FilterNone);

			// Create gpxtools telemetry task
			telemetry = Telemetry::create(app, settings);
			app.append(telemetry);
		}
		break;

	case GPXTools::CommandCompute: {
			// Telemetry settings
			settings = TelemetrySettings(
					app.settings().telemetryOffset(),
					app.settings().telemetryCheck(),
					app.settings().telemetryPauseDetection(),
					app.settings().telemetryMethod(),
					app.settings().telemetryRate(),
					app.settings().telemetryFormat());

			settings.setDataRange(
					app.settings().telemetryBegin(),
					app.settings().telemetryEnd());
			settings.setComputeRange(
					app.settings().telemetryFrom(),
					app.settings().telemetryTo());
			settings.setFilter(app.settings().telemetryFilter());

			settings.setTelemetrySmoothMethod(TelemetryData::DataGrade, app.settings().telemetrySmoothMethod(TelemetryData::DataGrade));
			settings.setTelemetrySmoothPoints(TelemetryData::DataGrade, app.settings().telemetrySmoothPoints(TelemetryData::DataGrade));

			settings.setTelemetrySmoothMethod(TelemetryData::DataSpeed, app.settings().telemetrySmoothMethod(TelemetryData::DataSpeed));
			settings.setTelemetrySmoothPoints(TelemetryData::DataSpeed, app.settings().telemetrySmoothPoints(TelemetryData::DataSpeed));

			settings.setTelemetrySmoothMethod(TelemetryData::DataElevation, app.settings().telemetrySmoothMethod(TelemetryData::DataElevation));
			settings.setTelemetrySmoothPoints(TelemetryData::DataElevation, app.settings().telemetrySmoothPoints(TelemetryData::DataElevation));

			settings.setTelemetrySmoothMethod(TelemetryData::DataAcceleration, app.settings().telemetrySmoothMethod(TelemetryData::DataAcceleration));
			settings.setTelemetrySmoothPoints(TelemetryData::DataAcceleration, app.settings().telemetrySmoothPoints(TelemetryData::DataAcceleration));

			settings.setTelemetrySmoothMethod(TelemetryData::DataVerticalSpeed, app.settings().telemetrySmoothMethod(TelemetryData::DataVerticalSpeed));
			settings.setTelemetrySmoothPoints(TelemetryData::DataVerticalSpeed, app.settings().telemetrySmoothPoints(TelemetryData::DataVerticalSpeed));

			// Create gpxtools telemetry task
			telemetry = Telemetry::create(app, settings);
			app.append(telemetry);
		}
		break;

	case GPXTools::CommandTest: {
			// Test
			test = Test::create(app);
			app.append(test);
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
	event_base_free(evbase);

	exit(EXIT_SUCCESS);
}

