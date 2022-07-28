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

#include "log.h"
#include "evcurl.h"
#include "gpx.h"
#include "mapsettings.h"
#include "videowidget.h"
#include "gpx2video.h"


class Map : public VideoWidget {
public:
	class Tile {
	public:
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
		static int downloadProgress(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow);
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

	static Map * create(GPX2Video &app, const MapSettings& settings);

	const MapSettings& settings() const;

	void setSize(int width, int height);

	static int lat2pixel(int zoom, float lat);
	static int lon2pixel(int zoom, float lon);

	bool run(void) {
		log_call();

		download();
		
		return true;
	}

	// Draw track path
	void draw(void);
	void path(OIIO::ImageBuf &outbuf, GPX *gpx, double divider=1.0);

	// Render map
	void prepare(OIIO::ImageBuf *buf);
	void render(OIIO::ImageBuf *frame, const GPXData &data);

	static void downloadProgress(Tile &tile, double dltotal, double dlnow);
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

private:
	OIIO::ImageBuf *buf_;

//	Map(const MapSettings &settings, struct event_base *evbase);
	Map(GPX2Video &app, const MapSettings &settings, struct event_base *evbase);

	std::string buildURI(int zoom, int x, int y);
	std::string buildPath(int zoom, int x, int y);
	std::string buildFilename(int zoom, int x, int y);

	bool drawPicto(OIIO::ImageBuf &map, int x, int y, OIIO::ROI roi, const char *picto, int size);

	GPX2Video &app_;
	MapSettings settings_;

	struct event_base *evbase_;

	EVCurl *evcurl_;

	OIIO::ImageBuf *mapbuf_;

	// Map filename to tmp save
	std::string filename_;

	// Bounding box (map area)
	int x1_, y1_, x2_, y2_;
	int px1_, py1_, px2_, py2_;

	// Bounding box (track area)
	int lim_x1_, lim_y1_, lim_x2_, lim_y2_;

	unsigned int nbr_downloads_;
	std::list<Tile *> tiles_;

	// Start & end position
	int x_end_, y_end_;
	int x_start_, y_start_;
};

#endif

