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

	static Track * create(GPXApplication &app, const TrackSettings& track_settings, TelemetrySource *telemetry_source = NULL);

	ShapeBase * shape(void) {
		return this;
	}

	TrackSettings& settings();
	const TrackSettings& settings() const;

	bool hasFeature(ShapeBase::Feature feature) const {
		switch (feature) {
		case FeatureRoundCorner:
			return true;

		default:
			break;
		}

		return false;
	}

	void setSettings(const TrackSettings &settings);

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

	static int lat2pixel(int zoom, double divider, float lat);
	static int lon2pixel(int zoom, double divider, float lon);

	// Draw track path
	void path(OIIO::ImageBuf &outbuf, TelemetrySource *source, double divider=1.0);
	void path(OIIO::ImageBuf &outbuf, const TelemetryData &data, double divider=1.0);

	// Render track
	OIIO::ImageBuf * render(const TelemetryData &data, bool &is_update);

	bool updated(const TelemetryData &data) const;
	void draw(cairo_t *cr, const TelemetryData &data);
	void clear(void);

	std::string getIconFilename(TrackSettings::Icon icon, TrackSettings::Icon bydefault=TrackSettings::IconUnknown);

protected:
	OIIO::ImageBuf *bg_buf_;
	OIIO::ImageBuf *fg_buf_;

	Track(GPXApplication &app, const TrackSettings &track_settings, VideoWidget::Widget type, TelemetrySource *telemetry_source, struct event_base *evbase);

	bool isInitialized(void) {
		return is_init_;
	}

	bool preinit(void);
	void init(void);
	bool load(void);

	bool icon(OIIO::ImageBuf &map, OIIO::ImageBuf &icon, int x, int y, OIIO::ROI roi);

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

	void xmlwrite(std::ostream &os);

	GPXApplication &app_;
	TrackSettings track_settings_;

	struct event_base *evbase_;

	bool is_init_;

	std::string assets_path_;

	OIIO::ImageBuf *trackbuf_;

	OIIO::ImageBuf *icon_end_buf_;
	OIIO::ImageBuf *icon_start_buf_;
	OIIO::ImageBuf *icon_position_buf_;

	TelemetryData last_data_;

	double divider_;

	int last_posX_, last_posY_;

	// Data boudning box (range)
	TelemetryData lim_p1_, lim_p2_;

	// Bounding box (pixel view area)
	int pvx1_, pvy1_, pvx2_, pvy2_;
	// Bounding box (pixel extended view area)
	int pevx1_, pevy1_, pevx2_, pevy2_;
	// Bounding box (pixel data range)
	int lim_px1_, lim_py1_, lim_px2_, lim_py2_;

	// Start & end position/timestamp
	int x_end_, y_end_;
	int x_start_, y_start_;
	uint64_t ts_start_, ts_end_;
};

#endif
