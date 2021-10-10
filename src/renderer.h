#ifndef __GPX2VIDEO__RENDERER_H__
#define __GPX2VIDEO__RENDERER_H__

#include <string>
#include <vector>

#include <OpenImageIO/imageio.h>
#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>

#include "gpx.h"
#include "frame.h"


class Renderer {
public:
	virtual ~Renderer();

	static Renderer * create(void);

	void draw(FramePtr frame, const GPXData &data);

private:
	Renderer();

	void add(OIIO::ImageBuf *frame, int x, int y, const char *picto, const char *label, const char *value, double divider);
};

#endif

