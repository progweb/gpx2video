#ifndef __GPX2VIDEO__RENDERER_H__
#define __GPX2VIDEO__RENDERER_H__

#include <string>
#include <vector>

#include <OpenImageIO/imageio.h>
#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>

#include "layoutlib/Map.h"
#include "layoutlib/Widget.h"

#include "gpx.h"
#include "map.h"
#include "frame.h"
#include "media.h"
#include "decoder.h"
#include "encoder.h"
#include "videowidget.h"
#include "gpx2video.h"


class Renderer : public GPX2Video::Task {
public:
	virtual ~Renderer();

	static Renderer * create(GPX2Video &app); //, Map *map=NULL);

	void append(VideoWidget *widget);

	bool start(void);
	bool run(void);
	bool stop(void);

	void draw(FramePtr frame, const GPXData &data);

private:
	GPX2Video &app_;

	GPX *gpx_;
	GPXData data_;

	MediaContainer *container_;
	Decoder *decoder_audio_;
	Decoder *decoder_video_;
	Encoder *encoder_;

	std::list<VideoWidget *> widgets_;

	OIIO::ImageBuf *overlay_;

	time_t started_at_;

	char duration_[16];
	unsigned int duration_ms_;

	int64_t frame_time_ = 0;

	Renderer(GPX2Video &app); //, Map *map);

	void init(void);
	bool load(void);
	bool loadMap(layout::Map *m);
	bool loadWidget(layout::Widget *w);
	void computeWidgetsPosition(void);

	void add(OIIO::ImageBuf *frame, int x, int y, const char *picto, const char *label, const char *value, double divider=1.9);
};

#endif

