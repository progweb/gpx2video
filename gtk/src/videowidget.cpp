#include <span>
#include <vector>

#include <epoxy/gl.h>
#undef GLAPI

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "log.h"
#include "videowidget.h"


static GLuint vao_ = 0;
static GLuint vbo_ = 0;
static GLuint ebo_ = 0;
static GLuint texture_;


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


GPX2VideoWidget::GPX2VideoWidget(VideoWidget *widget) 
	: widget_(widget) {
	is_update_ = false;

	// Layout size
	layout_width_ = 0;
	layout_height_ = 0;

	// Widgets overlay
	overlay_ = new OIIO::ImageBuf(OIIO::ImageSpec(widget->width(), widget->height(), 
		4, OIIO::TypeDesc::UINT8));
//		stream_.nbChannels(), OIIOUtils::getOIIOBaseTypeFromFormat(stream_.format())));

}


GPX2VideoWidget::~GPX2VideoWidget() {
}


GPX2VideoWidget * GPX2VideoWidget::create(VideoWidget *widget) {
	log_call();

	GPX2VideoWidget *component = new GPX2VideoWidget(widget);

	return component;
}


void GPX2VideoWidget::setLayoutSize(int width, int height) {
	log_call();

	layout_width_ = width;
	layout_height_ = height;
}


bool GPX2VideoWidget::draw(void) {
	log_call();

	TelemetryData data;

	bool is_bg_update, is_fg_update;

	OIIO::ImageBuf *bg_buf = widget_->prepare(is_bg_update);
	OIIO::ImageBuf *fg_buf = widget_->render(data, is_fg_update);

	is_update_ = (is_bg_update || is_fg_update);

	// Draw bg
//	bg_buf->specmod().x = widget_->x();
//	bg_buf->specmod().y = widget_->y();
	OIIO::ImageBufAlgo::copy(*overlay_, *bg_buf, bg_buf->spec().format, bg_buf->roi());

	// Draw fg
//	fg_buf->specmod().x = widget_->x();
//	fg_buf->specmod().y = widget_->y();
	OIIO::ImageBufAlgo::over(*overlay_, *fg_buf, *overlay_, fg_buf->roi());

	return is_update_;
}


void GPX2VideoWidget::init_buffers(void) {
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
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) 0);
	// texture attribute
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float)));
}


void GPX2VideoWidget::load_texture(void) {
	log_call();

	// 
	int nchannels = 4;
	static std::vector<unsigned char> m_tex_buffer;

	if (!texture_) {
		m_tex_buffer.resize(overlay_->spec().width * overlay_->spec().height * nchannels
							* overlay_->spec().channel_bytes());
	}

	overlay_->get_pixels(OIIO::ROI(), 
			overlay_->spec().format, 
			reinterpret_cast<char*>(m_tex_buffer.data()));

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
				0, GL_RGBA, GL_UNSIGNED_BYTE, m_tex_buffer.data());

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
				GL_RGBA, GL_UNSIGNED_BYTE, m_tex_buffer.data());
	}
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


void GPX2VideoWidget::render(GPX2VideoShader *shader) {
	log_call();

	// Transformations
	glm::mat4 transform = glm::mat4(1.0f);
//	transform = glm::translate(transform, glm::vec3(0.5f, -0.5f, 0.0f));
//	transform = glm::rotate(transform, (float) 30.0, glm::vec3(0.0f, 0.0f, 1.0f));
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

