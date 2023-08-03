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
#include "track.h"
#include "mapsettings.h"
#include "videowidget.h"
#include "gpx2video.h"


class Map : public Track {
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

	bool run(void) {
		log_call();

		init();
		limits();
		download();
		
		return true;
	}

	// Draw track path
	void draw(void);

	// Render map
	OIIO::ImageBuf * prepare(void);
	OIIO::ImageBuf * render(const GPXData &data, bool &is_update);

	static void downloadProgress(Tile &tile, double dltotal, double dlnow);
	static void downloadComplete(Tile &tile);

protected:
	EVCurl *evcurl(void) {
		return evcurl_;
	}

	void init(bool zoomfit=false);
	void limits(void);
	bool load(void);

	// Download each tule
	void download(void);
	// Draw the full map
	void build(void);

private:
	OIIO::ImageBuf *bg_buf_;
	OIIO::ImageBuf *fg_buf_;

	Map(GPX2Video &app, const MapSettings &settings, struct event_base *evbase);

	std::string buildURI(int zoom, int x, int y);
	std::string buildPath(int zoom, int x, int y);
	std::string buildFilename(int zoom, int x, int y);

	MapSettings settings_;

	EVCurl *evcurl_;

	OIIO::ImageBuf *mapbuf_;

	// Map filename to tmp save
	std::string filename_;

	// Bounding box (track area)
	int lim_x1_, lim_y1_, lim_x2_, lim_y2_;

	unsigned int nbr_downloads_;
	std::list<Tile *> tiles_;
};

#endif

