#ifndef __GPX2VIDEO__RENDERER_H__
#define __GPX2VIDEO__RENDERER_H__

#include <string>
#include <vector>

#include <OpenImageIO/imageio.h>
#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>

#include "layoutlib/Map.h"
#include "layoutlib/Track.h"
#include "layoutlib/Widget.h"

#include "gpx.h"
#include "map.h"
#include "track.h"
#include "frame.h"
#include "media.h"
#include "decoder.h"
#include "encoder.h"
#include "videowidget.h"
#include "gpx2video.h"


class Renderer : public GPX2Video::Task {
public:
	virtual ~Renderer();

	void append(VideoWidget *widget);

protected:
	GPX2Video &app_;

	GPX *gpx_;
	GPXData data_;

	MediaContainer *container_;

	std::list<VideoWidget *> widgets_;

	int layout_width_;
	int layout_height_;

	OIIO::ImageBuf *overlay_;

	Renderer(GPX2Video &app); //, Map *map);

	bool init(void);
	bool load(void);
	bool loadMap(layout::Map *m);
	bool loadTrack(layout::Track *t);
	bool loadWidget(layout::Widget *w);
	void computeWidgetsPosition(void);

	void rotate(OIIO::ImageBuf *buf, int orientation);
	void resize(OIIO::ImageBuf *buf, int width, int height);
	void add(OIIO::ImageBuf *frame, int x, int y, const char *picto, const char *label, const char *value, double divider=1.9);
};

#endif

