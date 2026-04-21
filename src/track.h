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

#include "utils.h"
#include "tracksettings.h"
#include "videowidget.h"
#include "telemetrymedia.h"
#include "application.h"
#include "shape/base.h"


class Track : public VideoWidget, public ShapeBase {
public:
	virtual ~Track();

	static Track * create(GPXApplication &app, const TelemetrySettings& telemetry_settings, const TrackSettings& track_settings);

	TrackSettings& settings();
	const TrackSettings& settings() const;

	virtual bool isShapeSupported(Shape type) {
		(void) type;

		return false;
	}

	void setSize(int width, int height);

	bool start(void) {
		// Register task status
		VideoWidget::start();

		init();

		return true;
	}

	bool run(void) {
		complete();

		return true;
	}

	static int lat2pixel(int zoom, float lat);
	static int lon2pixel(int zoom, float lon);

	// Draw track path
	void path(OIIO::ImageBuf &outbuf, TelemetrySource *source, double divider=1.0);

	// Render track
	OIIO::ImageBuf * prepare(bool &is_update);
	OIIO::ImageBuf * render(const TelemetryData &data, bool &is_update);
	void clear(void);

protected:
	OIIO::ImageBuf *bg_buf_;
	OIIO::ImageBuf *fg_buf_;

	Track(GPXApplication &app, const TelemetrySettings &telemetry_settings, const TrackSettings &track_settings, VideoWidget::Widget type, struct event_base *evbase);

	bool preinit(void);
	void init(bool zoomfit=true);
	bool load(void);

	bool drawPicto(OIIO::ImageBuf &map, int x, int y, OIIO::ROI roi, const char *picto, int size);

	void xmlopen(std::ostream &os) {
		log_call();

		os << "<track";
		os <<   " x=\"" << x() << "\" y=\"" << y() << "\"";
		os <<   " width=\"" << theme().width() << "\" height=\"" << theme().height() << "\"";
		os <<   " position=\"" << position2string(position()) << "\"";
		os <<   " orientation=\"" << orientation2string(orientation()) << "\"";
		os <<   " display=\"true\"";
		os <<   ">" << std::endl;
	}

	void xmlclose(std::ostream &os) {
		log_call();

		os << "</track>" << std::endl;
	}

	void xmlwrite(std::ostream &os) {
		IndentingOStreambuf indent(os, 4);

		os << "<margin-left>" << margin(Margin::MarginLeft) << "</margin-left>" << std::endl;
		os << "<margin-right>" << margin(Margin::MarginRight) << "</margin-right>" << std::endl;
		os << "<margin-top>" << margin(Margin::MarginTop) << "</margin-top>" << std::endl;
		os << "<margin-bottom>" << margin(Margin::MarginBottom) << "</margin-bottom>" << std::endl;

		os << "<border>" << theme().border() << "</border>" << std::endl;
		os << "<border-color>" << VideoWidget::Theme::color2hex(theme().borderColor()) << "</border-color>" << std::endl;

		os << "<background-color>" << VideoWidget::Theme::color2hex(theme().backgroundColor()) << "</background-color>" << std::endl;

		os << "<marker>" << settings().markerSize() << "</marker>" << std::endl;

		os << "<path-thick>" << settings().pathThick() << "</path-thick>" << std::endl;
		os << "<path-border>" << settings().pathBorder() << "</path-border>" << std::endl;
	}

	GPXApplication &app_;
	TrackSettings track_settings_;
	TelemetrySettings telemetry_settings_;

	struct event_base *evbase_;

	OIIO::ImageBuf *trackbuf_;

	double divider_;

	// Bounding box
	int x1_, y1_, x2_, y2_;
	int px1_, py1_, px2_, py2_;

	// Start & end position/timestamp
	int x_end_, y_end_;
	int x_start_, y_start_;
	uint64_t ts_start_, ts_end_;
};

#endif
