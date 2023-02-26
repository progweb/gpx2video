#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <math.h>

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
	divider_ = 2.0;
	source_ = MapSettings::SourceNull;
}


MapSettings::~MapSettings() {
}


const MapSettings::Source& MapSettings::source(void) const {
	return source_;
}


void MapSettings::setSource(const MapSettings::Source &source) {
	source_ = source;
}


void MapSettings::setZoom(const int &zoom) {
	zoom_ = zoom;
}


const double& MapSettings::divider(void) const {
	return divider_;
}


void MapSettings::setDivider(const double &divider) {
	divider_ = divider;
}


const std::string MapSettings::getFriendlyName(const MapSettings::Source &source) {
	switch (source) {
	case MapSettings::SourceNull:
		return "None";
	case MapSettings::SourceOpenStreetMap:
		return "OpenStreetMap I";
	case MapSettings::SourceOpenStreetMapRenderer:
		return "OpenStreetMap II";
	case MapSettings::SourceOpenAerialMap:
		return "OpenAerialMap";
	case MapSettings::SourceOpenCycleMap:
		return "OpenCycleMap";
	case MapSettings::SourceOpenTopoMap: 
		return "OpenTopoMap";
	case MapSettings::SourceOSMPublicTransport:
		return "Public Transport";
	case MapSettings::SourceOSMCTrails:
		return "OSMC Trails";
	case MapSettings::SourceMapsForFree:
		return "Maps-For-Free";
	case MapSettings::SourceGoogleStreet:
		return "Google Maps";
	case MapSettings::SourceGoogleSatellite:
		return "Google Satellite";
	case MapSettings::SourceGoogleHybrid:
		return "Google Hybrid";
	case MapSettings::SourceVirtualEarthStreet:
		return "Virtual Earth";
	case MapSettings::SourceVirtualEarthSatellite:
		return "Virtual Earth Satellite";
	case MapSettings::SourceVirtualEarthHybrid:
		return "Virtual Earth Hybrid";
	case MapSettings::SourceIGNEssentielMap:
		return "IGN Essentiel Map";
	case MapSettings::SourceIGNEssentielPhoto:
		return "IGN Essentiel Photo";
	case MapSettings::SourceCount:
	default:
		return "";
	}

	return "";
}


const std::string MapSettings::getCopyright(const MapSettings::Source &source) {
	switch (source) {
	case MapSettings::SourceOpenStreetMap:
		// https://www.openstreetmap.org/copyright
		return "© OpenStreetMap contributors";
	case MapSettings::SourceOpenCycleMap:
		// http://www.thunderforest.com/terms/
		return "Maps © thunderforest.com, Data © osm.org/copyright";
	case MapSettings::SourceOSMPublicTransport:
		return "Maps © ÖPNVKarte, Data © OpenStreetMap contributors";
	case MapSettings::SourceMapsForFree:
		return "Maps © Maps-For-Free";
	case MapSettings::SourceOpenTopoMap:
		return "© OpenTopoMap (CC-BY-SA)";
	case MapSettings::SourceGoogleStreet:
	case MapSettings::SourceGoogleSatellite:
	case MapSettings::SourceGoogleHybrid:
		return "Map provided by Google";
	case MapSettings::SourceVirtualEarthStreet:
	case MapSettings::SourceVirtualEarthSatellite:
	case MapSettings::SourceVirtualEarthHybrid:
		return "Map provided by Microsoft";
	case MapSettings::SourceIGNEssentielMap:
	case MapSettings::SourceIGNEssentielPhoto:
		return "Map provided by IGN";
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
	case MapSettings::SourceIGNEssentielMap:
		return "https://wxs.ign.fr/essentiels/geoportail/wmts?SERVICE=WMTS&REQUEST=GetTile&VERSION=1.0.0&LAYER=GEOGRAPHICALGRIDSYSTEMS.PLANIGNV2&TILEMATRIXSET=PM&TILEMATRIX=#Z&TILECOL=#X&TILEROW=#Y&STYLE=normal&FORMAT=image/png";
	case MapSettings::SourceIGNEssentielPhoto:
		return "https://wxs.ign.fr/essentiels/geoportail/wmts?SERVICE=WMTS&REQUEST=GetTile&VERSION=1.0.0&LAYER=ORTHOIMAGERY.ORTHOPHOTOS&TILEMATRIXSET=PM&TILEMATRIX=#Z&TILECOL=#X&TILEROW=#Y&STYLE=normal&FORMAT=image/jpeg";
	// TODO
	// Add more IGN map
	// Add user key param #K
	// "https://wxs.ign.fr/#K/geoportail/wmts?SERVICE=WMTS&REQUEST=GetTile&VERSION=1.0.0&LAYER=GEOGRAPHICALGRIDSYSTEMS.MAPS&TILEMATRIXSET=PM&TILEMATRIX=#Z&TILECOL=#X&TILEROW=#Y&STYLE=normal&FORMAT=image/jpeg";
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
	case MapSettings::SourceOpenStreetMap:
		return 19;
	case MapSettings::SourceOpenCycleMap:
		return 18;
	case MapSettings::SourceOSMPublicTransport:
		return OSM_MAX_ZOOM;
	case MapSettings::SourceOpenStreetMapRenderer:
	case MapSettings::SourceOpenAerialMap:
	case MapSettings::SourceOpenTopoMap:
		return 17;
	case MapSettings::SourceGoogleStreet:
	case MapSettings::SourceGoogleSatellite:
	case MapSettings::SourceGoogleHybrid:
		return OSM_MAX_ZOOM;
	case MapSettings::SourceVirtualEarthStreet:
	case MapSettings::SourceVirtualEarthSatellite:
	case MapSettings::SourceVirtualEarthHybrid:
		return 19;
	case MapSettings::SourceOSMCTrails:
		return 15;
	case MapSettings::SourceMapsForFree:
		return 11;
	case MapSettings::SourceIGNEssentielMap:
	case MapSettings::SourceIGNEssentielPhoto:
		return 18;
	case MapSettings::SourceCount:
	default:
		return 17;
	}

	return 17;
}

