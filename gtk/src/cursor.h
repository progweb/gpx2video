#ifndef __GPX2VIDEO__GTK__CURSOR_H__
#define __GPX2VIDEO__GTK__CURSOR_H__

#include "shader.h"


class GPX2VideoCursor {
public:
	static GPX2VideoCursor * create(void);
	virtual ~GPX2VideoCursor();

	const bool& visible(void) const;

	void set_visible(bool visible);
	void set_position(const double &x, const double &y);

	void init_buffers(void);
	void render(GPX2VideoShader *shader);

protected:
	GPX2VideoCursor();

private:
	bool visible_;

	double x_;
	double y_;

	GLuint vao_ = 0;
	GLuint vbo_ = 0;
	GLuint ebo_ = 0;
	GLuint pbo_ = 0;
};

#endif

