
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <signal.h>
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
#include "application.h"


GPXApplication::GPXApplication(struct event_base *evbase) 
	: evbase_(evbase) 
	, time_(0) { 
	log_call();

//	setLogLevel(AV_LOG_INFO);
	setLogLevel(0);
	setProgressInfo(false);

	init();
}


GPXApplication::~GPXApplication() {
	log_call();

	// Signal event
	if (ev_signal_) {
		event_del(ev_signal_);
		event_free(ev_signal_);
	}
}


void GPXApplication::setLogLevel(int level) {
	log_call();

	(void) level;
}


void GPXApplication::setLogQuiet(bool enable) {
	log_call();

	if (enable)
		fclose(stderr);
}


bool GPXApplication::progressInfo(void) {
	log_call();

	return progress_info_;
}


void GPXApplication::setProgressInfo(bool enable) {
	log_call();

	progress_info_ = enable;
}


const std::string GPXApplication::version(void) {
	log_call();

	return GPX2VIDEO_VERSION;
}


GPXApplication::Settings& GPXApplication::settings(void) {
	return settings_;
}


void GPXApplication::setSettings(const GPXApplication::Settings &settings) {
	settings_ = settings;
}


void GPXApplication::sighandler(int sfd, short kind, void *data) {
	GPXApplication *app = (GPXApplication *) data;

	log_call();

	(void) sfd;
	(void) kind;

	app->abort();
}


void GPXApplication::pipehandler(int sfd, short kind, void *data) {
	int32_t info;

	size_t bytes;

	GPXApplication *app = (GPXApplication *) data;

	log_call();

	(void) sfd;
	(void) kind;

	bytes = read(app->pipe_in_, &info, sizeof(info));

	(void) bytes;

	app->run((enum GPXApplication::Task::Action) info);
}


void GPXApplication::init(void) {
	int error;

	int fds[2];
//	int sfd = -1;
//
//	sigset_t mask;

	log_call();

	ev_signal_ = NULL;

//	// SIGHUP, SIGTERM, SIGINT, SIGQUIT management
//	sigemptyset(&mask);
//	sigaddset(&mask, SIGHUP);
//	sigaddset(&mask, SIGINT);
//	sigaddset(&mask, SIGPIPE);
//	sigaddset(&mask, SIGTERM);
//	sigaddset(&mask, SIGQUIT);
//	sigaddset(&mask, SIGCHLD);
//
//	if (pthread_sigmask(SIG_BLOCK, &mask, NULL) == -1) {
//		log_error("sigprocmask failure");
//		return;
//	}
//
//	sfd = signalfd(-1, &mask, SFD_NONBLOCK | SFD_CLOEXEC);
//
//	if (sfd == -1) {
//		log_error("signalfd failure");
//		return;
//	}
//
//	ev_signal_ = event_new(evbase_, sfd, EV_READ | EV_PERSIST, sighandler, this);
//	event_add(ev_signal_, NULL);

	if (!evbase_)
		return;

	// SIGINT management
	ev_signal_ = evsignal_new(evbase_, SIGINT, sighandler, this);
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


void GPXApplication::loop(void) {
	log_call();

	event_base_loop(evbase_, 0);
}

void GPXApplication::loopexit(void) {
	log_call();

	event_base_loopexit(evbase_, NULL);
}

