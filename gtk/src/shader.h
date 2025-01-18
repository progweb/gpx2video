#ifndef __GPX2VIDEO__GTK__SHADER_H__
#define __GPX2VIDEO__GTK__SHADER_H__

#include <gtkmm/glarea.h>
#include <epoxy/gl.h>
#undef GLAPI

#include <glm/glm.hpp>


class GPX2VideoShader {
public:
	virtual ~GPX2VideoShader();

	static GPX2VideoShader * create(const std::string &vertex_path, const std::string &fragment_path);

	void use(void);

	GLuint id(void) { return id_; }

	void set(const std::string &name, bool value) const;
	void set(const std::string &name, int value) const;
	void set(const std::string &name, float value) const;
	void set(const std::string &name, glm::mat4 &value) const;

protected:
	GLuint id_;

	GPX2VideoShader(const GLchar *vertex_path, const GLchar *fragment_path);

	GLuint build(int type, const char *src);
};

#endif

