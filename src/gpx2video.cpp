#include <iostream>
#include <cstdlib>
#include <string>

#include <string.h>
#include <getopt.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/signalfd.h>

extern "C" {
#include <event2/event.h>
}

#include "log.h"
#include "version.h"
#include "utils.h"
#include "evcurl.h"
#include "map.h"
#include "track.h"
#include "extractor.h"
#include "decoder.h"
#include "encoder.h"
#include "renderer.h"
#include "gpx2video.h"


GPX2Video::GPX2Video(struct event_base *evbase) 
	: evbase_(evbase)
	, container_(NULL) {
	log_call();

	setLogLevel(AV_LOG_INFO);
	setProgressInfo(false);

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

	init();
}


GPX2Video::~GPX2Video() {
	log_call();

	// Signal event
	event_del(ev_signal_);
	event_free(ev_signal_);
}


void GPX2Video::setLogLevel(int level) {
	log_call();

	av_log_set_level(level);
}


void GPX2Video::setLogQuiet(bool enable) {
	log_call();

	if (enable)
		fclose(stderr);
}


bool GPX2Video::progressInfo(void) {
	log_call();

	return progress_info_;
}


void GPX2Video::setProgressInfo(bool enable) {
	log_call();

	progress_info_ = enable;
}


const std::string GPX2Video::version(void) {
	log_call();

	return GPX2VIDEO_VERSION;
}


GPX2Video::Settings& GPX2Video::settings(void) {
	return settings_;
}


void GPX2Video::setSettings(const GPX2Video::Settings &settings) {
	settings_ = settings;
}


MediaContainer * GPX2Video::media(void) {
	std::string mediafile = settings().mediafile();
	
	// Probe input media
	if (container_ == NULL)
		container_ = Decoder::probe(mediafile);

	return container_;
}


Map * GPX2Video::buildMap(void) {
	// GPX input file
	GPXData data;

	log_call();

	// Open GPX file
	GPX *gpx = GPX::open(settings().gpxfile());

	if (gpx == NULL) {
		log_warn("Can't read GPS data, none GPX file found");
		return NULL;
	}

	gpx->dump();

	// Create map bounding box
	GPXData::point p1, p2;
	gpx->getBoundingBox(&p1, &p2);

	MapSettings mapSettings;
	mapSettings.setSource(settings().mapsource());
	mapSettings.setZoom(settings().mapzoom());
	mapSettings.setDivider(settings().mapfactor());
	mapSettings.setBoundingBox(p1.lat, p1.lon, p2.lat, p2.lon);

	Map *map = Map::create(*this, mapSettings);

	return map;
}


Extractor * GPX2Video::buildExtractor(void) {
	ExtractorSettings extractorSettings;
	extractorSettings.setFormat(settings().extractFormat());

	Extractor *extractor = Extractor::create(*this, extractorSettings);

	return extractor;
}


void GPX2Video::sighandler(int sfd, short kind, void *data) {
	pid_t pid;

	int status;

	ssize_t s;
	struct signalfd_siginfo fdsi;

	GPX2Video *app = (GPX2Video *) data;

	(void) kind;

	log_call();

	s = read(sfd, &fdsi, sizeof(struct signalfd_siginfo));

	if (s != sizeof(struct signalfd_siginfo)) {
		log_error("Read signal message failure");
		goto sighandler_error;
	}

	switch (fdsi.ssi_signo) {
	case SIGCHLD:
		log_debug("SIGCHLD signal received");

		for (;;) {
			pid = waitpid((pid_t)(-1), &status, WNOHANG | WUNTRACED | WCONTINUED);

			if (pid <= 0)
				goto sighandler_error;

			log_warn("Received SIGCHLD from PID: %d", pid);

			if (WIFEXITED(status))
				log_info("Completed and returned %d", WEXITSTATUS(status));
			else if (WIFSIGNALED(status))
				log_info("Killed due to the signal %d", WTERMSIG(status));
			else if (WIFSTOPPED(status))
				log_info("Stopped by %d and returned %d", WSTOPSIG(status), WEXITSTATUS(status));
			else if (WIFCONTINUED(status)) {
				log_debug("Child continue");
				continue;
			}
		}

		break;

	case SIGINT:
		app->abort();
		break;

	case SIGPIPE:
		log_error("SIGPIPE signal received (fd: %d)", fdsi.ssi_fd);
		break;

	case SIGTERM:
		log_error("SIGTERM %d", fdsi.ssi_pid);
		app->abort();
		break;

	case SIGQUIT:
		log_error("SIGQUIT %d", fdsi.ssi_pid);
		app->abort();
		break;

	default:
		log_error("UNDEFINED");
		break;
	}

	return;

sighandler_error:
	app->loopexit();
	return;
}


void GPX2Video::pipehandler(int sfd, short kind, void *data) {
	int32_t info;

	size_t bytes;

	GPX2Video *app = (GPX2Video *) data;

	log_call();

	(void) sfd;
	(void) kind;

	bytes = read(app->pipe_in_, &info, sizeof(info));

	(void) bytes;

	app->run((enum GPX2Video::Task::Action) info);
}


void GPX2Video::init(void) {
	int error;

	int fds[2];
	int sfd = -1;

	sigset_t mask;

	log_call();

	// SIGHUP, SIGTERM, SIGINT, SIGQUIT management
	sigemptyset(&mask);
	sigaddset(&mask, SIGHUP);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGPIPE);
	sigaddset(&mask, SIGTERM);
	sigaddset(&mask, SIGQUIT);
	sigaddset(&mask, SIGCHLD);

	if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1) {
		log_error("sigprocmask failure");
		return;
	}

	sfd = signalfd(-1, &mask, SFD_NONBLOCK | SFD_CLOEXEC);

	if (sfd == -1) {
		log_error("signalfd failure");
		return;
	}

	ev_signal_ = event_new(evbase_, sfd, EV_READ | EV_PERSIST, sighandler, this);
	event_add(ev_signal_, NULL);

	// Create pipe to schedule tasks
	error = pipe(fds);

	if (error == -1) {
		log_error("Unable to create pipe");
		return;
	}

	pipe_in_ = fds[0];
	pipe_out_ = fds[1];
	ev_pipe_ = event_new(evbase_, pipe_in_, EV_READ | EV_PERSIST, pipehandler, this);
	event_add(ev_pipe_, NULL);
}


void GPX2Video::loop(void) {
	log_call();

	event_base_loop(evbase_, 0);
}

void GPX2Video::loopexit(void) {
	log_call();

	event_base_loopexit(evbase_, NULL);
}


