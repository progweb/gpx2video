#include <cstring>
#include <math.h>

#include "utils.h"
#include "map.h"


#define TILESIZE 256
#define MAX_ZOOM 20
#define MIN_ZOOM 0

#define OSM_REPO_URI        "https://tile.openstreetmap.org/#Z/#X/#Y.png"
#define OSM_MIN_ZOOM        1
#define OSM_MAX_ZOOM        20
#define OSM_IMAGE_FORMAT    "png"

#define URI_MARKER_X    "#X"
#define URI_MARKER_Y    "#Y"
#define URI_MARKER_Z    "#Z"
#define URI_MARKER_S    "#S"
#define URI_MARKER_Q    "#Q"
#define URI_MARKER_Q0   "#W"
#define URI_MARKER_YS   "#U"
#define URI_MARKER_R    "#R"



MapSettings::MapSettings() {
}


MapSettings::~MapSettings() {
}


const MapSettings::Source& MapSettings::source(void) const {
	return source_;
}


void MapSettings::setSource(const MapSettings::Source &source) {
	source_ = source;
}


const int& MapSettings::zoom(void) const {
	return zoom_;
}


void MapSettings::setZoom(const int &zoom) {
	zoom_ = zoom;
}


void MapSettings::getBoundingBox(double *lat1, double *lon1, double *lat2, double *lon2) const {
	*lat1 = lat1_;
	*lon1 = lon1_;

	*lat2 = lat2_;
	*lon2 = lon2_;
}


void MapSettings::setBoundingBox(double lat1, double lon1, double lat2, double lon2) {
	lat1_ = lat1;
	lon1_ = lon1;

	lat2_ = lat2;
	lon2_ = lon2;
}


const std::string MapSettings::getFriendlyName(const MapSettings::Source &source) {
	switch (source) {
	case SourceNull:
		return "None";
	case SourceOpenStreetMap:
		return "OpenStreetMap I";
	case SourceOpenStreetMapRenderer:
		return "OpenStreetMap II";
	case SourceOpenAerialMap:
		return "OpenAerialMap";
	case SourceOpenCycleMap:
		return "OpenCycleMap";
	case SourceOpenTopoMap: 
		return "OpenTopoMap";
	case SourceOSMPublicTransport:
		return "Public Transport";
	case SourceOSMCTrails:
		return "OSMC Trails";
	case SourceMapsForFree:
		return "Maps-For-Free";
	case SourceGoogleStreet:
		return "Google Maps";
	case SourceGoogleSatellite:
		return "Google Satellite";
	case SourceGoogleHybrid:
		return "Google Hybrid";
	case SourceVirtualEarthStreet:
		return "Virtual Earth";
	case SourceVirtualEarthSatellite:
		return "Virtual Earth Satellite";
	case SourceVirtualEarthHybrid:
		return "Virtual Earth Hybrid";
	case SourceCount:
	default:
		return "";
	}

	return "";
}


const std::string MapSettings::getCopyright(const MapSettings::Source &source) {
	switch (source) {
	case SourceOpenStreetMap:
		// https://www.openstreetmap.org/copyright
		return "© OpenStreetMap contributors";
	case SourceOpenCycleMap:
		// http://www.thunderforest.com/terms/
		return "Maps © thunderforest.com, Data © osm.org/copyright";
	case SourceOSMPublicTransport:
		return "Maps © ÖPNVKarte, Data © OpenStreetMap contributors";
	case SourceMapsForFree:
		return "Maps © Maps-For-Free";
	case SourceOpenTopoMap:
		return "© OpenTopoMap (CC-BY-SA)";
	case SourceGoogleStreet:
		return "Map provided by Google";
	case SourceGoogleSatellite:
		return "Map provided by Google ";
	case SourceGoogleHybrid:
		return "Map provided by Google";
	case SourceVirtualEarthStreet:
		return "Map provided by Microsoft";
	case SourceVirtualEarthSatellite:
		return "Map provided by Microsoft";
	case SourceVirtualEarthHybrid:
		return "Map provided by Microsoft";
	default:
		return "";
	}

	return "";
}


// http://www.internettablettalk.com/forums/showthread.php?t=5209
// https://garage.maemo.org/plugins/scmsvn/viewcvs.php/trunk/src/maps.c?root=maemo-mapper&view=markup
// http://www.ponies.me.uk/maps/GoogleTileUtils.java
// http://www.mgmaps.com/cache/MapTileCacher.perl
const std::string MapSettings::getRepoURI(const MapSettings::Source &source) {
	switch (source) {
	case MapSettings::SourceNull:
		return "none://";
	case MapSettings::SourceOpenStreetMap:
		return OSM_REPO_URI;
	case MapSettings::SourceOpenAerialMap:
		// OpenAerialMap is down, offline till furthur notice
		// http://openaerialmap.org/pipermail/talk_openaerialmap.org/2008-December/000055.html
		return "";
	case MapSettings::SourceOpenStreetMapRenderer:
		// The Tile@Home serverhas been shut down.
		// return "http://tah.openstreetmap.org/Tiles/tile/#Z/#X/#Y.png";
		return "";
	case MapSettings::SourceOpenCycleMap:
		// return "http://c.andy.sandbox.cloudmade.com/tiles/cycle/#Z/#X/#Y.png";
		return "http://b.tile.opencyclemap.org/cycle/#Z/#X/#Y.png";
	case MapSettings::SourceOSMPublicTransport:
		return "http://tile.xn--pnvkarte-m4a.de/tilegen/#Z/#X/#Y.png";
	case MapSettings::SourceOSMCTrails:
		// Appears to be shut down
		return "";
	case MapSettings::SourceMapsForFree:
		return "https://maps-for-free.com/layer/relief/z#Z/row#Y/#Z_#X-#Y.jpg";
	case MapSettings::SourceOpenTopoMap:
		return "https://a.tile.opentopomap.org/#Z/#X/#Y.png";
	case MapSettings::SourceGoogleStreet:
		return "http://mt#R.google.com/vt/lyrs=m&hl=en&x=#X&s=&y=#Y&z=#Z";
	case MapSettings::SourceGoogleHybrid:
		return "http://mt#R.google.com/vt/lyrs=y&hl=en&x=#X&s=&y=#Y&z=#Z";
	case MapSettings::SourceGoogleSatellite:
		return "http://mt#R.google.com/vt/lyrs=s&hl=en&x=#X&s=&y=#Y&z=#Z";
	case MapSettings::SourceVirtualEarthStreet:
		return "http://a#R.ortho.tiles.virtualearth.net/tiles/r#W.jpeg?g=50";
	case MapSettings::SourceVirtualEarthSatellite:
		return "http://a#R.ortho.tiles.virtualearth.net/tiles/a#W.jpeg?g=50";
	case MapSettings::SourceVirtualEarthHybrid:
		return "http://a#R.ortho.tiles.virtualearth.net/tiles/h#W.jpeg?g=50";
	case MapSettings::SourceCount:
	default:
		return "";
	}

	return "";
}


int MapSettings::getMinZoom(const MapSettings::Source &source) {
	(void) source;

	return 1;
}


int MapSettings::getMaxZoom(const MapSettings::Source &source) {
	switch (source) {
	case MapSettings::SourceNull:
		return 18;
	case SourceOpenStreetMap:
		return 19;
	case SourceOpenCycleMap:
		return 18;
	case SourceOSMPublicTransport:
		return OSM_MAX_ZOOM;
	case SourceOpenStreetMapRenderer:
	case SourceOpenAerialMap:
	case SourceOpenTopoMap:
		return 17;
	case SourceGoogleStreet:
	case SourceGoogleSatellite:
	case SourceGoogleHybrid:
		return OSM_MAX_ZOOM;
	case SourceVirtualEarthStreet:
	case SourceVirtualEarthSatellite:
	case SourceVirtualEarthHybrid:
		return 19;
	case MapSettings::SourceOSMCTrails:
		return 15;
	case SourceMapsForFree:
		return 11;
	case MapSettings::SourceCount:
	default:
		return 17;
	}

	return 17;
}




Map::Map(const MapSettings &settings) :
	settings_(settings) {
}


Map::~Map() {
}


const MapSettings& Map::settings() const {
	return settings_;
}


Map * Map::create(const MapSettings &settings) {
	Map *encoder = new Map(settings);

	return encoder;
}


int Map::lat2pixel(int zoom, float lat) {
    float lat_m;
    int pixel_y;

    double latrad = lat * M_PI / 180.0;

    lat_m = atanh(sin(latrad));

    // the formula is some more notes
    // http://manialabs.wordpress.com/2013/01/26/converting-latitude-and-longitude-to-map-tile-in-mercator-projection/
    //
    // pixel_y = -(2^zoom * TILESIZE * lat_m) / 2PI + (2^zoom * TILESIZE) / 2
    pixel_y = -(int)( (lat_m * TILESIZE * (1 << zoom) ) / (2*M_PI)) +
        ((1 << zoom) * (TILESIZE/2) );

    return pixel_y;
}


int Map::lon2pixel(int zoom, float lon) {
    int pixel_x;

    double lonrad = lon * M_PI / 180.0;

    // the formula is
    //
    // pixel_x = (2^zoom * TILESIZE * lon) / 2PI + (2^zoom * TILESIZE) / 2
    pixel_x = (int)(( lonrad * TILESIZE * (1 << zoom) ) / (2*M_PI)) +
        ( (1 << zoom) * (TILESIZE/2) );

    return pixel_x;
}


std::string Map::buildURI(int zoom, int x, int y) {
	char s[16];

	int max_zoom = settings().getMaxZoom(settings().source());

	std::string uri = settings().getRepoURI(settings().source());

	if (std::strstr(uri.c_str(), URI_MARKER_X)) {
		snprintf(s, sizeof(s), "%d", x);
		uri = replace(uri, URI_MARKER_X, s);
	}

	if (std::strstr(uri.c_str(), URI_MARKER_Y)) {
		snprintf(s, sizeof(s), "%d", y);
		uri = replace(uri, URI_MARKER_Y, s);
	}

	if (std::strstr(uri.c_str(), URI_MARKER_Z)) {
		snprintf(s, sizeof(s), "%d", zoom);
		uri = replace(uri, URI_MARKER_Z, s);
	}

	if (std::strstr(uri.c_str(), URI_MARKER_S)) {
		snprintf(s, sizeof(s), "%d", max_zoom-zoom);
		uri = replace(uri, URI_MARKER_S, s);
	}

	if (std::strstr(uri.c_str(), URI_MARKER_Q)) {
//		map_convert_coords_to_quadtree_string(map, x, y, zoom, location, 't', "qrts");
//		uri = replace(uri, URI_MARKER_Q, location);
	}

	if (std::strstr(uri.c_str(), URI_MARKER_Q0)) {
//		map_convert_coords_to_quadtree_string(map, x, y, zoom, location, '\0', "0123");
//		uri = replace(uri, URI_MARKER_Q0, location);
	}

	if (std::strstr(uri.c_str(), URI_MARKER_YS)) {
	}

	if (std::strstr(uri.c_str(), URI_MARKER_R)) {
		snprintf(s, sizeof(s), "%d", (int) (random() % 4));
		uri = replace(uri, URI_MARKER_R, s);
	}

	if (std::strstr(uri.c_str(), "google.com")) {
	}

	return uri;
}


void Map::download(void) {
	int zoom;
	int x1, y1, x2, y2;

	double lat1, lon1;
	double lat2, lon2;

	std::string uri;

	zoom = settings().zoom();
	settings().getBoundingBox(&lat1, &lon1, &lat2, &lon2);

	x1 = floorf((float) Map::lon2pixel(zoom, lon1) / (float) TILESIZE);
	y1 = floorf((float) Map::lat2pixel(zoom, lat1) / (float) TILESIZE);

	x2 = floorf((float) Map::lon2pixel(zoom, lon2) / (float) TILESIZE);
	y2 = floorf((float) Map::lat2pixel(zoom, lat2) / (float) TILESIZE);

	for (int y=y1; y<y2; y++) {
		for (int x=x1; x<x2; x++) {
			uri = this->buildURI(zoom, x, y);
		
//			printf("URI: curl -q -o tile-%04d-%04d.png \"%s\"\n", y, x, uri.c_str());
			printf("URI: %s\n", uri.c_str());
		}
	}
}

