#include <span>
#include <vector>

#include <epoxy/gl.h>
#undef GLAPI

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "log_i.h"
#include "datetime.h"
#include "videowidget.h"


//#define USE_GL_SURFACE
//#define WIDGET_DEBUG


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
	, mutex_()
	, queue_mutex_() {
	log_call();

	is_update_ = false;
	is_buffer_init_ = false;

	timestamp_ = 0;

	// Layout size
	layout_width_ = 0;
	layout_height_ = 0;

	// Init
	index_ = 0;
	queue_size_ = !widget->isStatic() ? 2 : 1;

	buffer_ = NULL;
	overlay_ = NULL;

	clear_req_ = false;

	// Texture
	texture_ = 0;

	// Stats
	stats_texture_reused_ = 0;
	stats_texture_delayed_ = 0;
	stats_texture_dropped_ = 0;
	stats_texture_updated_ = 0;
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


ShapeBase * GPX2VideoWidget::shape(void) {
	log_call();

	return widget_->shape();
}


VideoWidget * GPX2VideoWidget::widget(void) {
	log_call();

	return widget_;
}


bool GPX2VideoWidget::is_over(const double &x, const double &y) {
	log_call();

	double width = glWidth(); // / 2.0;
	double height = glHeight(); // / 2.0;

	if ((x < (glX() - width)) || (x > (glX() + width)))
		return false;

	if ((y < (glY() - height)) || (y > (glY() + height)))
		return false;

	return true;
}


GPX2VideoWidget::type_signal_changed GPX2VideoWidget::signal_changed() {
	return signal_changed_;
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


bool GPX2VideoWidget::ready(void) {
	return !clear_req_;
}


/**
 * Switch to next frame
 * (Called from video thread)
 */
void GPX2VideoWidget::set_timestamp(uint64_t timestamp) {
	log_call();

	int dropped = -1;

	GPX2VideoWidget::BufferPtr next;

	std::lock_guard<std::mutex> lock(queue_mutex_);

	// Save current timestamp
	timestamp_ = timestamp;

	// Update widget for profiling
	widget_->setTimestamp(timestamp);

	// Drop old buffers
	while (queue_.size() > 1) {
		next = queue_[1];

		if (next->timestamp() > timestamp)
			break;

		queue_.pop_front();

		dropped++;

		is_update_ = true;
	}

	// Stats
	// dropped > 1, as texture hasn't been used
	if (dropped > 0)
		stats_texture_dropped_ += dropped;
}


/**
 * draw widget
 */
#ifdef USE_GL_SURFACE
bool GPX2VideoWidget::draw(const TelemetryData &data) {
	log_call();

	int index;

	bool is_update;

	GPX2VideoWidget::BufferPtr buffer;

	// Lock widget settings
	std::lock_guard<std::mutex> lock(mutex_);

	// Draw changes
	is_update = widget()->updated(data);

#ifdef WIDGET_DEBUG
printf("WIDGET::DRAW %s - updated: %s\n", 
		widget()->name().c_str(),
		is_update ? "true" : "false");
#endif

	if (is_update) {
		// Buffer index
		index = index_;
		index_ = (index_ + 1) % queue_size_;

		// Create new buffer
		buffer = GPX2VideoWidget::Buffer::create();

		buffer->setTimestamp(data.timestamp());
		buffer->setData(index, buffer_[index]);

		// Clear data
		memset(reinterpret_cast<unsigned char *>(buffer->data()), 0x000000, 
				theme().width() * theme().height() * 4);

		// Create cairo context
		int stride = Cairo::ImageSurface::format_stride_for_width(Cairo::Surface::Format::ARGB32, theme().width());

		Cairo::RefPtr<Cairo::ImageSurface> surface = Cairo::ImageSurface::create(
				reinterpret_cast<unsigned char *>(buffer->data()), Cairo::Surface::Format::ARGB32, theme().width(), theme().height(), stride);

		Cairo::RefPtr<Cairo::Context> cr = Cairo::Context::create(surface);

		// Draw
		widget_->draw(cr->cobj(), data);

		// Save buffer
		std::lock_guard<std::mutex> lock(queue_mutex_);

		queue_.push_back(buffer);

		// Stats
		stats_texture_updated_++;
	}
	else {
		// Update buffer timestamp
		std::lock_guard<std::mutex> lock(queue_mutex_);

		buffer = queue_.back();
		buffer->setTimestamp(data.timestamp());
		queue_.back() = buffer;

		// Stats
		stats_texture_reused_++;
	}

	return is_update;
}
#else
bool GPX2VideoWidget::draw(const TelemetryData &data) {
	log_call();

	int index;

	bool is_update;
	bool is_bg_update, is_fg_update;

	GPX2VideoWidget::BufferPtr buffer;

#ifdef WIDGET_DEBUG
printf("WIDGET::DRAW %s\n", widget()->name().c_str());
#endif

	// Lock widget settings
	std::lock_guard<std::mutex> lock(mutex_);

	OIIO::ImageBuf *bg_buf = widget_->prepare(is_bg_update);
	OIIO::ImageBuf *fg_buf = widget_->render(data, is_fg_update);

	is_update = (is_bg_update || is_fg_update);

#ifdef WIDGET_DEBUG
printf("WIDGET::DRAW %s - updated: %s\n", 
		widget()->name().c_str(),
		is_update ? "true" : "false");
#endif

	if (is_update) {
		// Buffer index
		index = index_;
		index_ = (index_ + 1) % queue_size_;

		// Create new buffer
		buffer = GPX2VideoWidget::Buffer::create();

		buffer->setTimestamp(data.timestamp());
		buffer->setData(index, buffer_[index]);

		// Swap fg & bg ?
		if (!bg_buf) {
			bg_buf = fg_buf;
			fg_buf = NULL;
		}

		// Draw bg
		if (bg_buf)
			OIIO::ImageBufAlgo::copy(*overlay_, *bg_buf, bg_buf->spec().format, bg_buf->roi(), 4);

		// Draw fg
		if (fg_buf)
			OIIO::ImageBufAlgo::over(*overlay_, *fg_buf, *overlay_, fg_buf->roi(), 4);

		// Update buffer
		overlay_->get_pixels(OIIO::ROI(), 
				overlay_->spec().format, 
				reinterpret_cast<char*>(buffer->data()));

		// Save buffer
		std::lock_guard<std::mutex> lock(queue_mutex_);

		queue_.push_back(buffer);

		// Stats
		stats_texture_updated_++;
	}
	else {
		// Update buffer timestamp
		std::lock_guard<std::mutex> lock(queue_mutex_);

		buffer = queue_.back();
		buffer->setTimestamp(data.timestamp());
		queue_.back() = buffer;

		// Stats
		stats_texture_reused_++;
	}

	return is_update;
}
#endif


/**
 * Clear widget cache
 * (Called from main thread)
 */
void GPX2VideoWidget::clear(void) {
	log_call();

	// Clear buffer requested
	clear_req_ = true;
}


void GPX2VideoWidget::texture_delayed(void) {
	log_call();

	stats_texture_delayed_++;
}

void GPX2VideoWidget::stats(void) {
	log_call();

	const size_t max = 16;

	size_t len = widget()->name().length();

	log_info("Widget %s stats: %*s %4d reused, %4d delayed, %4d dropped, %4d updated", 
			widget()->name().c_str(),
			(int) (max - len), "",
			stats_texture_reused_,
			stats_texture_delayed_,
			stats_texture_dropped_,
			stats_texture_updated_);
}


void GPX2VideoWidget::init_buffers(void) {
	log_call();

	size_t size;

	int nchannels = 4;

#ifdef WIDGET_DEBUG
printf("WIDGET::INIT BUFFERS %s\n", widget()->name().c_str());
#endif

	// Buffer
	buffer_ = (uint8_t **) malloc(queue_size_ * sizeof(uint8_t *));

	// Widgets overlay
	overlay_ = new OIIO::ImageBuf(OIIO::ImageSpec(widget_->theme().width(), widget_->theme().height(), 
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

#ifdef WIDGET_DEBUG
printf("WIDGET::RESIZE BUFFERS %s\n", widget()->name().c_str());
#endif

	// Old texture not valid
	unload_texture();

	// Widgets overlay
	overlay_->reset(OIIO::ImageSpec(widget_->theme().width(), widget_->theme().height(), 
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
void GPX2VideoWidget::write_buffers(const TelemetryData &data, bool &loop) {
	log_call();

	bool clear_req = clear_req_;

#ifdef WIDGET_DEBUG
printf("WIDGET::WRITE BUFFERS %s (ts: %ld)\n", widget()->name().c_str(), data.timestamp());
#endif

	// By default continue to buffering
	loop = true;

	if (clear_req_) {
		// Clear & free texture
		clear_buffers();

		// Done
		clear_req_ = false;
	}

	if (!is_buffer_init_)
		return;

	if ((data.type() != TelemetryData::TypeUnknown) || queue_.empty()) {
		// Save data
		data_ = data;

		// Draw
		draw(data);

		if (clear_req)
			is_update_ = true;
	}
	else {
		// Stop buffering
		loop = false;
	}
}


void GPX2VideoWidget::clear_buffers(void) {
	log_call();

#ifdef WIDGET_DEBUG
printf("WIDGET::CLEAR BUFFERS %s\n", widget()->name().c_str());
#endif

	std::lock_guard<std::mutex> lock(queue_mutex_);

	log_info("Clear widget '%s' cache", widget()->name().c_str());

	is_update_ = false;

	// Drop buffers
	while (!queue_.empty())
		queue_.pop_front();

	// Stats
	stats_texture_reused_ = 0;
	stats_texture_delayed_ = 0;
	stats_texture_dropped_ = 0;
	stats_texture_updated_ = 0;

	// Reset index
	index_ = 0;

	// Reset
	widget_->clear();

	// Clear previous data
	data_ = TelemetryData();
}


/**
 * Load new texture
 * (Called from video thread)
 */
uint64_t GPX2VideoWidget::load_texture(void) {
	log_call();

#ifdef WIDGET_DEBUG
printf("WIDGET::LOAD TEXTURE %s - updated: %s\n", 
		widget()->name().c_str(),
		is_update_ ? "true" : "false");
#endif

	GPX2VideoWidget::BufferPtr buffer;

	std::lock_guard<std::mutex> lock(queue_mutex_);

	if (queue_.empty())
		return 0;

	buffer = queue_.front();

	if (texture_ && !is_update_)
		return !widget()->isStatic() ? buffer->timestamp() : 0;

#ifdef WIDGET_DEBUG
printf("WIDGET::LOADING TEXTURE...\n");
#endif

	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_[buffer->index()]);

	// texture
	if (!texture_) {
		glGenTextures(1, &texture_);
		glBindTexture(GL_TEXTURE_2D, texture_);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); //GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); //GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //GL_LINEAR_MIPMAP_LINEAR); //GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0); //frame->linesizeBytes() / 4);
#ifdef USE_GL_SURFACE
		glTexImage2D(GL_TEXTURE_2D, 
				0, 
				GL_RGBA8,
				overlay_->spec().width,
				overlay_->spec().height,
				0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);
#else
		glTexImage2D(GL_TEXTURE_2D, 
				0, 
				GL_RGBA, 
				overlay_->spec().width,
				overlay_->spec().height,
				0, GL_RGBA, GL_UNSIGNED_BYTE, NULL); //m_tex_buffer.data());
#endif
	}
	else {
		glBindTexture(GL_TEXTURE_2D, texture_);
		glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
		glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#ifdef USE_GL_SURFACE
		glTexSubImage2D(GL_TEXTURE_2D, 
				0, 
				0, 0,
				overlay_->spec().width,
				overlay_->spec().height,
				GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);
#else
		glTexSubImage2D(GL_TEXTURE_2D, 
				0, 
				0, 0,
				overlay_->spec().width,
				overlay_->spec().height,
				GL_RGBA, GL_UNSIGNED_BYTE, NULL); //m_tex_buffer.data());
#endif
	}

	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

	// Texture updated
	is_update_ = false;

	return !widget()->isStatic() ? buffer->timestamp() : 0;
}


void GPX2VideoWidget::unload_texture(void) {
	log_call();

	std::lock_guard<std::mutex> lock(queue_mutex_);

	if (texture_)
		glDeleteTextures(1, &texture_);
	texture_ = 0;
}


void GPX2VideoWidget::move(double dx, double dy) {
	log_call();

	int x, y;

	if (widget_->position() != VideoWidget::PositionNone)
		return;

	dx = dx / 2.0;
	dy = dy / 2.0;

	x = widget_->x() + (dx * layout_width_);
	y = widget_->y() - (dy * layout_height_);

	x = std::max(0, x);
	x = std::min(x, layout_width_ - widget_->theme().width());

	y = std::max(0, y);
	y = std::min(y, layout_height_ - widget_->theme().height());

	widget_->setPosition(x, y);
}


double GPX2VideoWidget::glX(void) const {
	log_call();

	double x = widget_->x() + (widget_->theme().width() / 2.0) - (layout_width_ / 2.0);

	return x * 2.0 / layout_width_;
}


double GPX2VideoWidget::glY(void) const {
	log_call();

	double y = widget_->y() + (widget_->theme().height() / 2.0) - (layout_height_ / 2.0);

	return -(y * 2.0 / layout_height_);
}


double GPX2VideoWidget::glWidth(void) const {
	log_call();

	return widget_->theme().width() * 2.0 / layout_width_;
}


double GPX2VideoWidget::glHeight(void) const {
	log_call();

	return widget_->theme().height() * 2.0 / layout_height_;
}


GPX2VideoWidget::BufferPtr GPX2VideoWidget::get_last_buffer(void) {
	log_call();

	std::lock_guard<std::mutex> lock(queue_mutex_);

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

#ifdef WIDGET_DEBUG
printf("WIDGET::RENDER %s\n", widget()->name().c_str());
#endif

	std::lock_guard<std::mutex> lock(queue_mutex_);

	if (!texture_) {
		log_warn("Widget %s can't rendered, no texture", widget()->name().c_str());
		return;
	}

#ifdef WIDGET_DEBUG
printf("WIDGET::RENDERING...\n");
#endif

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

