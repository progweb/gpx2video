#ifndef __GPX2VIDEO__GPX2VIDEO_H__
#define __GPX2VIDEO__GPX2VIDEO_H__

#include <iostream>
#include <cstdlib>
#include <string>
#include <list>

#include <unistd.h>

#include "log.h"
#include "media.h"
#include "mapsettings.h"
#include "extractorsettings.h"
#include "telemetrysettings.h"


class Map;
class Extractor;

class GPX2Video {
public:
	class Settings {
	public:
		Settings(std::string gpx_file="", 
			std::string media_file="", 
			std::string layout_file="",
			std::string output_file="",
			int offset=0,
			double map_factor=1.0,
			int map_zoom=8, 
			int max_duration_ms=0,
			MapSettings::Source map_source=MapSettings::SourceOpenStreetMap,
			std::string gpx_from="",
			std::string gpx_to="",
			ExtractorSettings::Format extract_format=ExtractorSettings::FormatDump,
			TelemetrySettings::Filter telemetry_filter=TelemetrySettings::FilterNone)
			: gpx_file_(gpx_file)
			, media_file_(media_file)
			, layout_file_(layout_file)
			, output_file_(output_file)
			, offset_(offset)
			, map_factor_(map_factor)
			, map_zoom_(map_zoom)
			, max_duration_ms_(max_duration_ms)
			, map_source_(map_source)
			, gpx_from_(gpx_from)
			, gpx_to_(gpx_to)
	   		, extract_format_(extract_format) 
			, telemetry_filter_(telemetry_filter) {
		}

		const std::string& gpxfile(void) const {
			return gpx_file_;
		}
		
		const std::string& mediafile(void) const {
			return media_file_;
		}

		const std::string& layoutfile(void) const {
			return layout_file_;
		}

		const std::string& outputfile(void) const {
			return output_file_;
		}

		const int& offset(void) const {
			return offset_;
		}

		const MapSettings::Source& mapsource(void) const {
			return map_source_;
		}

		const ExtractorSettings::Format& extractFormat(void) const {
			return extract_format_;
		}

		const TelemetrySettings::Filter& telemetryFilter(void) const {
			return telemetry_filter_;
		}

		const double& mapfactor(void) const {
			return map_factor_;
		}

		const int& mapzoom(void) const {
			return map_zoom_;
		}

		const unsigned int& maxDuration(void) const {
			return max_duration_ms_;
		}

		const std::string& gpxFrom(void) const {
			return gpx_from_;
		}

		const std::string& gpxTo(void) const {
			return gpx_to_;
		}

	private:
		std::string gpx_file_;
		std::string media_file_;
		std::string layout_file_;
		std::string output_file_;

		int offset_;

		double map_factor_;
		int map_zoom_;
		unsigned int max_duration_ms_;
		MapSettings::Source map_source_;

		std::string gpx_from_;
		std::string gpx_to_;

		ExtractorSettings::Format extract_format_;
		TelemetrySettings::Filter telemetry_filter_;
	};

	class Task {
	public:
		enum Action {
			ActionStart,
			ActionPerform,
			ActionStop
		};

		typedef void (*callback_t)(void *object);

		Task(GPX2Video &app)
			: app_(app) {
		}

		virtual ~Task() {
		}

		virtual bool start(void) {
			return true;
		};

		virtual bool run(void) = 0;

		virtual bool stop(void) {
			return true;
		};

		void schedule(void) {
			app_.perform(ActionPerform);
		}

		void complete(void) {
			app_.perform(ActionStop);
		}

	private:
		GPX2Video &app_;
	};

	enum Command {
		CommandNull,

		CommandSource,	// Dump map source list
		CommandFormat,  // Dump extract format supported
		CommandFilter,  // Dump telemetry filter supported
		CommandSync,	// Auto sync video time with gps sensor
		CommandExtract,	// Extract gps sensor data from video
		CommandClear,	// Clear cache directories
		CommandMap,		// Download & build map
		CommandTrack,	// Download, build map & draw track
		CommandCompute, // Compute telemetry data from gpx
		CommandVideo,	// Render video with telemtry overlay

		CommandCount
	};

	GPX2Video(struct event_base *evbase);
	~GPX2Video();

	void setLogLevel(int level);
	void setLogQuiet(bool enable);

	bool progressInfo(void);
	void setProgressInfo(bool enable);

	static const std::string version(void);

	Settings& settings(void);
	void setSettings(const Settings &settings);

	Command& command(void);
	void setCommand(const Command &command);

	int parseCommandLine(int argc, char *argv[]);

	void append(Task *task) {
		tasks_.push_back(task);
	}

	struct event_base *evbase(void) {
		return evbase_;
	}

	MediaContainer * media(void);
	Map * buildMap(void);
	Extractor * buildExtractor(void);

	const time_t& time(void) {
		return time_;
	}

	void setTime(const time_t &time) {
		time_ = time;
	}

	void perform(enum Task::Action action=Task::ActionPerform) {
		int32_t info;

		log_call();

		info = (int32_t) action;

		write(pipe_out_, &info, sizeof(info));
	}

	void run(enum Task::Action action) {
		Task *task;

		if (tasks_.empty())
			goto done;

		switch (action) {
		case Task::ActionStart:
			task = tasks_.front();
			if (task->start() == true)
				perform(Task::ActionPerform);
			else
				perform(Task::ActionStop);
			break;

		case Task::ActionPerform:
			task = tasks_.front();
			task->run();
			break;

		case Task::ActionStop:
			task = tasks_.front();
			task->stop();

			tasks_.pop_front();

			perform(Task::ActionStart);
			break;

		default:
			break;
		}

		return;

	done:
		abort();
	}

	void exec(void) {
		log_call();

		perform(Task::ActionStart);
		loop();
	}

	void abort(void) {
		log_call();

		// Before loop exit, stop the current task
		if (!tasks_.empty()) {
			Task *task = tasks_.front();

			task->stop();
		}

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

	bool progress_info_;

	Command command_;
	Settings settings_;

	MediaContainer *container_;

	std::list<Task *> tasks_;

	time_t time_;
};

#endif
