#ifndef __GPX2VIDEO__RENDERER_H__
#define __GPX2VIDEO__RENDERER_H__

#include <string>
#include <vector>

#include "frame.h"


class Renderer {
public:
	virtual ~Renderer();

	static Renderer * create(void);

	void draw(FramePtr frame);

private:
	Renderer();
};

#endif

