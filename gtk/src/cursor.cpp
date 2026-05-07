#include <epoxy/gl.h>
#undef GLAPI

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "log.h"
#include "cursor.h"


static float vertices[] = {
	// points             // color coords
	0.5f, 0.5f, 0.0f,     1.0f, 1.0f, 1.0f,   // top right
	0.5f, -0.5f, 0.0f,    1.0f, 1.0f, 1.0f,   // bottom right
	-0.5f, -0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   // bottom left
	-0.5f, 0.5f, 0.0f,    1.0f, 1.0f, 1.0f    // top left
}; 

static unsigned int indices[] = {
	0, 1, 3,
	1, 2, 3
};


GPX2VideoCursor::GPX2VideoCursor() 
	: visible_(false) {
	log_call();
}


GPX2VideoCursor::~GPX2VideoCursor() {
	log_call();
}


GPX2VideoCursor * GPX2VideoCursor::create() {
	log_call();

	GPX2VideoCursor *component = new GPX2VideoCursor();

	return component;
}


const bool& GPX2VideoCursor::visible(void) const {
	log_call();

	return visible_;
}


void GPX2VideoCursor::set_visible(bool visible) {
	log_call();

	visible_ = visible;
}


void GPX2VideoCursor::set_position(const double &x, const double &y) {
	log_call();

	x_ = x;
	y_ = y;

	if (x_ <= -1.0)
		x_ = -1.0;
	if (x_ >= 1.0)
		x_ = 1.0;

	if (y_ <= -1.0)
		y_ = -1.0;
	if (y_ >= 1.0)
		y_ = 1.0;
}


void GPX2VideoCursor::init_buffers(void) {
	log_call();

	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glGenVertexArrays(1, &vao_);
	glBindVertexArray(vao_);

	glGenBuffers(1, &vbo_);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
//  glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &ebo_);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
//  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) 0);
	glEnableVertexAttribArray(0);

	// color attribute
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) (3 * sizeof(float)));
}


void GPX2VideoCursor::render(GPX2VideoShader *shader) {
	log_call();

	glm::mat4 transform;

	if (!visible_)
		return;

	// x-axis
	//--------

	// transform
	transform = glm::mat4(1.0f);
	transform = glm::translate(transform, glm::vec3(x_, y_, 0.0f));
	transform = glm::scale(transform, glm::vec3(4.0f, 0.005f, 0.0f));
	
	// shader
	shader->use();
	shader->set("transform", transform);

	//
	glBindVertexArray(vao_);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	// y-axis
	//--------

	// transform
	transform = glm::mat4(1.0f);
	transform = glm::translate(transform, glm::vec3(x_, y_, 0.0f));
	transform = glm::scale(transform, glm::vec3(0.005f, 4.0f, 0.0f));
	
	// shader
	shader->use();
	shader->set("transform", transform);

	//
	glBindVertexArray(vao_);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

