#include <iostream>
#include <string>
#include <vector>
#include <exception>
#include <chrono>

//#include <cairomm/context.h>

#include <epoxy/gl.h>
#include <gdkmm/general.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

extern "C" {
#include <libavutil/time.h>
}

#include "../../src/oiioutils.h"
#include "log.h"
#include "compat.h"
#include "datetime.h"
#include "area.h"


// polls for possible required screen refresh at least this often, should be less than 1/fps
#define REFRESH_RATE 0.005


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
	: thread_(NULL)
	, dispatcher_()
	, mutex_()
	, cond_()
	, container_(NULL)
	, decoder_video_(NULL) 
	, frame_(NULL) {
	log_call();

	is_init_ = false;

	index_ = 0;
	queue_size_ = 5;

	frame_time_ = 0;

	seek_pos_ = 0.0;
	seek_req_ = false;

	// Texture
	texture_ = 0;

	// Buffer
	buffer_ = (uint8_t **) malloc(queue_size_ * sizeof(uint8_t *));
}


GPX2VideoStream::~GPX2VideoStream() {
	log_call();

	stop();
	close();
}


MediaContainer * GPX2VideoStream::media(void) {
	log_call();

	return container_;
}


//bool GPX2VideoStream::open(const Glib::ustring &video_file) {
bool GPX2VideoStream::open(MediaContainer *container) {
	log_call();

	VideoStreamPtr video_stream;

	// Media container
	container_ = container;

//	// Probe input media
//	container_ = Decoder::probe(video_file);

	// Retrieve audio & video streams
	video_stream = container_->getVideoStream();

	// Open & decode input media
	decoder_video_ = Decoder::create();
	decoder_video_->open(video_stream);

	return true;
}


void GPX2VideoStream::close(void) {
	log_call();

	if (decoder_video_)
		delete decoder_video_;

	if (container_)
		delete container_;

	decoder_video_ = NULL;
	container_ = NULL;
}


void GPX2VideoStream::init(void) {
	log_call();

	size_t size;

	if (decoder_video_ == NULL)
		return;

	size = decoder_video_->videoSize();

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

	glGenBuffers(queue_size_, pbo_);
	for (size_t i=0; i<queue_size_; i++) {
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_[i]);
		glBufferData(GL_PIXEL_UNPACK_BUFFER, size, NULL, GL_STREAM_DRAW);

		buffer_[i] = (uint8_t *) glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
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
	glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

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

	if (decoder_video_ == NULL)
		return;

	// Transformations
	glm::mat4 transform = glm::mat4(1.0f);

	// 
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_);
//	glActiveTexture(GL_TEXTURE1);
//	glBindTexture(GL_TEXTURE_2D, widgets_texture_);

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

	if (!is_init_ || !decoder_video_)
		return false;

	// Launch decoder thread
	if (!thread_) {
		thread_ = new std::thread([this] {
			run();
		});
	}

	return true;
}


void GPX2VideoStream::stop(void) {
	log_call();

	loop_ = false;

	// Wait & destroy
	if (thread_) {
		cond_.notify_all();

		if (thread_->joinable())
			thread_->join();

		delete thread_;
	}

	thread_ = NULL;
}


void GPX2VideoStream::seek(double pos) {
	log_call();

	if (!seek_req_) {
		seek_pos_ = pos;
		seek_req_ = true;

		FramePtr frame = getFrame();

		log_info("Stream seek position %s", Datetime::timestamp2string(pos, Datetime::FormatTime).c_str());

		cond_.notify_all();
	}
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

void GPX2VideoStream::nextFrame(void) {
	log_call();

	// Prepare next buffer
	FramePtr frame = getFrame();
	
	if (frame) {
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_[frame->index_]);
		buffer_[frame->index_] = (uint8_t *) glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
	}

	// Delete oldest frame
	std::lock_guard<std::mutex> lock(mutex_);

	if (queue_.empty())
		return;

	// Destroy frame
	queue_.pop_front();

	cond_.notify_all();
}


FramePtr GPX2VideoStream::getFrame(void) {
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


FramePtr GPX2VideoStream::getNextFrame(void) {
	log_call();

	size_t size;
	size_t index = 0;

	std::lock_guard<std::mutex> lock(mutex_);

	if (queue_.empty())
		return NULL;

	size = queue_.size();

	FramePtr frame = queue_[(index + 1) % size];

	return frame;
}


void GPX2VideoStream::flushFrame(void) {
	log_call();

	std::lock_guard<std::mutex> lock(mutex_);

	while (!queue_.empty())
		queue_.pop_front();
}


double GPX2VideoStream::getFrameDuration(void) {
	log_call();

	FramePtr frame = getFrame();
	FramePtr nextframe = getNextFrame();
	
	if (frame && nextframe) {
		double duration = nextframe->timestamp() - frame->timestamp();

		return duration * av_q2d(frame->videoParams().timeBase());
	}

	return 0.0;
}


bool GPX2VideoStream::read(void) {
	log_call();

	int index;

	uint8_t *buffer;

	AVRational video_time;

	// Texture buffer data
	index = index_;
	index_ = (index_ + 1) % queue_size_;
//	index = (index_ + 1) % 2;

	buffer = buffer_[index];

	// Retrieve audio & video streams
	VideoStreamPtr video_stream = container_->getVideoStream();

	video_time = av_div_q(av_make_q(1000 * frame_time_, 1), video_stream->frameRate());

	frame_ = decoder_video_->retrieveVideo(video_time, buffer);
	frame_->index_ = index;

	if (!frame_)
		return false;

	// next frame
	frame_time_ += 1;

	// Lock mutex
	std::lock_guard<std::mutex> lock(mutex_);

	queue_.push_back(frame_);

	return true;
}


void GPX2VideoStream::wait(void) {
	log_call();

	std::unique_lock<std::mutex> lock(mutex_);

    cond_.wait(lock);
}


void GPX2VideoStream::run(void) {
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
			log_info("Seeking... flush previous video frames");

			decoder_video_->seek((int64_t) (seek_pos_ * AV_TIME_BASE / 1000.0));

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
		if (read() == false)
			break;
	}
}








GPX2VideoArea::GPX2VideoArea(GPXApplication &app)
	: Glib::ObjectBase("GPX2VideoArea") 
	, app_(app) {
	log_call();
}


GPX2VideoArea::GPX2VideoArea(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &ref_builder, GPXApplication &app) 
	: Glib::ObjectBase("GPX2VideoArea")
	, Gtk::GLArea(cobject)
	, ref_builder_(ref_builder)
	, adjustment_(NULL)
	, shader_(NULL)
	, app_(app) 
	, source_(NULL) 
	, renderer_(NULL) {
	log_call();

	//
	is_init_ = false;
	is_playing_ = false;
	is_seeking_ = false;
	force_refresh_ = false;

	//
	is_gl_context_ready_ = false;

//#if GTKMM_CHECK_VERSION(4, 12, 0)
//	set_allowed_apis(Gdk::GLApi::GL);
//#else
//	set_use_es(false);
//#endif
	set_expand(true);
//	set_size_request(320, 240);
	set_auto_render(true);

	stream_.data_ready().connect(sigc::mem_fun(*this, &GPX2VideoArea::on_data_ready));

	// Creation of a new object prevents long lines and shows us a little
	// how slots work.  We have 0 parameters and bool as a return value
	// after calling sigc::bind.
	refresh_slot_ = sigc::bind(sigc::mem_fun(*this, &GPX2VideoArea::on_timeout));

	// Connect signals
//	signal_realize().connect(sigc::mem_fun(*this, &GPX2VideoArea::realize));
//	signal_unrealize().connect(sigc::mem_fun(*this, &GPX2VideoArea::unrealize), false);
//	signal_render().connect(sigc::mem_fun(*this, &GPX2VideoArea::on_render), false);
	signal_resize().connect(sigc::mem_fun(*this, &GPX2VideoArea::on_resize), false);
}


GPX2VideoArea::~GPX2VideoArea() {
	log_call();

	close_stream();
}


void GPX2VideoArea::set_renderer(GPX2VideoRenderer *renderer) {
	log_call();

	renderer_ = renderer;
}


void GPX2VideoArea::set_adjustment(Glib::RefPtr<Gtk::Adjustment> adjustment) {
	log_call();

	adjustment_ = adjustment;
}


void GPX2VideoArea::configure_adjustment(void) {
	log_call();

	double duration = stream_.duration();

	log_info("Video duration: %s", Datetime::timestamp2string(duration, Datetime::FormatTime).c_str());

	adjustment_->configure(0.0, 0.0, duration, 1.0, 10.0, 0.0); //60.0);
}

void GPX2VideoArea::update_adjustment(double value) {
	log_call();

	if (is_seeking_)
		return;

	if (!is_playing_)
		log_info("Update scale position position: %s", Datetime::timestamp2string(value, Datetime::FormatTime).c_str());

	adjustment_->set_value(value);
}


void GPX2VideoArea::update_layout(void) {
	log_call();

	// Widgets resize
	widgets_resize(stream_.width(), stream_.height());

	if (!is_gl_context_ready_)
		return;

	// 
	init_widgets_buffers();

	// 
	refresh();
}


TelemetrySource * GPX2VideoArea::telemetry(void) {
	return source_;
}


void GPX2VideoArea::set_telemetry(TelemetrySource *source) {
	// Save telemetry source
	source_ = source;

	//
	refresh();
}


//void GPX2VideoArea::open_stream(const Glib::ustring &video_file) {
void GPX2VideoArea::open_stream(MediaContainer *container) {
	log_call();

	VideoStreamPtr video_stream;

//	// Reset
//	real_duration_ms_ = 0;
//	last_timecode_ms_ = 0;

	// Retrieve audio & video streams
	video_stream = container->getVideoStream();

	// Open stream
	stream_.open(container);

	// 
	if (is_gl_context_ready_)
		init_video_buffers();

	// Go
	stream_.play();

	// Widgets resize
	widgets_resize(stream_.width(), stream_.height());

	// Update UI
	is_init_ = false;
	force_refresh_ = true;

	schedule_refresh(100);

	resize_viewport(get_width(), get_height());

	configure_adjustment();
}


void GPX2VideoArea::close_stream(void) {
	log_call();

	// Close stream
	stream_.close();
}


void GPX2VideoArea::stream_toggle_pause(void) {
	log_call();

	// Toggle play  / pause
	is_init_ = false;
	is_playing_ = !is_playing_;
}


void GPX2VideoArea::toggle_pause(void) {
	log_call();

	is_step_ = false;

	stream_toggle_pause();
	schedule_refresh(100);
}


void GPX2VideoArea::step_to_next_frame(void) {
	log_call();

	if (!is_playing_)
		toggle_pause();

	is_step_ = true;
}


void GPX2VideoArea::seek(double incr) {
	log_call();

	double pos;

	// Get frame
	FramePtr frame = stream_.getFrame();

	if (!frame)
		return;

	// OpenGL context
	make_current();

	// Reset telemetry data from start
	if ((incr < 0) && (source_ != NULL))
		source_->retrieveFrom(data_);

	// Force to refresh widgets
	widgets_clear();

	// Compute new video position (in ms)
	pos = frame->time();
	pos += incr * 1000.0;

	stream_.seek(pos);
}


void GPX2VideoArea::seeking(bool status) {
	log_call();

//	is_seeking_ = status;
}


void GPX2VideoArea::video_render(void) {
	log_call();

	stream_.render(shader_);
}


//void GPX2VideoArea::widget_append(VideoWidget *widget) {
//	log_call();
//
//	widget_ = GPX2VideoWidget::create(widget);
//}


void GPX2VideoArea::widgets_draw(void) {
	log_call();

////	// Reset overlay layer
////	overlay_->reset(OIIO::ImageSpec(stream_.width(), stream_.height(), 
////		stream_.nbChannels(), OIIOUtils::getOIIOBaseTypeFromFormat(stream_.format())));

	// Draw each widget
	for (GPX2VideoWidget *item : renderer_->widgets_)
		item->draw(data_);
}


void GPX2VideoArea::widgets_render(void) {
	log_call();

	for (GPX2VideoWidget *item : renderer_->widgets_)
		item->render(shader_);
}


void GPX2VideoArea::widgets_resize(int width, int height) {
	log_call();

	log_info("Set widget layout %d x %d", width, height);

	for (GPX2VideoWidget *item : renderer_->widgets_)
		item->setLayoutSize(width, height);
}


void GPX2VideoArea::widgets_clear(void) {
	log_call();

	for (GPX2VideoWidget *item : renderer_->widgets_)
		item->clear();
}


void GPX2VideoArea::on_realize(void) {
	log_call();

	// Call parent
	Gtk::GLArea::on_realize();

	// OpenGL context
	make_current();

	try {
		throw_if_error();

		const bool use_es = get_context()->get_use_es();

		const std::string vertex_path = use_es ? "/com/progweb/gpx2video/gl/glarea-gles.vs.glsl" : "/com/progweb/gpx2video/gl/glarea-gl.vs.glsl";
		const std::string fragment_path = use_es ? "/com/progweb/gpx2video/gl/glarea-gles.fs.glsl" : "/com/progweb/gpx2video/gl/glarea-gl.fs.glsl";

		log_info("%s", use_es ? "Use OpenGL ES" : "Use OpenGL");

		shader_ = GPX2VideoShader::create(vertex_path, fragment_path);

		init_video_buffers();
		init_widgets_buffers();

		// TODO
		stream_.play();

		// GL context is now ready
		is_gl_context_ready_ = true;
	}
	catch(const Gdk::GLError &gle) {
		std::cerr << "An error occured making the context current during realize:" << std::endl;
		std::cerr << gle.domain() << "-" << gle.code() << "-" << gle.what() << std::endl;
	}
}


void GPX2VideoArea::on_unrealize(void) {
	log_call();

	// OpenGL context
	make_current();

	try {
		throw_if_error();

//		// Delete buffers and program
//		glDeleteBuffers(1, &ebo_);
//		glDeleteBuffers(1, &vbo_);
//		glDeleteBuffers(1, &vao_);

		if (shader_)
			delete shader_;
	}
	catch(const Gdk::GLError &gle) {
		std::cerr << "An error occured making the context current during unrealize" << std::endl;
		std::cerr << gle.domain() << "-" << gle.code() << "-" << gle.what() << std::endl;
	}

	// Call parent
	Gtk::GLArea::on_unrealize();
}


bool GPX2VideoArea::on_render(const Glib::RefPtr<Gdk::GLContext> &context) {
	log_call();

	(void) context;

	try {
		throw_if_error();

		//
		glClearColor(0.0, 0.0, 0.3, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);

		//
		video_render();

		//
		widgets_render();

		//
		glBindVertexArray(0);
		glUseProgram(0);

		glFlush();

		return true;
	}
	catch (const Gdk::GLError &gle) {
		std::cerr << "An error occurred in the render callback of the GLArea" << std::endl;
		std::cerr << gle.domain() << "-" << gle.code() << "-" << gle.what() << std::endl;
		return false;
	}
}


//
// Diego macario - how to keep the aspect ratio of an opengl window constant
//
void GPX2VideoArea::on_resize(gint width, gint height) {
	log_call();

	resize_viewport(width, height);
}


void GPX2VideoArea::on_data_ready(void) {
	log_call();

	if (!is_playing_)
		step_to_next_frame();
}


bool GPX2VideoArea::on_timeout(void) {
	log_call();

	double remaining_time = 0.0;

	for (;;) {
        remaining_time = 0.0;

		if (!video_refresh(remaining_time)) {
			// Retry later
			schedule_refresh(100);
			break;
		}

        if (remaining_time > 0.0) {
			// Convert in ms
			remaining_time *= 1000.0;

			if (remaining_time < 1.0) {
				// Just sleep
				av_usleep((int64_t) (remaining_time * 1000.0));
				continue;
			}

		}

		schedule_refresh((unsigned int) remaining_time);
		break;
	}

	return false;
}


void GPX2VideoArea::refresh(void) {
	log_call();

	// Reset telemetry data from start
	if (source_ != NULL)
		source_->retrieveFrom(data_);

	// Force to draw widget again
	widgets_clear();
	
	// Render video again
	force_refresh_ = true;

	schedule_refresh(0);
}


void GPX2VideoArea::schedule_refresh(unsigned int delay) {
	log_call();

	if (!is_playing_ && !force_refresh_)
		return;

	// This is where we connect the slot to the Glib::signal_timeout() 
	// (interval time in ms)
	timer_ = Glib::signal_timeout().connect(refresh_slot_, delay);
}


bool GPX2VideoArea::video_refresh(double &remaining_time) {
	log_call();

// no AV sync correction is done if below the minimum AV sync threshold
#define AV_SYNC_THRESHOLD_MIN 0.04
// AV sync correction is done if above the maximum AV sync threshold
#define AV_SYNC_THRESHOLD_MAX 0.1

	double time;
	double delay;
	double duration;

	// Get frame
	FramePtr frame = stream_.getFrame();

	if (!frame)
		return false;

	// OpenGL context
	make_current();

	if (is_playing_) {
		time = av_gettime_relative() / 1000000.0;

		if (!is_init_) {
			frame_timer_ = time;
			is_init_ = true;
		}

		// Get frame delay
		duration = stream_.getFrameDuration();
		delay = duration;

//		log_debug("Time:\t\t\t\t%f", time);
//		log_debug("Frame timer:\t\t\t%f", frame_timer_);
//		log_debug("PTS Duration:\t\t\t%f", duration); 

		if (time < frame_timer_ + delay) {
			remaining_time = frame_timer_ + delay - time;

//			log_debug("Remaining time:\t\t\t%f", remaining_time);
		}
		else {
			frame_timer_ += delay;
			if ((delay > 0) && ((time - frame_timer_) > AV_SYNC_THRESHOLD_MAX))
				frame_timer_ = time;

			// Drop frame
			// ...

			// Next frame
			stream_.nextFrame();
			force_refresh_ = true;

			// Step frame
			if (is_step_ && is_playing_)
				stream_toggle_pause();
		}

//		log_debug("");
	}

	// Display frame
	if (force_refresh_)
		video_display();

	// Refresh done
	force_refresh_ = false;

	return true;
}


void GPX2VideoArea::video_display(void) {
	log_call();

	uint64_t start_time;

	uint64_t timecode_ms;

	double time_factor;

	unsigned int real_duration_ms;

	time_factor = 1.0;

	start_time = stream_.media()->startTime() + stream_.media()->timeOffset();

//	// OpenGL context
//	make_current();

	// Get frame
	FramePtr frame = stream_.getFrame();

	if (!frame)
		return;

	timecode_ms = frame->time();

	// Compute real time, so assume time_factor is constant 
	// todo: fix later since time_factor is variable
	real_duration_ms = time_factor * timecode_ms;

	// Update video real time 
	app_.setTime(start_time + real_duration_ms);

	if (source_) {
		uint64_t timestamp = start_time + real_duration_ms;

		// Read GPX data
		timestamp -= (timestamp % renderer_->telemetrySettings().telemetryRate());
		source_->retrieveNext(data_, timestamp);
	}

	// Draw widgets
	widgets_draw();

	// Create & load OpenGL texture
	load_video_texture(frame);
	load_widgets_texture(frame);

	// OpenGL context redraw
	queue_draw();

	// Refresh adjustment
	update_adjustment(frame->time());

//	// Compute real time by step, since time_factor is variable
//	real_duration_ms = timecode_ms - last_timecode_ms_;
//	real_duration_ms_ += time_factor * real_duration_ms;

//	last_timecode_ms_ = timecode_ms;
}


void GPX2VideoArea::init_video_buffers(void) {
	log_call();

	// OpenGL context
	make_current();

	// Play
	stream_.init();
}


void GPX2VideoArea::init_widgets_buffers(void) {
	log_call();

	// OpenGL context
	make_current();

	for (GPX2VideoWidget *item : renderer_->widgets_)
		item->init_buffers();
}


void GPX2VideoArea::load_video_texture(FramePtr frame) {
	log_call();

	if (!frame)
		return;

	stream_.load(frame);

//	log_info("width: %d x height: %d - linesize: %d", 
//			frame->width(), frame->height(), frame->linesizeBytes());
//
//	// Status
//	log_info("Frame PTS: %ld - TS: %ld ms", 
//			frame->timestamp(),
//			(uint64_t) (frame->timestamp() * 1000 * av_q2d(frame->videoParams().timeBase())));

	check_gl_error();
}


void GPX2VideoArea::load_widgets_texture(FramePtr frame) {
	log_call();


	for (GPX2VideoWidget *item : renderer_->widgets_)
		item->load_texture();

//	log_call();
//
//	if (!frame)
//		return;
//
//	// 
//	int nchannels = 4;
//	static std::vector<unsigned char> m_tex_buffer;
//
//	if (!widgets_texture_) {
//		m_tex_buffer.resize(overlay_->spec().width * overlay_->spec().height * nchannels
//							* overlay_->spec().channel_bytes());
//	}
//
//	overlay_->get_pixels(OIIO::ROI(), 
//			overlay_->spec().format, 
//			reinterpret_cast<char*>(m_tex_buffer.data()));
//
//	// texture
//	if (!widgets_texture_) {
//		glGenTextures(1, &widgets_texture_);
////		glBindTexture(GL_UNPACK_ALIGNMENT, 1);
//		glActiveTexture(GL_TEXTURE1);
//		glBindTexture(GL_TEXTURE_2D, widgets_texture_);
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); //GL_CLAMP);
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); //GL_CLAMP);
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //GL_LINEAR_MIPMAP_LINEAR); //GL_LINEAR);
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//
////		glPixelStorei(GL_UNPACK_ROW_LENGTH, frame->linesizeBytes() / 4);
////		glTexImage2D(GL_TEXTURE_2D, 
////				0, 
////				GL_RGBA, 
////				frame->width(),
////				frame->height(),
////				0, GL_RGBA, GL_UNSIGNED_BYTE, frame->constData());
//		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0); //frame->linesizeBytes() / 4);
//		glTexImage2D(GL_TEXTURE_2D, 
//				0, 
//				GL_RGBA, 
//				overlay_->spec().width,
//				overlay_->spec().height,
//				0, GL_RGBA, GL_UNSIGNED_BYTE, m_tex_buffer.data());
//
//		shader_->use();
//		shader_->set("inputTexture2", 1);
//	}
//	else {
//		glActiveTexture(GL_TEXTURE1);
//		glBindTexture(GL_TEXTURE_2D, widgets_texture_);
////		glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
////		glPixelStorei(GL_UNPACK_LSB_FIRST, GL_TRUE);
//		glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
//		glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
//		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
//
////		glPixelStorei(GL_UNPACK_ROW_LENGTH, frame->linesizeBytes() / 4);
////		glTexSubImage2D(GL_TEXTURE_2D, 
////				0,
////				0, 0, 
////				frame->width(),
////				frame->height(),
////				GL_RGBA, GL_UNSIGNED_BYTE, frame->constData());
//		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
//		glTexSubImage2D(GL_TEXTURE_2D, 
//				0, 
//				0, 0,
//				overlay_->spec().width,
//				overlay_->spec().height,
//				GL_RGBA, GL_UNSIGNED_BYTE, m_tex_buffer.data());
//	}
//
//	check_gl_error();
}


void GPX2VideoArea::resize_viewport(gint width, gint height) {
	log_call();

	float desiredAspectRatio = 1.0;

	int widthOfViewport = 1.0, heightOfViewport = 1.0;
	int lowerLeftCornerOfViewportX = 0, lowerLeftCornerOfViewportY = 0;

	float requiredWidthOfViewport, requiredHeightOfViewport;

	make_current();

	// Compute ratio
	if ((stream_.width() > 0) && (stream_.height())) {
		desiredAspectRatio = (double) stream_.width() / (double) stream_.height();
	}

	// Compute width x height layout
	requiredHeightOfViewport = width * (1.0f / desiredAspectRatio);

	if (requiredHeightOfViewport > height) {
		requiredWidthOfViewport = height * desiredAspectRatio;

		if (requiredWidthOfViewport > width) {
			std::cout << "can't find size..." << std::endl;
		}
		else {
			widthOfViewport = static_cast<int>(requiredWidthOfViewport);
			heightOfViewport = height;

			lowerLeftCornerOfViewportX = static_cast<int>((width - widthOfViewport) / 2.0f);
			lowerLeftCornerOfViewportY = 0;
		}
	}
	else {
		widthOfViewport = width;
		heightOfViewport = static_cast<int>(requiredHeightOfViewport);

		lowerLeftCornerOfViewportX = 0;
		lowerLeftCornerOfViewportY = static_cast<int>((height - heightOfViewport) / 2.0f);
	}

	glViewport(lowerLeftCornerOfViewportX, lowerLeftCornerOfViewportY,
			widthOfViewport, heightOfViewport);
}


void GPX2VideoArea::check_gl_error(void) {
	GLenum err;

	while ((err = glGetError()) != GL_NO_ERROR) {
		std::string error;

		switch(err) {
			case GL_INVALID_OPERATION:
				error="INVALID_OPERATION";
				break;
			case GL_INVALID_ENUM:
				error="INVALID_ENUM";
				break;
			case GL_INVALID_VALUE:
				error="INVALID_VALUE";
				break;
			case GL_OUT_OF_MEMORY:
				error="OUT_OF_MEMORY";
				break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:
				error="INVALID_FRAMEBUFFER_OPERATION";
				break;
		}

		log_error("GL '%d' error: %s", err, error.c_str());
	}
}

