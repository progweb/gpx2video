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

// TST BEGIN
extern "C" {
#include <libavformat/avio.h>
#include <libavutil/bprint.h>
}
// TST END

#include <pulse/error.h>

#include "../../src/oiioutils.h"
#include "log.h"
#include "compat.h"
#include "datetime.h"
#include "area.h"


// polls for possible required screen refresh at least this often, should be less than 1/fps
#define REFRESH_RATE 0.01 // 0.05



GPX2VideoArea::GPX2VideoArea(GPXApplication &app)
	: Glib::ObjectBase("GPX2VideoArea") 
	, app_(app) 
	, audio_device_(NULL) {
	log_call();
}


GPX2VideoArea::GPX2VideoArea(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &ref_builder, GPXApplication &app) 
	: Glib::ObjectBase("GPX2VideoArea")
	, Gtk::GLArea(cobject)
	, ref_builder_(ref_builder)
	, adjustment_(NULL)
	, shader_(NULL)
	, app_(app) 
	, audio_device_(NULL)
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
	set_auto_render(false);

	stream_.video().data_ready().connect(sigc::mem_fun(*this, &GPX2VideoArea::on_data_ready));

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
	
	renderer_->ready().connect(sigc::mem_fun(*this, &GPX2VideoArea::on_renderer_ready));
}


void GPX2VideoArea::set_audio_device(GPX2VideoAudioDevice *audio_device) {
	log_call();

	stream_.set_audio_device(audio_device);

	audio_device_ = audio_device;
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
	renderer_->set_layout_size(stream_.width(), stream_.height());

	if (!is_gl_context_ready_)
		return;

	// 
	init_widgets_buffers();

	// 
	refresh();
}


//void GPX2VideoArea::set_telemetry(TelemetrySource *source) {
//	// Save telemetry source
//	source_ = source;
//
//	//
//	refresh();
//}


//void GPX2VideoArea::open_stream(const Glib::ustring &video_file) {
void GPX2VideoArea::open_stream(MediaContainer *container) {
	log_call();

	VideoStreamPtr video_stream;

//	// Reset
//	real_duration_ms_ = 0;
//	last_timecode_ms_ = 0;

	// Audio
	audio_device_->connect();

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
	renderer_->set_layout_size(stream_.width(), stream_.height());

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

	// Audio
	if (audio_device_)
		audio_device_->disconnect();
}


void GPX2VideoArea::stream_toggle_pause(void) {
	log_call();

	if (!is_playing_) {
		frame_timer_ += av_gettime_relative() / 1000000.0 - stream_.video().clock().last_updated_;
		stream_.video().clock().paused_ = 0;
		stream_.video().clock().set(stream_.video().clock().get(), stream_.video().clock().serial_);
	}

	// Clock
	stream_.clock().set(stream_.clock().get(), stream_.clock().serial_);

	stream_.audio().clock().paused_ = stream_.video().clock().paused_ = stream_.clock().paused_ = is_playing_;

	// Toggle play  / pause
	is_init_ = false;
	is_playing_ = !is_playing_;

	// Audio
	stream_.toggle_pause();
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
	FramePtr frame = stream_.video().getFrame();

	if (!frame)
		return;

	// OpenGL context
	make_current();

	// Force to refresh widgets
	pos = renderer_->time();
	pos += (incr * 1000.0);

	renderer_->seek(pos);

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

//	log_info("R: %7.3f", stream_.get_master_clock());

	try {
		throw_if_error();

		//
		glClearColor(0.0, 0.0, 0.3, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);

		//
		video_render();

		// Widgets rendering
		renderer_->render(shader_);

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


void GPX2VideoArea::on_renderer_ready(void) {
	log_call();

	if (!is_playing_)
		refresh();
}


bool GPX2VideoArea::on_timeout(void) {
	log_call();

//	static double time = 0.0;

	double remaining_time = 0.0;

//	log_info("T: %7.3f", stream_.get_master_clock());

//	time += 0.0100;
//
//	stream_.update_video_pts(time, 0);
//
//	schedule_refresh(100);
//
//	queue_draw();
//
//	return false;



	for (;;) {
        remaining_time = REFRESH_RATE; //0.0;

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

// TODO
//	// Reset telemetry data from start
//	if (source_ != NULL)
//		source_->retrieveFrom(data_);

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
	double last_duration;

//	log_info("U: %7.3f", stream_.get_master_clock());

//	// OpenGL context
//	make_current();

retry:
	// Get frame
	FramePtr frame = stream_.video().getFrame();
	FramePtr lastframe = stream_.video().getLastFrame();

	if (!frame)
		return false;

	if (is_playing_) {
		time = av_gettime_relative() / 1000000.0;

		if (!is_init_) {
			frame_timer_ = time;
			is_init_ = true;
		}

		// Compute nominal last_duration
		last_duration = stream_.vp_duration(lastframe, frame);
		delay = stream_.compute_target_delay(last_duration);

//		log_debug("Time:\t\t\t\t%f", time);
//		log_debug("Frame timer:\t\t\t%f", frame_timer_);
//		log_debug("PTS last duration:\t\t\t%f", last_duration); 
//		log_debug("Delay:\t\t\t%f", delay); 

		if (time < frame_timer_ + delay) {
//			remaining_time = frame_timer_ + delay - time;
			remaining_time = FFMIN(frame_timer_ + delay - time, remaining_time);

			log_debug("Remaining time:\t\t\t%f", remaining_time);
		}
		else {
			frame_timer_ += delay;
			if ((delay > 0) && ((time - frame_timer_) > AV_SYNC_THRESHOLD_MAX))
				frame_timer_ = time;

			stream_.update_video_pts(frame->time() / 1000.0, 0);

			// Drop frame
			if (stream_.video().getFrameNbRemaining() > 1) {
				double duration;

				FramePtr nextFrame = stream_.video().getNextFrame();

				duration = stream_.vp_duration(frame, nextFrame);

				if (!is_step_
						&& (stream_.get_master_sync_type() != AV_SYNC_VIDEO_MASTER) 
						&& (time > frame_timer_ + duration)) {
					stream_.frame_drops_late_++;
					stream_.video().nextFrame();
					goto retry;
				}
			}

			// Next frame
			stream_.video().nextFrame();
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

#if 0
	{
		AVBPrint buf;
		static int64_t last_time;
		int64_t cur_time;
		double av_diff;

		cur_time = av_gettime_relative();

		if (!last_time || (cur_time - last_time) >= 30000) {
			av_diff = 0;
			if (stream_.withAudio() && stream_.withVideo())
				av_diff = stream_.audio().clock().get() - stream_.video().clock().get();
			else if (stream_.withAudio())
				av_diff = stream_.get_master_clock() - stream_.audio().clock().get();
			else if (stream_.withVideo())
				av_diff = stream_.get_master_clock() - stream_.video().clock().get();

			av_bprint_init(&buf, 0, AV_BPRINT_SIZE_AUTOMATIC);
			av_bprintf(&buf,
					"%7.2f %s: A%s: %7.3f / V%s: %7.3f - %7.3f fd=%4d \r",
					stream_.clock().get(),
					(stream_.withAudio() && stream_.withVideo()) ? "A-V" : (stream_.withVideo() ? "M-V" : (stream_.withAudio() ? "M-A" : "   ")),
					stream_.get_master_sync_type() == AV_SYNC_AUDIO_MASTER ? "*" : "",
					stream_.audio().clock().get(),
					stream_.get_master_sync_type() == AV_SYNC_VIDEO_MASTER ? "*" : "",
					stream_.video().clock().get(),
					av_diff,
					stream_.frame_drops_early_ + stream_.frame_drops_late_);

			if (is_playing_)
				fprintf(stderr, "%s", buf.str);
			else
				fprintf(stderr, "\n");

			av_bprint_finalize(&buf, NULL);

			last_time = cur_time;
		}
	}
#endif

	return true;
}


void GPX2VideoArea::video_display(void) {
	log_call();

	uint64_t datetime;
	uint64_t start_time;

	uint64_t timecode_ms;

	double time_factor;

	unsigned int real_duration_ms;

//	log_info("D: %7.3f", stream_.get_master_clock());

	time_factor = 1.0;

	start_time = stream_.media()->startTime() + stream_.media()->timeOffset();

	// OpenGL context
	make_current();

	// Get frame
	FramePtr frame = stream_.video().getFrame();

	if (!frame) {
		log_warn("No frame");
		return;
	}

	timecode_ms = frame->time();

	// Compute real time, so assume time_factor is constant 
	// todo: fix later since time_factor is variable
	real_duration_ms = time_factor * timecode_ms;

	// Update video real time 
	datetime = start_time + real_duration_ms;

//	if (source_) {
//		uint64_t timestamp = start_time + real_duration_ms;
//
//		// Read GPX data
//		timestamp -= (timestamp % renderer_->telemetrySettings().telemetryRate());
//		source_->retrieveNext(data_, timestamp);
//	}

	// Notify renderer
	renderer_->update(datetime);

//	// Draw widgets
//	renderer_->draw();

	// Create & load OpenGL texture
	load_video_texture(frame);
	load_widgets_texture(frame);

	// OpenGL context redraw
//	log_info("Q: %7.3f", stream_.get_master_clock());
//	queue_draw();
	queue_render();

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

	// Widget buffers initialization
	renderer_->init_buffers();
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

	renderer_->load_texture();

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

		switch (err) {
		case GL_INVALID_OPERATION:
			error = "INVALID_OPERATION";
			break;
		case GL_INVALID_ENUM:
			error = "INVALID_ENUM";
			break;
		case GL_INVALID_VALUE:
			error = "INVALID_VALUE";
			break;
		case GL_OUT_OF_MEMORY:
			error = "OUT_OF_MEMORY";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			error = "INVALID_FRAMEBUFFER_OPERATION";
			break;
		}

		log_error("GL '%d' error: %s", err, error.c_str());
	}
}

