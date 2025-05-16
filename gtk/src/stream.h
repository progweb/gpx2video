#ifndef __GPX2VIDEO__GTK__STREAM_H__
#define __GPX2VIDEO__GTK__STREAM_H__

//#include <list>
#include <deque>

#include <glibmm/dispatcher.h>

#include <gdkmm/general.h>
#include <epoxy/gl.h>
#undef GLAPI

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <pulse/stream.h>

#include "../../src/stream.h"
#include "../../src/decoder.h"
#include "../../src/application.h"
#include "../../src/videowidget.h"
#include "../../src/telemetrymedia.h"
#include "../../src/telemetrysettings.h"
#include "audiodevice.h"
#include "shader.h"



enum {
	AV_SYNC_AUDIO_MASTER, /* default choice */
	AV_SYNC_VIDEO_MASTER,
	AV_SYNC_EXTERNAL_CLOCK, /* synchronize to an external clock */
};


class GPX2VideoStream {
public:
	class Clock {
	public:
		Clock();
		virtual ~Clock();

		void init(int *queue_serial);

		double get(void);

		void set(double pts, int serial);
		void set_at(double pts, int serial, double time);

		void sync_to_slave(Clock &slave);

		double pts_;           // clock base */
		double pts_drift_;     // clock base minus time at which we updated the clock
		double last_updated_;
		double speed_;
		int serial_;           // clock is based on a packet with this serial
		int paused_;
		int *queue_serial_;    // pointer to the current packet queue serial, used for obsolete clock detection
	};

	class Audio {
	public:
		Audio(Clock &extclk);
		~Audio();

		void init(void);

		Clock& clock(void);

		bool isOpened(void) const;

		void open(AudioStreamPtr stream);
		void close(void);

		void set_audio_device(GPX2VideoAudioDevice *audio_device);

		void toggle_pause(void);

		void notify(void);

		bool read(void);
		void write(size_t length);

		void run(void);
		void stop(void);
		void wait(void);

		void seek(double pos);

		void on_data_requested(void);

		size_t getQueueSize(void) const;

		void nextSampleBuffer(void);
		SampleBufferPtr getSampleBuffer(void);

		void flushSampleBuffer(void);

	private:
		bool loop_;

		Clock clock_;
		Clock &extclk_;

		AudioParams audio_params_;

		mutable std::mutex mutex_;
		mutable std::condition_variable cond_;

		bool is_playing_;

		Decoder *decoder_;

		AudioStreamPtr stream_;

		GPX2VideoAudioDevice *audio_device_;

		std::deque<SampleBufferPtr> queue_;

		bool seek_req_;
		double seek_pos_;

		size_t queue_size_;
	
		int serial_;
	};

	class Video {
	public:
		Video(Clock &extclk);
		~Video();

		void init(void);

		Clock& clock(void);

		bool isOpened(void) const;

		void open(VideoStreamPtr stream);
		void close(void);

		void notify(void);

		bool read(void);

		void run(void);
		void stop(void);
		void wait(void);

		void seek(double pos);

		Glib::Dispatcher& data_ready(void) {
			return dispatcher_;
		}

		size_t size(void) const;

		size_t getQueueSize(void) const;
		size_t getFrameNbRemaining(void) const;

		void nextFrame(void);
		FramePtr getFrame(void);
		FramePtr getNextFrame(void);
		FramePtr getLastFrame(void);

		void flushFrame(void);

		uint8_t **buffer(void);

	private:
		bool loop_;

		Clock clock_;
		Clock &extclk_;

		Glib::Dispatcher dispatcher_;

		mutable std::mutex mutex_;
		mutable std::condition_variable cond_;

		bool is_playing_;

		Decoder *decoder_;

		VideoStreamPtr stream_;

		std::deque<FramePtr> queue_;
		int frame_time_;

		bool seek_req_;
		double seek_pos_;

		int index_;
		size_t queue_size_;
		uint8_t **buffer_;
	
		int serial_;

		int shown_;
	};


public:
	GPX2VideoStream();
	~GPX2VideoStream();

	MediaContainer * media();

	void set_audio_device(GPX2VideoAudioDevice *audio_device);

	bool open(MediaContainer *container);
	void close(void);

	Clock& clock(void);

	Audio& audio(void);
	Video& video(void);

	void init(void);
	void load(FramePtr frame);
	void render(GPX2VideoShader *shader);

	bool play(void);
	void stop(void);
	void seek(double pos); //, int64_t rel);

	void toggle_pause(void);

	bool withAudio(void) const;
	bool withVideo(void) const;

	int width(void) const;
	int height(void) const;

	double duration(void) const;
	double timeBase(void) const;

	int nbChannels(void) const;

	VideoParams::Format format(void) const;

private:
	bool loop_;
	bool is_init_;

	Clock extclk_;

	Audio audio_;
	Video video_;

	std::thread* thread_audio_;
	std::thread* thread_video_;
	Glib::Dispatcher dispatcher_;

	GPX2VideoAudioDevice *audio_device_;

	MediaContainer *container_;

	GLuint vao_ = 0;
	GLuint vbo_ = 0;
	GLuint ebo_ = 0;
	GLuint pbo_[5];
	GLuint texture_;



public:
	int av_sync_type_;

	int frame_drops_late_;
	int frame_drops_early_;

	int get_master_sync_type(void);
	double get_master_clock(void);

	double vp_duration(FramePtr vp, FramePtr nextvp);

	double compute_target_delay(double delay);

	void sync_clock_to_slave(Clock &c, Clock &slave);
	void update_video_pts(double pts, int serial);
};


#endif

