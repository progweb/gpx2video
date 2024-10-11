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
#include "gpxtools.h"


namespace gpxtools {

static const struct option options[] = {
	{ "help",                  no_argument,       0, 'h' },
	{ "verbose",               no_argument,       0, 'v' },
	{ "quiet",                 no_argument,       0, 'q' },
	{ "input",                 required_argument, 0, 'i' },
	{ "output",                required_argument, 0, 'o' },
	{ "telemetry-method",      required_argument, 0, 0 },
	{ "telemetry-method-list", no_argument,       0, 0 },
	{ "telemetry-rate",        required_argument, 0, 'r' },
	{ 0,                       0,                 0, 0 }
};


static void print_usage(const std::string &name) {
	log_call();

	std::cout << "Usage: " << name << "%s [-v] -i input-file -o output-file command" << std::endl;
	std::cout << "       " << name << " -h" << std::endl;
	std::cout << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "\t- i, --input=file              : Input telemetry file name" << std::endl;
	std::cout << "\t- o, --output=file             : Output telemetry file name" << std::endl;
	std::cout << "\t-    --from                    : Set begin (format: yyyy-mm-dd hh:mm:ss) (not required)" << std::endl;
	std::cout << "\t-    --to                      : Set end (format: yyyy-mm-dd hh:mm:ss) (not required)" << std::endl;
	std::cout << "\t-    --telemetry-method=method : Interpolate method (none, sample, linear...)" << std::endl;
	std::cout << "\t-    --telemetry-rate=value    : Telemetry rate (refresh each second) (default: 1000))" << std::endl;
	std::cout << "\t- v, --verbose                 : Show trace" << std::endl;
	std::cout << "\t- q, --quiet                   : Quiet mode" << std::endl;
	std::cout << "\t- h, --help                    : Show this help screen" << std::endl;
	std::cout << std::endl;
	std::cout << "Option format:" << std::endl;
	std::cout << "\t-    --telemetry-method-list   : Dump telemetry method supported" << std::endl;
	std::cout << std::endl;
	std::cout << "Command:" << std::endl;
	std::cout << "\t convert: Convert telemetry data file" << std::endl;
	std::cout << "\t compute: Compute telemetry data from gpx, csv... data" << std::endl;

	return;
}


static void print_method_supported(const std::string &name) {
	int i;

	log_call();

	std::cout << "Telemetry method supported: " << name << std::endl;

	for (i=TelemetrySettings::MethodNone; i != TelemetrySettings::MethodCount; i++) {
		std::string name = TelemetrySettings::getFriendlyName((TelemetrySettings::Method) i);

		std::cout << "\t- " << i << ":\t" << name << std::endl;
	}
}

}; // namespace gpxtools


GPXTools::GPXTools(struct event_base *evbase) 
	: GPXApplication(evbase) {
	log_call();
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


int GPXTools::parseCommandLine(int argc, char *argv[]) {
	int index;
	int option;

	int rate = 0; // By default, no change
	int verbose = 0;

	const char *s;

	std::string inputfile;
	std::string outputfile;

	std::string to;
	std::string from;

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
			if (s && !strcmp(s, "from")) {
				from = std::string(optarg);
			}
			else if (s && !strcmp(s, "to")) {
				to = std::string(optarg);
			}
			else if (s && !strcmp(s, "telemetry-method")) {
				method = (TelemetrySettings::Method) atoi(optarg);
			}
			else if (s && !strcmp(s, "telemetry-rate")) {
				rate = atoi(optarg);
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
		else if (!strcmp(argv[0], "convert")) {
			setCommand(GPXTools::CommandConvert);
		}
		else if (!strcmp(argv[0], "compute")) {
			setCommand(GPXTools::CommandCompute);
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
		method = TelemetrySettings::MethodNone;
	}
	
	// Save app settings
	setSettings(GPXTools::Settings(
		inputfile,
		outputfile,
		from,
		to,
		0,
		0,
		method,
		rate)
	);

	return 0;
}


int main(int argc, char *argv[], char *envp[]) {
	int result;

	Telemetry *telemetry = NULL;

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
	case GPXTools::CommandMethod:
		gpxtools::print_method_supported(name);
		goto exit;
		break;

	case GPXTools::CommandConvert: {
			// Telemetry settings
			TelemetrySettings settings(
					app.settings().telemetryMethod(),
					app.settings().telemetryRate());

			telemetry = Telemetry::create(app, settings);
			app.append(telemetry);
		}
		break;

	case GPXTools::CommandCompute: {
			// Telemetry settings
			TelemetrySettings settings(
					app.settings().telemetryMethod(),
					app.settings().telemetryRate(),
					TelemetrySettings::FormatCSV);

			telemetry = Telemetry::create(app, settings);
			app.append(telemetry);
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

