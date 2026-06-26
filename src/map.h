#ifndef __GPX2VIDEO__MAP_H__
#define __GPX2VIDEO__MAP_H__

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

#include "evcurl.h"
#include "track.h"
#include "mapsettings.h"
#include "videowidget.h"
#include "telemetrymedia.h"
#include "application.h"


class Map : public Track {
public:
	class Tile {
	public:
		time_t last_update_;

		Tile(Map &map, int zoom, int x, int y);
		virtual ~Tile();

		Map& map(void);
		int x(void) {
			return x_;
		}
		int y(void) {
			return y_;
		}
		const std::string& uri(void);
		const std::string& path(void);
		const std::string& filename(void);
		bool download(void);

	protected:
		static int downloadDebug(CURL *curl, curl_infotype type, char *ptr, size_t size, void *userdata);
		static int downloadProgress(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);
		static size_t downloadWrite(char *ptr, size_t size, size_t nmemb, void *userdata);
		static void downloadComplete(EVCurlTask *evtaskh, CURLcode result, void *userdata);

	private:
		Map &map_;
		int zoom_;
		int x_, y_;
		std::string uri_;
		std::string path_;
		std::string filename_;
		std::FILE *fp_;
		EVCurlTask *evtaskh_;
	};

	virtual ~Map();

	static Map * create(GPXApplication &app, const MapSettings& map_settings, TelemetrySource *telemetry_source = NULL);

	MapSettings& settings(void);
	const MapSettings& settings(void) const;

	bool hasFeature(ShapeBase::Feature feature) const {
		switch (feature) {
//		case FeatureRoundCorner:
//			return true;

		default:
			break;
		}

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
		download();

		return true;
	}

	// Draw track path
	void draw(void);

	// Render map
	OIIO::ImageBuf * render(const TelemetryData &data, bool &is_update);

	bool updated(const TelemetryData &data) const;
	void draw(cairo_t *cr, const TelemetryData &data);
	void clear(void);

	static void downloadProgress(Tile &tile, curl_off_t dltotal, curl_off_t dlnow);
	static void downloadComplete(Tile &tile);

protected:
	EVCurl *evcurl(void) {
		return evcurl_;
	}

	void init(void);
	bool load(void);

	// Download each tule
	void download(void);
	// Draw the full map
	void build(void);

	void xmlopen(std::ostream &os) {
		log_call();

		os << "<map";
		os <<   " x=\"" << x() << "\" y=\"" << y() << "\"";
		os <<   " width=\"" << theme().width() << "\" height=\"" << theme().height() << "\"";
		os <<   " position=\"" << position2string(position()) << "\"";
		os <<   " orientation=\"" << orientation2string(orientation()) << "\"";
		os <<   " display=\"true\"";
		os <<   ">" << std::endl;
	}

	void xmlclose(std::ostream &os) {
		log_call();

		os << "</map>" << std::endl;
	}

	void xmlwrite(std::ostream &os) {
		Track::xmlwrite(os);

		os << "<source>" << settings().source() << "</source>" << std::endl;
		os << "<zoom>" << settings().zoom() << "</zoom>" << std::endl;
	}

private:
	OIIO::ImageBuf *bg_buf_;
	OIIO::ImageBuf *fg_buf_;

	Map(GPXApplication &app, const MapSettings &map_settings, TelemetrySource *telemetry_source, struct event_base *evbase);

	std::string buildURI(int zoom, int x, int y);
	std::string buildPath(int zoom, int x, int y);
	std::string buildFilename(int zoom, int x, int y);

	MapSettings map_settings_;

	EVCurl *evcurl_;

	OIIO::ImageBuf *mapbuf_;

	// Map filename to tmp save
	std::string filename_;

	// Bounding box (tile view area)
	int vx1_, vy1_, vx2_, vy2_;

	// Bounding box (track area)
	int lim_x1_, lim_y1_, lim_x2_, lim_y2_;

	bool refresh_is_required_;

	unsigned int nbr_downloads_;
	std::list<Tile *> tiles_;
};

#endif

