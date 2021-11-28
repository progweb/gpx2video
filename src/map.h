#ifndef __GPX2VIDEO__MAP_H__
#define __GPX2VIDEO__MAP_H__

#include <iostream>
#include <memory>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <list>

#include <stdlib.h>
#include <event2/event.h>

#include <OpenImageIO/imageio.h>
#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>

#include "evcurl.h"
#include "gpx.h"


class MapSettings {
public:
	enum Source {
		SourceNull,

		SourceOpenStreetMap,
		SourceOpenStreetMapRenderer,
		SourceOpenAerialMap,
		SourceMapsForFree,
		SourceOpenCycleMap,
		SourceOpenTopoMap,
		SourceOSMPublicTransport,
		SourceGoogleStreet,
		SourceGoogleSatellite,
		SourceGoogleHybrid,
		SourceVirtualEarthStreet,
		SourceVirtualEarthSatellite,
		SourceVirtualEarthHybrid,
		SourceOSMCTrails,

		SourceCount
	};

	MapSettings();
	virtual ~MapSettings();

	const Source& source(void) const;
	void setSource(const Source &source);

	const int& zoom(void) const;
	void setZoom(const int &zoom);

	void getBoundingBox(double *lat1, double *lon1, double *lat2, double *lon2) const;
	void setBoundingBox(double lat1, double lon1, double lat2, double lon2);

	static const std::string getFriendlyName(const Source &source);
	static const std::string getCopyright(const Source &source);
	static int getMinZoom(const Source &source);
	static int getMaxZoom(const Source &source);
	static const std::string getRepoURI(const Source &source);

private:
	int zoom_;

	enum Source source_;

	double lat1_, lat2_;
	double lon1_, lon2_;
};


class Map {
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

	static Map * create(const MapSettings& settings, struct event_base *evbase);

	const MapSettings& settings() const;

	static int lat2pixel(int zoom, float lat);
	static int lon2pixel(int zoom, float lon);

	// Download each tule
	void download(void);
	// Draw the full map
	void build(void);
	// Draw track path
	void draw(GPX *gpx);

	static void downloadProgress(Tile &tile, double dltotal, double dlnow);
	static void downloadComplete(Tile &tile);

protected:
	EVCurl *evcurl(void) {
		return evcurl_;
	}

private:
	Map(const MapSettings &settings, struct event_base *evbase);

	void init(void);

	std::string buildURI(int zoom, int x, int y);
	std::string buildPath(int zoom, int x, int y);
	std::string buildFilename(int zoom, int x, int y);

	void drawPicto(OIIO::ImageBuf &map, int x, int y, const char *picto, double divider=1.0);

	MapSettings settings_;

	struct event_base *evbase_;

	EVCurl *evcurl_;

	int x1_, y1_, x2_, y2_;

	unsigned int nbr_downloads_;
	std::list<Tile *> tiles_;
};

#endif

