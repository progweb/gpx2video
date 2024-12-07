#include <iostream>
#include <string>
#include <vector>
#include <exception>
#include <chrono>

//#include <cairomm/context.h>

#include <gdkmm/general.h>
#include <gtkmm/version.h>
#include <epoxy/gl.h>

extern "C" {
#include <libavutil/time.h>
}

#include "log.h"
#include "area.h"


GPX2VideoStream::GPX2VideoStream() 
	: thread_(NULL)
//	, dispatcher_()
	, mutex_()
	, container_(NULL)
	, decoder_video_(NULL) 
	, frame_(NULL) {
	log_call();

	frame_time_ = 0;

//	dispatcher_.connect(sigc::mem_fun(*this, &GPX2VideoStream::on_data_ready));
}


GPX2VideoStream::~GPX2VideoStream() {
	log_call();

	// Wait & destroy
	if (thread_) {
		if (thread_->joinable())
			thread_->join();

		delete thread_;
	}

	if (decoder_video_)
		delete decoder_video_;
	if (container_)
		delete container_;
}


bool GPX2VideoStream::open(const Glib::ustring &video_file) {
	log_call();

	VideoStreamPtr video_stream;

	// Probe input media
	container_ = Decoder::probe(video_file);

	// Retrieve audio & video streams
	video_stream = container_->getVideoStream();

	// Open & decode input media
	decoder_video_ = Decoder::create();
	decoder_video_->open(video_stream);

	return true;
}


bool GPX2VideoStream::play(void) {
	if (!decoder_video_)
		return false;

	if (!thread_) {
		thread_ = new std::thread([this] {
			run();
		});
	}

	return true;
}


//bool GPX2VideoStream::pause(void) {
//	return true;
//}
//
//
//bool GPX2VideoStream::stop(void) {
//	return true;
//}


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


FramePtr GPX2VideoStream::getFrame(void) {
	log_call();

	if (queue_.empty())
		return NULL;

	std::lock_guard<std::mutex> lock(mutex_);

	FramePtr frame = queue_.front();

	queue_.pop_front();

	return frame;
}


bool GPX2VideoStream::read(void) {
	log_call();

	AVRational video_time;

	// Retrieve audio & video streams
	VideoStreamPtr video_stream = container_->getVideoStream();

	// Read video data
	video_time = av_div_q(av_make_q(1000 * frame_time_, 1), video_stream->frameRate());

	frame_ = decoder_video_->retrieveVideo(video_time);

	if (!frame_)
		return false;

	// next frame
	frame_time_ += 1;

	// Lock mutex
	std::lock_guard<std::mutex> lock(mutex_);

	queue_.push_back(frame_);

	return true;
}


void GPX2VideoStream::run(void) {
	log_call();

	for (;;) {
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

		if (queue_.size() > 5) {
			usleep(10);
			continue;
		}

		if (read() == false)
			break;
	}
}


//void GPX2VideoStream::on_data_ready(void) {
////	log_call();
//
//	std::lock_guard<std::mutex> lock(mutex_);
//
////	while (!queue_.empty())
////		queue_.pop_front();
//
//	queue_.push_back(frame_);
//}




GLuint vao_ = 0;
GLuint vbo_ = 0;
GLuint ebo_ = 0;
GLuint texture_ = 0;


float vertices[] = {
	// points             // texture coords
	1.0f, 1.0f, 0.0f,     1.0f, 0.0f,   // top right
	1.0f, -1.0f, 0.0f,    1.0f, 1.0f,   // bottom right
	-1.0f, -1.0f, 0.0f,   0.0f, 1.0f,   // bottom left
	-1.0f, 1.0f, 0.0f,    0.0f, 0.0f    // top left
};

unsigned int indices[] = {
	0, 1, 3,
	1, 2, 3
};


GPX2VideoArea::Shader::Shader(const GLchar *vertex_path, const GLchar *fragment_path) {
	int status;

	id_ = 0;

	auto vshader_bytes = Gio::Resource::lookup_data_global(vertex_path);
	if (!vshader_bytes) {
		std::cerr << "Failed fetching vertex shader resource" << std::endl;
		return;
	}

	gsize vshader_size {vshader_bytes->get_size()};

	auto vertex = build(GL_VERTEX_SHADER,
			(const char *) vshader_bytes->get_data(vshader_size));

	if (vertex == 0) {
		return;
	}

	auto fshader_bytes = Gio::Resource::lookup_data_global(fragment_path);

	if (!fshader_bytes) {
		std::cerr << "Failed fetching fragment shader resource" << std::endl;
		glDeleteShader(vertex);
		return;
	}

	gsize fshader_size {fshader_bytes->get_size()};

	auto fragment = build(GL_FRAGMENT_SHADER,
			(const char * ) fshader_bytes->get_data(fshader_size));

	if (fragment == 0) {
		glDeleteShader(vertex);
		return;
	}

	// create shader program
	id_ = glCreateProgram();
	glAttachShader(id_, vertex);
	glAttachShader(id_, fragment);

	glLinkProgram(id_);

	glGetProgramiv(id_, GL_LINK_STATUS, &status);

	if (status == GL_FALSE) {
		int log_len;
		glGetProgramiv(id_, GL_INFO_LOG_LENGTH, &log_len);

		std::string log_space(log_len+1, ' ');
		glGetProgramInfoLog(id_, log_len, nullptr, (GLchar*) log_space.c_str());

		std::cerr << "Linking failure: " << log_space << std::endl;

		glDeleteProgram(id_);
		id_ = 0;
	}
	else {
		log_info("Shader built with success");

		glDetachShader(id_, vertex);
		glDetachShader(id_, fragment);
	}

	glDeleteShader(vertex);
	glDeleteShader(fragment);
}


GPX2VideoArea::Shader::~Shader() {
	glDeleteProgram(id_);
}


GPX2VideoArea::Shader * GPX2VideoArea::Shader::create(const std::string &vertex_path, const std::string &fragment_path) {
	GPX2VideoArea::Shader *shader = new GPX2VideoArea::Shader(vertex_path.c_str(), fragment_path.c_str());

	return shader;
}


GLuint GPX2VideoArea::Shader::build(int type, const char *src) {
	int status;

	// shader compilation
	auto shader = glCreateShader(type);
	glShaderSource(shader, 1, &src, nullptr);
	glCompileShader(shader);

	// dump errors
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if(status == GL_FALSE) {
		int log_len;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_len);

		std::string log_space(log_len+1, ' ');
		glGetShaderInfoLog(shader, log_len, nullptr, (GLchar*)log_space.c_str());

		std::cerr << "Compile failure in " <<
			(type == GL_VERTEX_SHADER ? "vertex" : "fragment") <<
			" shader: " << log_space << std::endl;

		glDeleteShader(shader);

		return 0;
	}

	return shader;
}



void GPX2VideoArea::Shader::use(void) {
	glUseProgram(id_);
}


void GPX2VideoArea::Shader::set(const std::string &name, bool value) const {
	glUniform1i(glGetUniformLocation(id_, name.c_str()), (int) value);
}


void GPX2VideoArea::Shader::set(const std::string &name, int value) const {
	glUniform1i(glGetUniformLocation(id_, name.c_str()), value);
}


void GPX2VideoArea::Shader::set(const std::string &name, float value) const {
	glUniform1f(glGetUniformLocation(id_, name.c_str()), value);
}



GPX2VideoArea::GPX2VideoArea()
	: Glib::ObjectBase("GPX2VideoArea") {
	log_call();
}


GPX2VideoArea::GPX2VideoArea(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder>& ref_builder) 
	: Glib::ObjectBase("GPX2VideoArea")
	, Gtk::GLArea(cobject)
	, ref_builder_(ref_builder)
	, shader_(NULL) {
	log_call();

#if GTKMM_CHECK_VERSION(4, 12, 0)
	set_allowed_apis(Gdk::GLApi::GL);
#else
	set_use_es(false);
#endif
	set_expand(true);
	set_size_request(320, 240);
	set_auto_render(true);

	// Creation of a new object prevents long lines and shows us a little
	// how slots work.  We have 0 parameters and bool as a return value
	// after calling sigc::bind.
	refresh_slot_ = sigc::bind(sigc::mem_fun(*this, &GPX2VideoArea::on_timeout));

	// Connect signals
	signal_realize().connect(sigc::mem_fun(*this, &GPX2VideoArea::on_realize));
	signal_unrealize().connect(sigc::mem_fun(*this, &GPX2VideoArea::on_unrealize), false);
	signal_render().connect(sigc::mem_fun(*this, &GPX2VideoArea::on_render), false);
	signal_resize().connect(sigc::mem_fun(*this, &GPX2VideoArea::on_resize), false);
}


GPX2VideoArea::~GPX2VideoArea() {
	log_call();
}


void GPX2VideoArea::open_stream(const Glib::ustring &video_file) {
	log_call();

	stream_.open(video_file);
	stream_.play();
}


void GPX2VideoArea::play(void) {
	log_call();

	schedule_refresh(100);
}


void GPX2VideoArea::on_realize(void) {
	log_call();

	Gtk::GLArea::on_realize();

	// OpenGL context init
	make_current();

	try {
		throw_if_error();

		const bool use_es = get_context()->get_use_es();

		const std::string vertex_path = use_es ? "/com/progweb/gpx2video/gl/glarea-gles.vs.glsl" : "/com/progweb/gpx2video/gl/glarea-gl.vs.glsl";
		const std::string fragment_path = use_es ? "/com/progweb/gpx2video/gl/glarea-gles.fs.glsl" : "/com/progweb/gpx2video/gl/glarea-gl.fs.glsl";

		log_info("%s", use_es ? "Use OpenGL ES" : "Use OpenGL");

		shader_ = GPX2VideoArea::Shader::create(vertex_path, fragment_path);

		init_buffers();
	}
	catch(const Gdk::GLError &gle) {
		std::cerr << "An error occured making the context current during realize:" << std::endl;
		std::cerr << gle.domain() << "-" << gle.code() << "-" << gle.what() << std::endl;
	}
}


void GPX2VideoArea::on_unrealize(void) {
	log_call();

//	make_current();

	try {
		throw_if_error();

		// Delete buffers and program
		glDeleteBuffers(1, &ebo_);
		glDeleteBuffers(1, &vbo_);
		glDeleteBuffers(1, &vao_);

		if (shader_)
			delete shader_;
	}
	catch(const Gdk::GLError &gle) {
		std::cerr << "An error occured making the context current during unrealize" << std::endl;
		std::cerr << gle.domain() << "-" << gle.code() << "-" << gle.what() << std::endl;
	}

	Gtk::GLArea::on_unrealize();
}


bool GPX2VideoArea::on_render(const Glib::RefPtr<Gdk::GLContext> &context) {
	log_call();

	(void) context;

	if (!texture_)
		return true;

	try {
		throw_if_error();

		glClearColor(0.0, 0.0, 0.3, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);

		// 
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_);

		//
		shader_->use();

		//
		glBindVertexArray(vao_);
//		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

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

	float desiredAspectRatio = 1.0;

	int widthOfViewport = 1.0, heightOfViewport = 1.0;
	int lowerLeftCornerOfViewportX = 0, lowerLeftCornerOfViewportY = 0;

	float requiredWidthOfViewport, requiredHeightOfViewport;

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


/**
 * AV sync correction is done if the clock difference is above the maximum AV sync threshold.
 */
#define AV_SYNC_THRESHOLD 0.01

bool GPX2VideoArea::on_timeout(void) {
	log_call();

	double pts_delay;
	double real_delay;
	double sync_threshold;

	static double frame_timer_ = (double) av_gettime() / 1000000.0;
	static double frame_last_pts_ = 0;
	static double frame_last_delay_ = 40e-3;

	(void) sync_threshold;

	// Get frame
	FramePtr frame = stream_.getFrame();

	if (!frame)
		return true;

	double pts = frame->timestamp() * av_q2d(frame->videoParams().timeBase());

//	// 
//	log_info("Current Frame PTS:\t\t%f", pts);
//	log_info("Last Frame PTS:\t\t\t%f", frame_last_pts_);

	// get last frame pts
	pts_delay = pts - frame_last_pts_;

//	log_info("PTS Delay:\t\t\t%f", pts_delay); 

	// if the obtained delay is incorrect
	if (pts_delay <= 0 || pts_delay >= 1.0) {
		// use the previously calculated delay
		pts_delay = frame_last_delay_;
	}

//	log_info("Corrected PTS Delay:\t\t%f", pts_delay);

	// save delay information for the next time
	frame_last_delay_ = pts_delay;
	frame_last_pts_ = pts;

	// skip or repeat the frame taking into account the delay
	sync_threshold = (pts_delay > AV_SYNC_THRESHOLD) ? pts_delay : AV_SYNC_THRESHOLD;

//	log_info("Sync Threshold:\t\t\t%f", sync_threshold);

	frame_timer_ += pts_delay;

	// compute the real delay
	real_delay = frame_timer_ - (av_gettime() / 1000000.0);

//	log_info("Real Delay:\t\t\t%f", real_delay);

	if (real_delay < 0.010)
		real_delay = 0.010;

//	log_info("Corrected Real Delay:\t\t%f", real_delay);

	real_delay *= 1000 + 0.5;

//	log_info("Next Scheduled Refresh:\t\t%f\n", real_delay);

	// Schedule next frame
	schedule_refresh((int) real_delay);

	// Create & load OpenGL texture
	load_texture(frame);

	// OpenGL context redraw
	queue_draw();

	return false; //true;
}


void GPX2VideoArea::schedule_refresh(int delay) {
	log_call();

	// This is where we connect the slot to the Glib::signal_timeout()
	timer_ = Glib::signal_timeout().connect(refresh_slot_, delay);
}


void GPX2VideoArea::init_buffers(void) {
	log_call();

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

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) 0);
	glEnableVertexAttribArray(0);
	// texture attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float)));
	glEnableVertexAttribArray(1);
}


void GPX2VideoArea::load_texture(FramePtr frame) {
	log_call();

	if (!frame)
		return;

//	log_info("width: %d x height: %d - linesize: %d", 
//			frame->width(), frame->height(), frame->linesizeBytes());

	// texture
	if (!texture_) {
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
				0, GL_RGBA, GL_UNSIGNED_BYTE, frame->constData());
	}
	else {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_);
		glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
		glPixelStorei(GL_UNPACK_LSB_FIRST, GL_TRUE);
		glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
		glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, frame->linesizeBytes() / 4);
		glTexSubImage2D(GL_TEXTURE_2D, 
				0,
				0, 0, 
				frame->width(),
				frame->height(),
				GL_RGBA, GL_UNSIGNED_BYTE, frame->constData());
	}
//	glGenerateMipmap(GL_TEXTURE_2D);
}

