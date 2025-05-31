#include <iostream>
#include <string>
#include <vector>
#include <exception>
#include <chrono>

extern "C" {
#include <libavutil/time.h>
}

// TST BEGIN
extern "C" {
#include <libavformat/avio.h>
#include <libavutil/bprint.h>
}
// TST END

#include <pulse/error.h>

#include "log.h"
#include "compat.h"
#include "datetime.h"
#include "stream.h"


/* no AV sync correction is done if below the minimum AV sync threshold */
#define AV_SYNC_THRESHOLD_MIN 0.04
/* AV sync correction is done if above the maximum AV sync threshold */
#define AV_SYNC_THRESHOLD_MAX 0.1
/* If a frame duration is longer than this, it will not be duplicated to compensate AV sync */
#define AV_SYNC_FRAMEDUP_THRESHOLD 0.1 
/* no AV correction is done if too big error */
#define AV_NOSYNC_THRESHOLD 10.0





// FFMPEG AV SYNC

int GPX2VideoStream::get_master_sync_type(void) {
	if (av_sync_type_ == AV_SYNC_VIDEO_MASTER) {
		if (withVideo())
			return AV_SYNC_VIDEO_MASTER;
		else
			return AV_SYNC_AUDIO_MASTER;
	}
	else if (av_sync_type_ == AV_SYNC_AUDIO_MASTER) {
		if (withAudio())
			return AV_SYNC_AUDIO_MASTER;
		else
			return AV_SYNC_EXTERNAL_CLOCK;
	}
	else
		return AV_SYNC_EXTERNAL_CLOCK;
}

double GPX2VideoStream::get_master_clock(void) {
	double val;

	switch (get_master_sync_type()) {
	case AV_SYNC_VIDEO_MASTER:
		val = video_.clock().get();
		break;

	case AV_SYNC_AUDIO_MASTER:
		val = audio_.clock().get();
		break;

	case AV_SYNC_EXTERNAL_CLOCK:
	default:
		val = extclk_.get();
		break;
	}

	return val;
}


double GPX2VideoStream::vp_duration(FramePtr vp, FramePtr nextvp) {
	double duration = (nextvp->time() - vp->time()) / 1000.0;
	if (isnan(duration) || duration <= 0 || duration > container_->maxFrameDuration())
		return vp->duration();
	else        
		return duration;                    
}


double GPX2VideoStream::compute_target_delay(double delay) {
	double sync_threshold, diff = 0;

	// update delay to follow master synchronisation source
	if (get_master_sync_type() != AV_SYNC_VIDEO_MASTER) {
		// if video is slave, we try to correct big delays by
		// duplicating or deleting a frame
		diff = video_.clock().get() - get_master_clock();

		// skip or repeat frame. We take into account the delay to compute 
		// the threshold. I still don't know if it is the best guess
		sync_threshold = FFMAX(AV_SYNC_THRESHOLD_MIN, FFMIN(AV_SYNC_THRESHOLD_MAX, delay));
		if (!isnan(diff) && fabs(diff) < container_->maxFrameDuration()) {
			if (diff <= -sync_threshold)
				delay = FFMAX(0, delay + diff);
			else if (diff >= sync_threshold && delay > AV_SYNC_FRAMEDUP_THRESHOLD)
				delay = delay + diff;
			else if (diff >= sync_threshold)
				delay = 2 * delay;
		}
	}

//	av_log(NULL, AV_LOG_TRACE, "video: delay=%0.3f A-V=%f\n",
//			delay, -diff);

	return delay;
}


void GPX2VideoStream::sync_clock_to_slave(GPX2VideoStream::Clock &c, GPX2VideoStream::Clock &slave)
{
	double clock = c.get();
	double slave_clock = slave.get();
	if (!isnan(slave_clock) && (isnan(clock) || fabs(clock - slave_clock) > AV_NOSYNC_THRESHOLD))
		c.set(slave_clock, slave.serial_);
}


void GPX2VideoStream::update_video_pts(double pts, int serial)
{
	/* update current video pts */
	video_.clock().set(pts, serial);
	sync_clock_to_slave(extclk_, video_.clock());
}







/**
 * Queue:
 *  - rindex: last picture displayed picture (peek_last)
 *  - rindex + rindex_shown: current picture not yet displayed (peek)
 *  - rindex + rindex_shown + 1: next picture (peek_next)
 */


static float vertices[] = {
	// points             // texture coords
	1.0f, 1.0f, 0.0f,     1.0f, 0.0f,   // top right
	1.0f, -1.0f, 0.0f,    1.0f, 1.0f,   // bottom right
	-1.0f, -1.0f, 0.0f,   0.0f, 1.0f,   // bottom left
	-1.0f, 1.0f, 0.0f,    0.0f, 0.0f    // top left
};

static unsigned int indices[] = {
	0, 1, 3,
	1, 2, 3
};



GPX2VideoStream::GPX2VideoStream() 
	: extclk_()
	, audio_(extclk_)
	, video_(extclk_)
	, thread_audio_(NULL)
	, thread_video_(NULL)
	, audio_device_(NULL)
	, container_(NULL) {
	log_call();

	is_init_ = false;

	// Texture
	texture_ = 0;
}


GPX2VideoStream::~GPX2VideoStream() {
	log_call();

	stop();
	close();
}


void GPX2VideoStream::set_audio_device(GPX2VideoAudioDevice *audio_device) {
	log_call();

	audio_.set_audio_device(audio_device);

	audio_device_ = audio_device;

	audio_device_->data_requested().connect(sigc::mem_fun(audio_, &GPX2VideoStream::Audio::on_data_requested));
}


MediaContainer * GPX2VideoStream::media(void) {
	log_call();

	return container_;
}


bool GPX2VideoStream::open(MediaContainer *container) {
	log_call();

	AudioStreamPtr audio_stream;
	VideoStreamPtr video_stream;

	// Media container
	container_ = container;

	// Retrieve audio & video streams
	audio_stream = container_->getAudioStream();
	video_stream = container_->getVideoStream();

	// Init clock
	audio_.init();
	video_.init();

	extclk_.init(&extclk_.serial_);

	extclk_.get();

	video().clock().set(video().clock().get(), video().clock().serial_);

//	av_sync_type_ = AV_SYNC_VIDEO_MASTER;
	av_sync_type_ = AV_SYNC_AUDIO_MASTER;
//	av_sync_type_ = AV_SYNC_EXTERNAL_CLOCK;

	frame_drops_late_ = 0;
	frame_drops_early_ = 0;

	// Open & decode input media
	audio_.open(audio_stream);
	video_.open(video_stream);

	return true;
}


void GPX2VideoStream::close(void) {
	log_call();

	audio_.close();
	video_.close();

	if (container_)
		delete container_;

	container_ = NULL;
}


GPX2VideoStream::Clock& GPX2VideoStream::clock(void) {
	return extclk_;
}


GPX2VideoStream::Audio& GPX2VideoStream::audio(void) {
	return audio_;
}


GPX2VideoStream::Video& GPX2VideoStream::video(void) {
	return video_;
}


void GPX2VideoStream::init(void) {
	log_call();

	size_t size;

	uint8_t **buffer;

	size_t queue_size = video_.getQueueSize();

	if (!video_.isOpened())
		return;

	size = video_.size();

	buffer = video_.buffer();

	log_info("Init video buffer size: %ld", size);

	// Create buffers
	glGenVertexArrays(1, &vao_);
	glBindVertexArray(vao_);

	glGenBuffers(1, &vbo_);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
//	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &ebo_);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glGenBuffers(queue_size, pbo_);
	for (size_t i=0; i<queue_size; i++) {
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_[i]);

#if 0
		glBufferData(GL_PIXEL_UNPACK_BUFFER, size, NULL, GL_STREAM_DRAW);

		buffer[i] = (uint8_t *) glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
#else
		GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;

		glBufferStorage(GL_PIXEL_UNPACK_BUFFER, size, 0, flags);

		buffer[i] = (uint8_t *) glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, size, flags);
#endif

		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	}

	// position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) 0);
	// texture attribute
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float)));

	is_init_ = true;
}


void GPX2VideoStream::load(FramePtr frame) {
	log_call();

	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_[frame->index_]);
//	glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

	// texture
	if (!texture_) {
		// video texture
		glGenTextures(1, &texture_);
//		glBindTexture(GL_UNPACK_ALIGNMENT, 1);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); //GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); //GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //GL_LINEAR_MIPMAP_LINEAR); //GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		glPixelStorei(GL_UNPACK_ROW_LENGTH, frame->linesizeBytes() / 4);
		glTexImage2D(GL_TEXTURE_2D, 
				0, 
				GL_RGBA, 
				frame->width(),
				frame->height(),
				0, GL_RGBA, GL_UNSIGNED_BYTE, NULL); //frame->constData());

//		shader->use();
//		shader->set("inputTexture1", 0);
	}
	else {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_);
//		glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
//		glPixelStorei(GL_UNPACK_LSB_FIRST, GL_TRUE);
		glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
		glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glPixelStorei(GL_UNPACK_ROW_LENGTH, frame->linesizeBytes() / 4);
		glTexSubImage2D(GL_TEXTURE_2D, 
				0,
				0, 0, 
				frame->width(),
				frame->height(),
				GL_RGBA, GL_UNSIGNED_BYTE, NULL); //frame->constData());
	}
//	glGenerateMipmap(GL_TEXTURE_2D);

	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}


void GPX2VideoStream::render(GPX2VideoShader *shader) {
	log_call();

	if (!video_.isOpened())
		return;

	// Transformations
	glm::mat4 transform = glm::mat4(1.0f);

	// 
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_);

	//
	shader->use();
	shader->set("transform", transform);

	//
	glBindVertexArray(vao_);
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}


bool GPX2VideoStream::play(void) {
	log_call();

	if (!is_init_ || !video_.isOpened())
		return false;

	// Launch decoder thread
	if (!thread_audio_ && audio_.isOpened()) {
		thread_audio_ = new std::thread([this] {
			audio_.run();
		});
	}

	if (!thread_video_) {
		thread_video_ = new std::thread([this] {
			video_.run();
		});
	}

	return true;
}


void GPX2VideoStream::stop(void) {
	log_call();

	loop_ = false;

	// Wait & destroy
	if (thread_audio_) {
		audio_.stop();

		audio_.notify();

		if (thread_audio_->joinable())
			thread_audio_->join();

		delete thread_audio_;
	}

	if (thread_video_) {
		video_.stop();

		video_.notify();

		if (thread_video_->joinable())
			thread_video_->join();

		delete thread_video_;
	}

	thread_audio_ = NULL;
	thread_video_ = NULL;
}


void GPX2VideoStream::seek(double pos) {
	log_call();

	audio_.seek(pos);
	video_.seek(pos);
}


void GPX2VideoStream::toggle_pause(void) {
	log_call();

	audio_.toggle_pause();
}


bool GPX2VideoStream::withAudio(void) const {
	log_call();

	return audio_.isOpened();
}


bool GPX2VideoStream::withVideo(void) const {
	log_call();

	return video_.isOpened();
}


int GPX2VideoStream::width(void) const {
	log_call();

	if (!container_)
		return 0;

	VideoStreamPtr stream = container_->getVideoStream();

	return round(stream->width() * av_q2d(stream->pixelAspectRatio()));
}


int GPX2VideoStream::height(void) const {
	log_call();

	if (!container_)
		return 0;

	VideoStreamPtr stream = container_->getVideoStream();

	return stream->height();
}


double GPX2VideoStream::duration(void) const {
	log_call();

	if (!container_)
		return 0.0;

	VideoStreamPtr stream = container_->getVideoStream();

	return stream->duration() * av_q2d(stream->timeBase()) * 1000.0;
}


double GPX2VideoStream::timeBase(void) const {
	log_call();

	if (!container_)
		return 0.0;

	VideoStreamPtr stream = container_->getVideoStream();

	return av_q2d(stream->timeBase());
}


int GPX2VideoStream::nbChannels(void) const {
	log_call();

	if (!container_)
		return 0;

	VideoStreamPtr stream = container_->getVideoStream();

	return stream->nbChannels();
}


VideoParams::Format GPX2VideoStream::format(void) const {
	log_call();

	if (!container_)
		return VideoParams::FormatInvalid;

	VideoStreamPtr stream = container_->getVideoStream();

	return stream->format();
}


/**
 * Clock
 */

GPX2VideoStream::Clock::Clock() {
	queue_serial_ = NULL;
}


GPX2VideoStream::Clock::~Clock() {
}


void GPX2VideoStream::Clock::init(int *queue_serial) {
	speed_ = 1.0;
	paused_ = 0;
	queue_serial_ = queue_serial;
	set(NAN, -1);
}


/**
 * Get the current time in microseconds since some unspecified starting point
 * (see av_gettime_relative). Then returns delay in seconds.
 */
double GPX2VideoStream::Clock::get(void) {
	double time;

	if (*queue_serial_ != serial_)
		return NAN;

	if (paused_)
		return pts_;

	time = av_gettime_relative() / 1000000.0;

	return pts_drift_ + time - (time - last_updated_) * (1.0 - speed_);
}


void GPX2VideoStream::Clock::set(double pts, int serial) {
	double time = av_gettime_relative() / 1000000.0;
	set_at(pts, serial, time);
}


void GPX2VideoStream::Clock::set_at(double pts, int serial, double time) {
	pts_ = pts;
	last_updated_ = time;
	pts_drift_ = pts - time;
	serial_ = serial;
}


void GPX2VideoStream::Clock::sync_to_slave(GPX2VideoStream::Clock &slave) {
	double clock = get();
	double slave_clock = slave.get();

	if (!isnan(slave_clock) && (isnan(clock) || fabs(clock - slave_clock) > AV_NOSYNC_THRESHOLD))
		set(slave_clock, slave.serial_);
}


/**
 * Audio
 */

GPX2VideoStream::Audio::Audio(Clock &extclk) 
	: extclk_(extclk)
	, audio_params_(44100, AV_CH_LAYOUT_MONO, AudioParams::FormatSigned16)
	, mutex_()
	, cond_() {
	log_call();

	// Init
	decoder_ = NULL;
	stream_ = NULL;

	is_playing_ = false;

	queue_size_ = 20; //5;

	seek_pos_ = 0.0;
	seek_req_ = false;

	// Init clock
	init();
}


GPX2VideoStream::Audio::~Audio() {
	log_call();
}


void GPX2VideoStream::Audio::init(void) {
	serial_ = 0;

	clock_.init(&serial_);
}


void GPX2VideoStream::Audio::set_audio_device(GPX2VideoAudioDevice *audio_device) {
	log_call();

	audio_device_ = audio_device;
}


GPX2VideoStream::Clock& GPX2VideoStream::Audio::clock(void) {
	return clock_;
}


bool GPX2VideoStream::Audio::isOpened(void) const {
	log_call();

	return (decoder_ != NULL);
}


void GPX2VideoStream::Audio::open(AudioStreamPtr stream) {
	log_call();

	stream_ = stream;

	if (stream) {
		// Audio stream timebase
		audio_params_.setTimeBase(stream->timeBase());

		// Open audio decoder
		decoder_ = Decoder::create();
		decoder_->open(stream);
	}
}


void GPX2VideoStream::Audio::close(void) {
	log_call();

	if (decoder_)
		delete decoder_;

	decoder_ = NULL;
}


void GPX2VideoStream::Audio::toggle_pause(void) {
	size_t length = audio_device_->dataLengthRequired();

	// Clock
	clock_.paused_ = is_playing_;

	// Toggle play / pause
	is_playing_ = !is_playing_;

	// Suspend audio stream
	audio_device_->suspend(!is_playing_);

	if (is_playing_)
		write(length);
}


void GPX2VideoStream::Audio::notify(void) {
	log_call();

	cond_.notify_all();
}


bool GPX2VideoStream::Audio::read(void) {
	log_call();

	int duration = 0;

	AVRational time = {};

	SampleBufferPtr buffer;

	buffer = decoder_->retrieveAudio2(audio_params_, time, duration);

	if (!buffer)
		return false;

	// Lock mutex
	std::lock_guard<std::mutex> lock(mutex_);

	queue_.push_back(buffer);

	return true;
}


void GPX2VideoStream::Audio::write(size_t length) {
	log_call();

	size_t write_size = 0;

//	double write_size = 0.0;

	double audio_clock = NAN;

	int64_t audio_callback_time = av_gettime_relative();

	for (;;) {
		SampleBufferPtr buffer = getSampleBuffer();

		if (buffer == NULL)
			break;

		int size = buffer->size();

		audio_device_->write(
				reinterpret_cast<uint8_t *>(buffer->toRawPtrs().data()[0]),
				size);

		write_size += size;

		// Compute clock with the pts
		audio_clock = buffer->time() + (double) buffer->sampleCount() / buffer->audioParams().sampleRate();
		audio_clock /= 1000.0;

		nextSampleBuffer();

		if (write_size > 2 * length)
			break;
	}

	audio_device_->flush();

	// Update clock
	if (!isnan(audio_clock)) {
		clock_.set_at(
			audio_clock - (double) (audio_device_->bufferLength()) / audio_params_.bytesPerSecond(), 
			serial_, 
			audio_callback_time / 1000000.0
		);
		extclk_.sync_to_slave(clock_);
	}
}


void GPX2VideoStream::Audio::run(void) {
	log_call();

	loop_ = true;

	for (; loop_;) {
		// Read audio data
		if (seek_req_ == true) {
//			log_info("Seeking... flush previous audio samples buffer");

			decoder_->seek((int64_t) (seek_pos_ * AV_TIME_BASE / 1000.0));

			flushSampleBuffer();

			seek_req_ = false;
		}

		// If the queue are full, no need to read more
		if (queue_.size() >= queue_size_) {
			wait();
			continue;
		}

		// Read
		if (read() == false) {
			if (stream_->isEOF()) {
				wait();
				continue;
			}

			break;
		}
	}
}


void GPX2VideoStream::Audio::stop(void) {
	log_call();

	loop_ = false;
}


void GPX2VideoStream::Audio::wait(void) {
	log_call();

	std::unique_lock<std::mutex> lock(mutex_);

    cond_.wait(lock);
}


void GPX2VideoStream::Audio::seek(double pos) {
	log_call();

	if (!seek_req_) {
		seek_pos_ = pos;
		seek_req_ = true;

		cond_.notify_all();
	}
}


void GPX2VideoStream::Audio::on_data_requested(void) {
	log_call();

	if (!is_playing_)
		return;

	write(audio_device_->dataLengthRequired());
}


size_t GPX2VideoStream::Audio::getQueueSize(void) const {
	log_call();

	return queue_size_;
}


void GPX2VideoStream::Audio::nextSampleBuffer(void) {
	log_call();

	// Prepare next buffer
	SampleBufferPtr buffer = getSampleBuffer();
	
	// Delete oldest buffer
	std::lock_guard<std::mutex> lock(mutex_);

	if (queue_.empty())
		return;

	// Destroy frame
	queue_.pop_front();

	cond_.notify_all();
}


SampleBufferPtr GPX2VideoStream::Audio::getSampleBuffer(void) {
	log_call();

	size_t size;
	size_t index = 0;

	std::lock_guard<std::mutex> lock(mutex_);

	if (queue_.empty())
		return NULL;

	size = queue_.size();

	SampleBufferPtr buffer = queue_[index % size];

	return buffer;
}


void GPX2VideoStream::Audio::flushSampleBuffer(void) {
	log_call();

	std::lock_guard<std::mutex> lock(mutex_);

	while (!queue_.empty())
		queue_.pop_front();
}


/**
 * Video
 */

GPX2VideoStream::Video::Video(Clock &extclk)
	: extclk_(extclk)
	, dispatcher_()
	, mutex_()
	, cond_() {
	log_call();

	// Init
	decoder_ = NULL;

	is_playing_ = false;

	index_ = 0;
	queue_size_ = 5; //5;

	frame_time_ = 0;

	seek_pos_ = 0.0;
	seek_req_ = false;

	// Init clock
	init();

	// Video frames buffer
	buffer_ = (uint8_t **) malloc(queue_size_ * sizeof(uint8_t *));
}


GPX2VideoStream::Video::~Video() {
	log_call();

	free(buffer_);
}


void GPX2VideoStream::Video::init(void) {
	serial_ = 0;

	clock_.init(&serial_);
}


GPX2VideoStream::Clock& GPX2VideoStream::Video::clock(void) {
	return clock_;
}


bool GPX2VideoStream::Video::isOpened(void) const {
	log_call();

	return (decoder_ != NULL);
}


void GPX2VideoStream::Video::open(VideoStreamPtr stream) {
	log_call();

	shown_ = 0;

	stream_ = stream;

	if (stream) {
		// Open video decoder
		decoder_ = Decoder::create();
		decoder_->open(stream);
	}
}


void GPX2VideoStream::Video::close(void) {
	log_call();

	if (decoder_)
		delete decoder_;

	decoder_ = NULL;
}


void GPX2VideoStream::Video::notify(void) {
	log_call();

	cond_.notify_all();
}


bool GPX2VideoStream::Video::read(void) {
	log_call();

	int index;

	uint8_t *buffer;

	FramePtr frame;

	AVRational video_time;

	// Texture buffer data
	index = index_;
	index_ = (index_ + 1) % queue_size_;

	buffer = buffer_[index];

	// Retrieve video streams
	video_time = av_div_q(av_make_q(1000 * frame_time_, 1), stream_->frameRate());

	frame = decoder_->retrieveVideo(video_time, buffer);
	frame->index_ = index;

	if (!frame)
		return false;

	// next frame
//	frame_time_ += 1;

	// Lock mutex
	std::lock_guard<std::mutex> lock(mutex_);

	queue_.push_back(frame);

	return true;
}


void GPX2VideoStream::Video::run(void) {
	log_call();

	loop_ = true;

	for (; loop_;) {
//		// 25fps = 40ms
//		// 50fps = 20ms
//		std::this_thread::sleep_for(std::chrono::milliseconds(10)); //1000)); //250));
//
//		if (queue_.size() < 5) {
////			log_info("Read frame %ld", queue_.size());
//
//		read();
//
//			dispatcher_.emit();
//		}

		// Read video data
		if (seek_req_ == true) {
//			log_info("Seeking... flush previous video frames");

			decoder_->seek((int64_t) (seek_pos_ * AV_TIME_BASE / 1000.0));

			flushFrame();

			seek_req_ = false;

			dispatcher_.emit();
		}

		// If the queue are full, no need to read more
		if (queue_.size() >= queue_size_) {
			wait();
			continue;
		}

		// Read
		if (read() == false) {
			if (stream_->isEOF()) {
				wait();
				continue;
			}

			break;
		}
	}
}


void GPX2VideoStream::Video::stop(void) {
	log_call();

	loop_ = false;
}


void GPX2VideoStream::Video::wait(void) {
	log_call();

	std::unique_lock<std::mutex> lock(mutex_);

    cond_.wait(lock);
}


void GPX2VideoStream::Video::seek(double pos) {
	log_call();

	if (!seek_req_) {
		seek_pos_ = pos;
		seek_req_ = true;

		FramePtr frame = getFrame();

		log_info("Stream seek position %s", Datetime::timestamp2string(pos, Datetime::FormatTime).c_str());

		cond_.notify_all();
	}
}


size_t GPX2VideoStream::Video::size(void) const {
	log_call();

	return decoder_->videoSize();
}


size_t GPX2VideoStream::Video::getFrameNbRemaining(void) const {
	log_call();

	return queue_.size() - shown_;
}


size_t GPX2VideoStream::Video::getQueueSize(void) const {
	log_call();

	return queue_size_;
}


void GPX2VideoStream::Video::nextFrame(void) {
	log_call();

	if (!shown_) {
		shown_ = 1;
		return;
	}

	// Prepare next buffer
	FramePtr frame = getFrame();
	
//	if (frame) {
//		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_[frame->index_]);
//		buffer_[frame->index_] = (uint8_t *) glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
//	}

	// Delete oldest frame
	std::lock_guard<std::mutex> lock(mutex_);

	if (queue_.empty())
		return;

	// Destroy frame
	queue_.pop_front();

	cond_.notify_all();
}


FramePtr GPX2VideoStream::Video::getFrame(void) {
	log_call();

	size_t size;
	size_t index = 0;

	std::lock_guard<std::mutex> lock(mutex_);

	if (queue_.empty())
		return NULL;

	size = queue_.size();

	FramePtr frame = queue_[(index + shown_) % size];

	return frame;
}


FramePtr GPX2VideoStream::Video::getNextFrame(void) {
	log_call();

	size_t size;
	size_t index = 0;

	std::lock_guard<std::mutex> lock(mutex_);

	if (queue_.size() < (size_t) (2 + shown_))
		return NULL;

	size = queue_.size();

	FramePtr frame = queue_[(index + shown_ + 1) % size];

	return frame;
}


FramePtr GPX2VideoStream::Video::getLastFrame(void) {
	log_call();

	size_t size;
	size_t index = 0;

	std::lock_guard<std::mutex> lock(mutex_);

	if (queue_.empty())
		return NULL;

	size = queue_.size();

	FramePtr frame = queue_[(index) % size];

	return frame;
}


void GPX2VideoStream::Video::flushFrame(void) {
	log_call();

	std::lock_guard<std::mutex> lock(mutex_);

	while (!queue_.empty())
		queue_.pop_front();

	shown_ = 0;
}


uint8_t **GPX2VideoStream::Video::buffer(void) {
	log_call();

	return buffer_;
}

