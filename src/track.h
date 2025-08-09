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


class Track : public VideoWidget {
public:
	virtual ~Track();

	static Track * create(GPXApplication &app, const TelemetrySettings& telemetry_settings, const TrackSettings& track_settings);

	const TrackSettings& settings() const;

	void setSize(int width, int height);

	bool start(void) {
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

	Track(GPXApplication &app, const TelemetrySettings &telemetry_settings, const TrackSettings &track_settings, std::string name, struct event_base *evbase);

	void init(bool zoomfit=true);
	bool load(void);

	bool drawPicto(OIIO::ImageBuf &map, int x, int y, OIIO::ROI roi, const char *picto, int size);

	void xmlopen(std::ostream &os) {
		log_call();

		os << "<track";
		os <<   " x=\"" << x() << "\" y=\"" << y() << "\"";
		os <<   " width=\"" << width() << "\" height=\"" << height() << "\"";
		os <<   " position=\"" << position2string(position()) << "\"";
		os <<   " align=\"" << align2string(align()) << "\"";
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
		os << "<border>" << border() << "</border>" << std::endl;
		os << "<border-color>" << color2hex(textColor()) << "</border-color>" << std::endl;
		os << "<background-color>" << color2hex(backgroundColor()) << "</background-color>" << std::endl;
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
