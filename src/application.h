#ifndef __GPX2VIDEO__APPLICATION_H__
#define __GPX2VIDEO__APPLICATION_H__

#include <list>
#include <cstdlib>
#include <cstdint>
#include <cstring>

#include <unistd.h>

#include "telemetrysettings.h"


class GPXApplication {
public:
	class Settings {
	public:
		Settings(std::string input_file="", 
			std::string output_file="",
			int max_duration_ms=0)
			: input_file_(input_file)
			, output_file_(output_file)
			, max_duration_ms_(max_duration_ms) {
		}

		const std::string& inputfile(void) const {
			return input_file_;
		}

		void setInputfile(const std::string &file) {
			input_file_ = file;
		}
		
		const std::string& outputfile(void) const {
			return output_file_;
		}

		void setOutputfile(const std::string &file) {
			output_file_ = file;
		}
		
		const unsigned int& maxDuration(void) const {
			return max_duration_ms_;
		}

		void dump(void) const {
			std::cout << "Application settings:" << std::endl;
			std::cout << "  input file: " << input_file_ << std::endl;	
			std::cout << "  output file: " << output_file_ << std::endl;	
		}

	protected:
		std::string input_file_;
		std::string output_file_;

		unsigned int max_duration_ms_;
	};

	class Task {
	public:
		enum Action {
			ActionStart,
			ActionPerform,
			ActionStop,
			ActionExit,
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
		CommandFilter, 	// Dump telemetry filter supported
		CommandMethod,  // Dump telemetry method supported
		CommandSmooth,  // Dump telemetry smooth supported
		CommandSync,	// Auto sync video time with gps sensor
		CommandExtract,	// Extract gps sensor data from video
		CommandClear,	// Clear cache directories
		CommandMap,		// Download & build map
		CommandTrack,	// Download, build map & draw track
		CommandConvert, // Convert telemetry data
		CommandCompute, // Compute telemetry data from gpx, csv...
		CommandImage,	// Render alpha image with telemetry overlay
		CommandVideo,	// Render video with telemetry overlay
		CommandTest, 	// Test tool

		CommandCount
	};

	GPXApplication(struct event_base *evbase);
	virtual ~GPXApplication();

	void setLogLevel(int level);
	void setLogQuiet(bool enable);

	bool progressInfo(void);
	void setProgressInfo(bool enable);

	static const std::string version(void);

	Settings& settings(void);
	void setSettings(const Settings &settings);

	Command& command(void) {
		return command_;
	}

	void setCommand(const Command &command) {
		command_ = command;
	}

	void append(Task *task) {
		tasks_.push_back(task);
	}

	void remove(Task *task) {
		tasks_.remove(task);
	}

	struct event_base *evbase(void) {
		return evbase_;
	}

	void perform(enum Task::Action action=Task::ActionPerform);
	void run(enum Task::Action action);
	void exec(void);
	void abort(void);

protected:
	static void sighandler(int sfd, short kind, void *data);
	static void pipehandler(int sfd, short kind, void *data);

	void init(void);
	void listen(void);

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
};


#endif

