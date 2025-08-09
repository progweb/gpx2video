#include <span>
#include <vector>

#include <epoxy/gl.h>
#undef GLAPI

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "log.h"
#include "datetime.h"
#include "videowidget.h"


static float vertices[] = {
	// points             // texture coords
	0.5f, 0.5f, 0.0f,     1.0f, 0.0f,   // top right
	0.5f, -0.5f, 0.0f,    1.0f, 1.0f,   // bottom right
	-0.5f, -0.5f, 0.0f,   0.0f, 1.0f,   // bottom left
	-0.5f, 0.5f, 0.0f,    0.0f, 0.0f    // top left
};

static unsigned int indices[] = {
	0, 1, 3,
	1, 2, 3
};


/**
 * Widget buffer
 */

GPX2VideoWidget::Buffer::Buffer(void) 
	: data_(NULL) {
}


GPX2VideoWidget::Buffer::~Buffer(void) {
}


GPX2VideoWidget::BufferPtr GPX2VideoWidget::Buffer::create(void) {
	GPX2VideoWidget::BufferPtr buffer;

	buffer = std::make_shared<GPX2VideoWidget::Buffer>();

	return buffer;
}


const uint64_t& GPX2VideoWidget::Buffer::timestamp(void) const {
	return timestamp_;
}

// Save buffer

void GPX2VideoWidget::Buffer::setTimestamp(const uint64_t &timestamp) {
	timestamp_ = timestamp;
}


int GPX2VideoWidget::Buffer::index(void) {
	return index_;
}


uint8_t * GPX2VideoWidget::Buffer::data(void) {
	return data_;
}


void GPX2VideoWidget::Buffer::setData(int index, uint8_t *data) {
	index_ = index;
	data_ = data;
}


/**
 * Widget
 */

GPX2VideoWidget::GPX2VideoWidget(VideoWidget *widget) 
	: widget_(widget)
	, mutex_() {
	log_call();

	is_update_ = false;
	is_buffer_init_ = false;

	timestamp_ = 0;

	// Layout size
	layout_width_ = 0;
	layout_height_ = 0;

	// Init
	index_ = 0;
	queue_size_ = 2;

	buffer_ = NULL;
	overlay_ = NULL;

	clear_req_ = false;

	// Texture
	texture_ = 0;
}


GPX2VideoWidget::~GPX2VideoWidget() {
	log_call();

	clear_buffers();

	unload_texture();

	if (buffer_ != NULL)
		free(buffer_);

	if (overlay_ != NULL)
		delete overlay_;
}


VideoWidget * GPX2VideoWidget::widget(void) {
	log_call();

	return widget_;
}


GPX2VideoWidget * GPX2VideoWidget::create(VideoWidget *widget) {
	log_call();

	GPX2VideoWidget *component = new GPX2VideoWidget(widget);

	return component;
}


void GPX2VideoWidget::setSize(int width, int height) {
	log_call();

	is_buffer_init_ = false;

	widget()->setSize(width, height);
	widget()->initialize();

	resize_buffers();
}


void GPX2VideoWidget::setLayoutSize(int width, int height) {
	log_call();

	layout_width_ = width;
	layout_height_ = height;
}


bool GPX2VideoWidget::full(void) {
	if (clear_req_)
		return false;

	return (!is_buffer_init_ || (queue_.size() >= queue_size_));
}


/**
 * Switch to next frame
 * (Called from video thread)
 */
void GPX2VideoWidget::set_timestamp(uint64_t timestamp) {
	log_call();

	GPX2VideoWidget::BufferPtr next;

	std::lock_guard<std::mutex> lock(mutex_);

	// Save current timestamp
	timestamp_ = timestamp;

	// Drop old buffers
	while (queue_.size() > 1) {
		next = queue_[1];

		if (next->timestamp() > timestamp)
			break;

		queue_.pop_front();

		is_update_ = true;
	}
}


bool GPX2VideoWidget::draw(const TelemetryData &data) {
	log_call();

	int index;

	bool is_update;
	bool is_bg_update, is_fg_update;

	GPX2VideoWidget::BufferPtr buffer;

//printf("WIDGET::DRAW %s\n", widget()->name().c_str());

//	widget_->dump();

	OIIO::ImageBuf *bg_buf = widget_->prepare(is_bg_update);
	OIIO::ImageBuf *fg_buf = widget_->render(data, is_fg_update);

	is_update = (is_bg_update || is_fg_update);

//printf("WIDGET::DRAW %s - updated: %s\n", 
//		widget()->name().c_str(),
//		is_update ? "true" : "false");

	if (is_update) {
		// Buffer index
		index = index_;
		index_ = (index_ + 1) % queue_size_;

		// Create new buffer
		buffer = GPX2VideoWidget::Buffer::create();

		buffer->setTimestamp(data.timestamp());
		buffer->setData(index, buffer_[index]);

		// Draw bg
		OIIO::ImageBufAlgo::copy(*overlay_, *bg_buf, bg_buf->spec().format, bg_buf->roi(), 4);

		// Draw fg
		OIIO::ImageBufAlgo::over(*overlay_, *fg_buf, *overlay_, fg_buf->roi(), 4);

		// Update buffer
		overlay_->get_pixels(OIIO::ROI(), 
				overlay_->spec().format, 
				reinterpret_cast<char*>(buffer->data()));
		
		// Save buffer
		std::lock_guard<std::mutex> lock(mutex_);

		queue_.push_back(buffer);
	}

	return is_update;
}


/**
 * Clear widget cache
 * (Called from main thread)
 */
void GPX2VideoWidget::clear(void) {
	log_call();

	clear_req_ = true;
}


void GPX2VideoWidget::init_buffers(void) {
	log_call();

	size_t size;

	int nchannels = 4;

//printf("WIDGET::INIT BUFFERS %s\n", widget()->name().c_str());

	// Buffer
	buffer_ = (uint8_t **) malloc(queue_size_ * sizeof(uint8_t *));

	// Widgets overlay
	overlay_ = new OIIO::ImageBuf(OIIO::ImageSpec(widget_->width(), widget_->height(), 
		4, OIIO::TypeDesc::UINT8));
//		stream_.nbChannels(), OIIOUtils::getOIIOBaseTypeFromFormat(stream_.format())));

	// Buffer size
	size = overlay_->spec().width * overlay_->spec().height * nchannels
						* overlay_->spec().channel_bytes();

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

//	glGenBuffers(1, &pbo_);

	glGenBuffers(queue_size_, pbo_);
	for (size_t i=0; i<queue_size_; i++) {
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_[i]);

		GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;

		glBufferStorage(GL_PIXEL_UNPACK_BUFFER, size, 0, flags);

		buffer_[i] = (uint8_t *) glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, size, flags);

		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	}

	// position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) 0);
	// texture attribute
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float)));

	// Buffer ready
	is_buffer_init_ = true;

//	check_gl_error();
}


void GPX2VideoWidget::resize_buffers(void) {
	log_call();

	size_t size;

	int nchannels = 4;

//printf("WIDGET::RESIZE BUFFERS %s\n", widget()->name().c_str());

	// Old texture not valid
	unload_texture();

	// Widgets overlay
	overlay_->reset(OIIO::ImageSpec(widget_->width(), widget_->height(), 
		4, OIIO::TypeDesc::UINT8));

	// Buffer size
	size = overlay_->spec().width * overlay_->spec().height * nchannels
						* overlay_->spec().channel_bytes();

	glDeleteBuffers(queue_size_, pbo_);

	glGenBuffers(queue_size_, pbo_);
	for (size_t i=0; i<queue_size_; i++) {
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_[i]);

		GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;

		glBufferStorage(GL_PIXEL_UNPACK_BUFFER, size, 0, flags);

		buffer_[i] = (uint8_t *) glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, size, flags);

		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	}

	// Buffer ready
	is_buffer_init_ = true;

//	check_gl_error();
}


/**
 * Widget drawing
 * (Called from renderer thread)
 */
void GPX2VideoWidget::write_buffers(const TelemetryData &data) {
	log_call();

	bool clear_req = clear_req_;

//printf("WIDGET::WRITE BUFFERS %s\n", widget()->name().c_str());

	if (clear_req_) {
		// Clear & free texture
		clear_buffers();

		// Done
		clear_req_ = false;
	}

	if (!is_buffer_init_)
		return;

	if ((data.type() != TelemetryData::TypeUnknown) || queue_.empty()) {
		draw(data);

		is_update_ = clear_req;
	}
}


void GPX2VideoWidget::clear_buffers(void) {
	log_call();

//printf("WIDGET::CLEAR BUFFERS %s\n", widget()->name().c_str());

	is_update_ = false;

	std::lock_guard<std::mutex> lock(mutex_);

	log_info("Clear widget '%s' cache", widget()->name().c_str());

	// Drop buffers
	while (!queue_.empty())
		queue_.pop_front();

	// Reset index
	index_ = 0;

	// Reset
	widget_->clear();
}


/**
 * Load new texture
 * (Called from video thread)
 */
void GPX2VideoWidget::load_texture(void) {
	log_call();

//printf("WIDGET::LOAD TEXTURE %s\n", widget()->name().c_str());

	std::lock_guard<std::mutex> lock(mutex_);

	// 
//	int nchannels = 4;
//	static std::vector<unsigned char> m_tex_buffer;

	if (queue_.empty())
		return;

	if (texture_ && !is_update_)
		return;

//printf("WIDGET::LOADING TEXTURE...\n");

	GPX2VideoWidget::BufferPtr buffer = queue_.front();

//	size_t size = overlay_->spec().width * overlay_->spec().height * nchannels
//						* overlay_->spec().channel_bytes();

//	if (!texture_) {
//
//		m_tex_buffer.resize(size);
//	}
//
//	overlay_->get_pixels(OIIO::ROI(), 
//			overlay_->spec().format, 
//			reinterpret_cast<char*>(m_tex_buffer.data()));

//	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_);
//
//	glBufferData(GL_PIXEL_UNPACK_BUFFER, size, NULL, GL_STREAM_DRAW);
//
//	buffer = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
//
//	overlay_->get_pixels(OIIO::ROI(), 
//			overlay_->spec().format, 
//			reinterpret_cast<char*>(buffer));
//
//	glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_[buffer->index()]);

	// texture
	if (!texture_) {
		glGenTextures(1, &texture_);
//		glBindTexture(GL_UNPACK_ALIGNMENT, 1);
//		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture_);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); //GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); //GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //GL_LINEAR_MIPMAP_LINEAR); //GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

//		glPixelStorei(GL_UNPACK_ROW_LENGTH, frame->linesizeBytes() / 4);
//		glTexImage2D(GL_TEXTURE_2D, 
//				0, 
//				GL_RGBA, 
//				frame->width(),
//				frame->height(),
//				0, GL_RGBA, GL_UNSIGNED_BYTE, frame->constData());
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0); //frame->linesizeBytes() / 4);
		glTexImage2D(GL_TEXTURE_2D, 
				0, 
				GL_RGBA, 
				overlay_->spec().width,
				overlay_->spec().height,
				0, GL_RGBA, GL_UNSIGNED_BYTE, NULL); //m_tex_buffer.data());

//		shader_->use();
//		shader_->set("inputTexture2", 1);
	}
	else {
//		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture_);
//		glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
//		glPixelStorei(GL_UNPACK_LSB_FIRST, GL_TRUE);
		glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
		glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

//		glPixelStorei(GL_UNPACK_ROW_LENGTH, frame->linesizeBytes() / 4);
//		glTexSubImage2D(GL_TEXTURE_2D, 
//				0,
//				0, 0, 
//				frame->width(),
//				frame->height(),
//				GL_RGBA, GL_UNSIGNED_BYTE, frame->constData());
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glTexSubImage2D(GL_TEXTURE_2D, 
				0, 
				0, 0,
				overlay_->spec().width,
				overlay_->spec().height,
				GL_RGBA, GL_UNSIGNED_BYTE, NULL); //m_tex_buffer.data());
	}

	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
//	glBindTexture(GL_TEXTURE_2D, 0);

	// Texture updated
	is_update_ = false;
}


void GPX2VideoWidget::unload_texture(void) {
	log_call();

	std::lock_guard<std::mutex> lock(mutex_);

	if (texture_)
		glDeleteTextures(1, &texture_);
	texture_ = 0;
}


double GPX2VideoWidget::glX(void) const {
	log_call();

	double x = widget_->x() + (widget_->width() / 2.0) - (layout_width_ / 2.0);

	return x * 2.0 / layout_width_;
}


double GPX2VideoWidget::glY(void) const {
	log_call();

	double y = widget_->y() + (widget_->height() / 2.0) - (layout_height_ / 2.0);

	return -(y * 2.0 / layout_height_);
}


double GPX2VideoWidget::glWidth(void) const {
	log_call();

	return widget_->width() * 2.0 / layout_width_;
}


double GPX2VideoWidget::glHeight(void) const {
	log_call();

	return widget_->height() * 2.0 / layout_height_;
}


GPX2VideoWidget::BufferPtr GPX2VideoWidget::get_last_buffer(void) {
	log_call();

	std::lock_guard<std::mutex> lock(mutex_);

	if (!queue_.empty())
		return queue_.back();

	return NULL;
}


/**
 * Do OpenGL rendering
 * (Called from main thread)
 */
void GPX2VideoWidget::render(GPX2VideoShader *shader) {
	log_call();

//printf("WIDGET::RENDER %s\n", widget()->name().c_str());

	std::lock_guard<std::mutex> lock(mutex_);

	if (!texture_) {
		log_warn("Widget %s can't rendered, no texture", widget()->name().c_str());
		return;
	}

//printf("WIDGET::RENDERING...\n");

	// Transformations
	glm::mat4 transform = glm::mat4(1.0f);
	transform = glm::translate(transform, glm::vec3(glX(), glY(), 0.0f));
	transform = glm::scale(transform, glm::vec3(glWidth(), glHeight(), 0.0f));

	//
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_);

	//
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//
	shader->use();
	shader->set("transform", transform);

	//
	glBindVertexArray(vao_);
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	log_debug("DONE");
}


void GPX2VideoWidget::check_gl_error(void) {
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
