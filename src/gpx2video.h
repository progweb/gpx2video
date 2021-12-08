#ifndef __GPX2VIDEO__GPX2VIDEO_H__
#define __GPX2VIDEO__GPX2VIDEO_H__

#include <iostream>
#include <cstdlib>
#include <string>

#include <unistd.h>

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
			double map_factor=1.0,
			int map_zoom=8, 
			int max_duration_ms=0,
			MapSettings::Source map_source=MapSettings::SourceOpenStreetMap)
			: gpx_file_(gpx_file)
			, media_file_(media_file)
			, output_file_(output_file)
			, map_factor_(map_factor)
			, map_zoom_(map_zoom)
			, max_duration_ms_(max_duration_ms)
			, map_source_(map_source) {
		}

		const std::string& gpxfile(void) const {
			return gpx_file_;
		}
		
		const std::string& mediafile(void) const {
			return media_file_;
		}

		const std::string& outputfile(void) const {
			return output_file_;
		}

		const MapSettings::Source& mapsource(void) const {
			return  map_source_;
		}

		const double& mapfactor(void) const {
			return map_factor_;
		}

		const int& mapzoom(void) const {
			return map_zoom_;
		}

		const int& maxDuration(void) const {
			return max_duration_ms_;
		}

	private:
		std::string gpx_file_;
		std::string media_file_;
		std::string output_file_;

		double map_factor_;
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
		int32_t info;

		log_call();

		info = (int32_t) done;

		write(pipe_out_, &info, sizeof(info));
	}

	void run(bool done) {
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
	static void pipehandler(int sfd, short kind, void *data);

	void init(void);

	void loop(void);
	void loopexit(void);

private:
	int pipe_in_;
	int pipe_out_;

	struct event *ev_pipe_;
	struct event *ev_signal_;
	struct event_base *evbase_;

	Settings settings_;

	std::list<Task *> tasks_;
};

#endif
