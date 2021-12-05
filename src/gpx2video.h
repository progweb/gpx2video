#ifndef __GPX2VIDEO__GPX2VIDEO_H__
#define __GPX2VIDEO__GPX2VIDEO_H__

#include <iostream>
#include <cstdlib>
#include <string>

#include "log.h"
#include "mapsettings.h"


class Map;


class GPX2Video {
public:
	class Settings {
	public:
		Settings(std::string gpx_file="", 
			std::string media_file="", 
			std::string output_file="",
			int map_zoom=8, 
			int max_duration_ms=0,
			MapSettings::Source map_source=MapSettings::SourceOpenStreetMap)
			: gpx_file_(gpx_file)
			, media_file_(media_file)
			, output_file_(output_file)
			, map_zoom_(map_zoom)
			, max_duration_ms_(max_duration_ms)
			, map_source_(map_source) {
		}

		const std::string& gpxfile(void) const {
			return gpx_file_;
		}
		
		const MapSettings::Source& mapsource(void) const {
			return  map_source_;
		}

		const int& mapzoom(void) const {
			return map_zoom_;
		}

	private:
		std::string gpx_file_;
		std::string media_file_;
		std::string output_file_;

		int map_zoom_;
		int max_duration_ms_;
		MapSettings::Source map_source_;
	};

	class Task {
	public:
		typedef void (*callback_t)(void *object);

		Task(GPX2Video &app)
			: app_(app) {
		}

		virtual ~Task() {
		}

		virtual void run(void) = 0;

		void schedule(void) {
			app_.perform();
		}

		void complete(void) {
			app_.perform(true);
		}

	private:
		GPX2Video &app_;
	};

	GPX2Video(struct event_base *evbase);
	~GPX2Video();

	void setLogLevel(int level);
	static const std::string version(void);

	Settings& settings(void);
	void setSettings(const Settings &settings);

	int parseCommandLine(int argc, char *argv[]);

	void append(Task *task) {
		tasks_.push_back(task);
	}

	Map * buildMap(void);

	void perform(bool done=false) {
		log_call();

		if (done && !tasks_.empty())
			tasks_.pop_front();

		if (!tasks_.empty()) {
			Task *task = tasks_.front();

			task->run();
		}
		else
			abort();
	}

	void exec(void) {
		log_call();

		perform();
		loop();
	}

	void abort(void) {
		log_call();

		loopexit();
	}

protected:
	static void sighandler(int sfd, short kind, void *data);

	void init(void);

	void loop(void) {
		log_call();

		event_base_loop(evbase_, 0);
	}

	void loopexit(void) {
		log_call();

		event_base_loopexit(evbase_, NULL);
	}

private:
	struct event *ev_signal_;
	struct event_base *evbase_;

	Settings settings_;

	std::list<Task *> tasks_;
};

#endif
