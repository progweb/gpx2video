#ifndef __GPX2VIDEO__TRACK_H__
#define __GPX2VIDEO__TRACK_H__

#include <iostream>
#include <memory>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <list>

#include <stdlib.h>

#include <OpenImageIO/imageio.h>
#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>

#include "log.h"
#include "gpx.h"
#include "tracksettings.h"
#include "videowidget.h"
#include "gpx2video.h"


class Track : public VideoWidget {
public:
	virtual ~Track();

	static Track * create(GPX2Video &app, const TrackSettings& settings);

	const TrackSettings& settings() const;

	void setSize(int width, int height);

	static int lat2pixel(int zoom, float lat);
	static int lon2pixel(int zoom, float lon);

	// Draw track path
	void path(OIIO::ImageBuf &outbuf, GPX *gpx, double divider=1.0);

	// Render track
	void prepare(OIIO::ImageBuf *buf);
	void render(OIIO::ImageBuf *frame, const GPXData &data);

protected:
	OIIO::ImageBuf *buf_;

	Track(GPX2Video &app, const TrackSettings &settings, struct event_base *evbase);

	void init(bool zoomfit=true);
	bool load(void);

	bool drawPicto(OIIO::ImageBuf &map, int x, int y, OIIO::ROI roi, const char *picto, int size);

	GPX2Video &app_;
	TrackSettings settings_;

	struct event_base *evbase_;

	OIIO::ImageBuf *trackbuf_;

	double divider_;

	// Bounding box
	int x1_, y1_, x2_, y2_;
	int px1_, py1_, px2_, py2_;

	// Start & end position
	int x_end_, y_end_;
	int x_start_, y_start_;
};

#endif
