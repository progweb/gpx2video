#ifndef __GPX2VIDEO__APPLICATION_H__
#define __GPX2VIDEO__APPLICATION_H__

#include <cstdlib>
#include <list>

#include <unistd.h>

#include "log.h"
#include "telemetrysettings.h"


class GPXApplication {
public:
	class Settings {
	public:
		Settings(std::string input_file="", 
			std::string output_file="",
			std::string from="",
			std::string to="",
			int offset=0,
			int max_duration_ms=0)
//			TelemetrySettings::Method telemetry_method=TelemetrySettings::MethodNone,
//			int telemetry_rate=0)
			: input_file_(input_file)
			, output_file_(output_file)
			, from_(from)
			, to_(to)
			, offset_(offset)
			, max_duration_ms_(max_duration_ms) {
//			, telemetry_method_(telemetry_method) 
//			, telemetry_rate_(telemetry_rate) {
		}

		const std::string& inputfile(void) const {
			return input_file_;
		}
		
		const std::string& outputfile(void) const {
			return output_file_;
		}

		const std::string& from(void) const {
			return from_;
		}

		const std::string& to(void) const {
			return to_;
		}

		const int64_t& offset(void) const {
			return offset_;
		}

		const unsigned int& maxDuration(void) const {
			return max_duration_ms_;
		}

//		const TelemetrySettings::Method& telemetryMethod(void) const {
//			return telemetry_method_;
//		}
//
//		const int& telemetryRate(void) const {
//			return telemetry_rate_;
//		}

	protected:
		std::string input_file_;
		std::string output_file_;

		std::string from_;
		std::string to_;

		int64_t offset_;
		unsigned int max_duration_ms_;

//		TelemetrySettings::Method telemetry_method_;
//		int telemetry_rate_;
	};

	class Task {
	public:
		enum Action {
			ActionStart,
			ActionPerform,
			ActionStop
		};

		typedef void (*callback_t)(void *object);

		Task(GPXApplication &app)
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
		GPXApplication &app_;
	};

	enum Command {
		CommandNull,

		CommandSource,	// Dump map source list
		CommandFormat,  // Dump extract format supported
		CommandMethod,  // Dump telemetry method supported
		CommandSync,	// Auto sync video time with gps sensor
		CommandExtract,	// Extract gps sensor data from video
		CommandClear,	// Clear cache directories
		CommandMap,		// Download & build map
		CommandTrack,	// Download, build map & draw track
		CommandConvert, // Convert telemetry data
		CommandCompute, // Compute telemetry data from gpx, csv...
		CommandImage,	// Render alpha image with telemetry overlay
		CommandVideo,	// Render video with telemtry overlay
		CommandTest, 	// Test tool

		CommandCount
	};

	GPXApplication(struct event_base *evbase);
	virtual ~GPXApplication();

	void setLogLevel(int level);
	void setLogQuiet(bool enable);

	bool progressInfo(void);
	void setProgressInfo(bool enable);

	const time_t& time(void) {
		return time_;
	}

	void setTime(const time_t &time) {
		time_ = time;
	}

	static const std::string version(void);

	Settings& settings(void);
	void setSettings(const Settings &settings);

	Command& command(void) {
		return command_;
	}

	void setCommand(const Command &command) {
		command_ = command;
	}

	virtual int parseCommandLine(int argc, char *argv[]) = 0;

	void append(Task *task) {
		tasks_.push_back(task);
	}

	struct event_base *evbase(void) {
		return evbase_;
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

	std::list<Task *> tasks_;

	time_t time_;
};


#endif

