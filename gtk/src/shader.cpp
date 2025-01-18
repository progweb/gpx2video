#include <iostream>
#include <string>

#include <gtkmm/builder.h>
#include <epoxy/gl.h>
#undef GLAPI

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "log.h"
#include "shader.h"


GPX2VideoShader::GPX2VideoShader(const GLchar *vertex_path, const GLchar *fragment_path) {
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


GPX2VideoShader::~GPX2VideoShader() {
	glDeleteProgram(id_);
}


GPX2VideoShader * GPX2VideoShader::create(const std::string &vertex_path, const std::string &fragment_path) {
	GPX2VideoShader *shader = new GPX2VideoShader(vertex_path.c_str(), fragment_path.c_str());

	return shader;
}


GLuint GPX2VideoShader::build(int type, const char *src) {
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



void GPX2VideoShader::use(void) {
	glUseProgram(id_);
}


void GPX2VideoShader::set(const std::string &name, bool value) const {
	glUniform1i(glGetUniformLocation(id_, name.c_str()), (int) value);
}


void GPX2VideoShader::set(const std::string &name, int value) const {
	glUniform1i(glGetUniformLocation(id_, name.c_str()), value);
}


void GPX2VideoShader::set(const std::string &name, float value) const {
	glUniform1f(glGetUniformLocation(id_, name.c_str()), value);
}


void GPX2VideoShader::set(const std::string &name, glm::mat4 &value) const {
	glUniformMatrix4fv(glGetUniformLocation(id_, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}

