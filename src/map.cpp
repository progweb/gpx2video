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

#include <OpenImageIO/imageio.h>
#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>

#include <cairo.h>

#include "utils.h"
#include "log.h"
#include "evcurl.h"
#include "gpx.h"
#include "oiioutils.h"
#include "videoparams.h"
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




Map::Map(GPX2Video &app, const MapSettings &settings, struct event_base *evbase)
	: Track(app, settings, evbase)
	, settings_(settings)
	, nbr_downloads_(0) {
	log_call();

	buf_ = NULL;
	mapbuf_ = NULL;

	evcurl_ = EVCurl::init(evbase);
	
	evcurl_->setOption(CURLMOPT_MAXCONNECTS, 1);
	evcurl_->setOption(CURLMOPT_MAX_HOST_CONNECTIONS, 1);
	evcurl_->setOption(CURLMOPT_MAX_PIPELINE_LENGTH, 1);
}


Map::~Map() {
	log_call();

	if (mapbuf_ != NULL)
		delete mapbuf_;
	if (buf_)
		delete buf_;

	delete evcurl_;
}


const MapSettings& Map::settings() const {
	log_call();

	return settings_;
}


Map * Map::create(GPX2Video &app, const MapSettings &settings) {
	Map *map;

	log_call();

	map = new Map(app, settings, app.evbase());

	map->init();
	map->limits();

	return map;
}


void Map::setSize(int width, int height) {
	Track::setSize(width, height); 

	settings_.setSize(width, height);
}


std::string Map::buildURI(int zoom, int x, int y) {
	char s[16];

	int max_zoom = settings().getMaxZoom(settings().source());

	std::string uri = settings().getRepoURI(settings().source());

	log_call();

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


std::string Map::buildPath(int zoom, int x, int y) {
	std::ostringstream stream;

	(void) x;
	(void) y;

	stream << std::getenv("HOME");
	stream << "/.gpx2video/cache/" << settings().source() << "/" << zoom;

	return stream.str();
}


std::string Map::buildFilename(int zoom, int x, int y) {
	std::ostringstream stream;

	(void) zoom;

	stream << "tile_" << y << "_" << x << ".png";

	return stream.str();
}


void Map::init(bool zoomfit) {
	int zoom;

	std::string uri;

	Tile *tile;

	log_call();

	zoom = settings().zoom();
	divider_ = settings().divider();

	// Track compute
	Track::init(zoomfit);

	// Build each tile
	for (int y=y1_; y<y2_; y++) {
		for (int x=x1_; x<x2_; x++) {
			tile = new Tile(*this, zoom, x, y);
			tiles_.push_back(tile);
		}
	}
}


void Map::limits(void) {
	int w, h;
	int width = settings().width();
	int height = settings().height();

	int padding;

	log_call();

	// Use padding (to see markers)
	padding = this->border() + settings().markerSize(); 

	// width x height of track
	w = floorf((float) px2_ - px1_);
	h = floorf((float) py2_ - py1_);

	w *= divider_;
	h *= divider_;

	// Compute display limits
	if ((w + 2 * padding) < width) {
		lim_x1_ = px1_ - (x1_ * TILESIZE);
		lim_x1_ *= divider_;
		lim_x1_ -= (width - w) / 2;

		lim_x2_ = lim_x1_;
	}
	else {
		lim_x1_ = px1_ - (x1_ * TILESIZE);
		lim_x1_ *= divider_;
		lim_x1_ -= padding;

		lim_x2_ = px2_ - (x1_ * TILESIZE);
		lim_x2_ *= divider_;
		lim_x2_ -= width;
		lim_x2_ += padding;
	}

	if ((h + 2 * padding) < height) {
		lim_y1_ = py1_ - (y1_ * TILESIZE);
		lim_y1_ *= divider_;
		lim_y1_ -= (height - h) / 2;

		lim_y2_ = lim_y1_;
	}
	else {
		lim_y1_ = py1_ - (y1_ * TILESIZE);
		lim_y1_ *= divider_;
		lim_y1_ -= padding;

		lim_y2_ = py2_ - (y1_ * TILESIZE);
		lim_y2_ *= divider_;
		lim_y2_ -= height;
		lim_y2_ += padding;
	}
}


void Map::download(void) {
	std::string uri;

	log_call();

	log_notice("Download map from %s...", MapSettings::getFriendlyName(settings().source()).c_str());

	nbr_downloads_ = 1;

	// Build & download each tile
	for (Tile *tile : tiles_) {
//		log_info("Download tile: %s", tile->uri().c_str());

		if (tile->download() == false)
			log_error("Download failure!");
	}
}


void Map::build(void) {
	int fd;

	int width, height;

	const char *template_name = "/tmp/map-XXXXXX";

	log_call();

	log_notice("Build map...");

	// Filename is output if user builds map/track
	if (app_.command() == GPX2Video::CommandMap) {
		filename_ = app_.settings().outputfile();
	}
	else {
		char *s;

		// Make tmp filename
		s = strdup(template_name);
		fd = mkstemp(s);

		filename_ = s;

		close(fd);
		free(s);
	}

	// Map size
	width = (x2_ - x1_) * TILESIZE;
	height = (y2_ - y1_) * TILESIZE;

	// Create map
	std::unique_ptr<OIIO::ImageOutput> out = OIIO::ImageOutput::create("map.png");
	OIIO::ImageSpec outspec(width, height, 4);

	outspec.tile_width = TILESIZE;
	outspec.tile_height = TILESIZE;

	if (out->open(filename_, outspec) == false) {
		log_error("Build map failure, can't open '%s' file", filename_.c_str());
		goto error;
	}

	// Collapse echo tile
	for (Tile *tile : tiles_) {
		std::string filename = tile->path() + "/" + tile->filename();

		// Open tile image
		auto img = OIIO::ImageInput::open(filename.c_str());

		if (img == NULL) {
			log_warn("Can't open '%s' tile", filename.c_str());
			continue;
		}

		const OIIO::ImageSpec& spec = img->spec();
		OIIO::TypeDesc::BASETYPE type = (OIIO::TypeDesc::BASETYPE) spec.format.basetype;

		// Create tile buffer
		OIIO::ImageBuf buf(OIIO::ImageSpec(spec.width, spec.height, spec.nchannels, type));
		img->read_image(type, buf.localpixels());

		// Add alpha channel
		int channelorder[] = { 0, 1, 2, -1 /*use a float value*/ };
		float channelvalues[] = { 0 /*ignore*/, 0 /*ignore*/, 0 /*ignore*/, 1.0 };
		std::string channelnames[] = { "", "", "", "A" };

		OIIO::ImageBuf outbuf = OIIO::ImageBufAlgo::channels(buf, 4, channelorder, channelvalues, channelnames);

		// Image over
		out->write_tile((tile->x() - x1_) * TILESIZE, (tile->y() - y1_) * TILESIZE, 0, type, outbuf.localpixels());
	}

	out->close();

	// User requests track draw
	if (app_.command() == GPX2Video::CommandTrack)
		draw();

error:
	// Done
	complete();
}


void Map::draw(void) {
	int marker_size = settings().markerSize();

	std::string filename = "track.png";

	log_call();

	// Filename is output if user builds map/track
	if (app_.command() == GPX2Video::CommandTrack) {
		filename = app_.settings().outputfile();
	}

	log_notice("Draw track...");

	// Load map
	load();

	// Create output image buffer
	OIIO::ImageBuf buf(mapbuf_->spec());

	// Draw map
	OIIO::ImageBufAlgo::over(buf, *mapbuf_, buf);
	// Draw track
	OIIO::ImageBufAlgo::over(buf, *trackbuf_, buf);

	// Draw markers
	drawPicto(buf, x_end_, y_end_, OIIO::ROI(), "./assets/marker/end.png", marker_size);
	drawPicto(buf, x_start_, y_start_, OIIO::ROI(), "./assets/marker/start.png", marker_size);

	// Save
	std::unique_ptr<OIIO::ImageOutput> out = OIIO::ImageOutput::create(filename);

	if (out->open(filename, buf.spec()) == false) {
		log_error("Draw track failure, can't open '%s' file", filename.c_str());
		goto error;
	}

	out->write_image(buf.spec().format, buf.localpixels());
	out->close();

error:
	return;
}


bool Map::load(void) {
	if (Track::load() == false)
		return false;

	if (mapbuf_)
		return true;

	double divider = divider_; //settings().divider();

	std::string filename = app_.settings().gpxfile();

	GPXData wpt;

	log_call();

	// Open map
	auto img = OIIO::ImageInput::open(filename_);

	if (img == NULL) {
		log_warn("Can't load '%s' map file", filename_.c_str());
		return false;
	}

	const OIIO::ImageSpec& spec = img->spec();
	VideoParams::Format img_fmt = OIIOUtils::getFormatFromOIIOBaseType((OIIO::TypeDesc::BASETYPE) spec.format.basetype);
	OIIO::TypeDesc::BASETYPE type = OIIOUtils::getOIIOBaseTypeFromFormat(img_fmt);

	OIIO::ImageBuf buf(OIIO::ImageSpec(spec.width, spec.height, spec.nchannels, type)); //, OIIO::InitializePixels::No);
	img->read_image(type, buf.localpixels());

	// Resize map
	mapbuf_ = new OIIO::ImageBuf(OIIO::ImageSpec(spec.width * divider, spec.height * divider, spec.nchannels, type)); //, OIIO::InitializePixels::No);
	OIIO::ImageBufAlgo::resize(*mapbuf_, buf);

	return (mapbuf_ != NULL);
}


void Map::prepare(OIIO::ImageBuf *buf) {
	if (buf_ == NULL) {
		this->createBox(&buf_, this->width(), this->height());
		this->drawBorder(buf_);
	}

	// Load map
	if (this->load() == false)
		log_warn("Map renderer failure");

	// Image over
	buf_->specmod().x = this->x();
	buf_->specmod().y = this->y();
	OIIO::ImageBufAlgo::over(*buf, *buf_, *buf, OIIO::ROI());
}


void Map::render(OIIO::ImageBuf *frame, const GPXData &data) {
	int x = this->x();
	int y = this->y();
	int width = settings().width();
	int height = settings().height();

	int posX, posY;
	int offsetX, offsetY;

	int zoom = settings().zoom();
	double divider = divider_; //settings().divider();
	double marker_size = settings().markerSize();

	int border = this->border();

	// Check map & track buffers
	if ((mapbuf_ == NULL) || (trackbuf_ == NULL)) {
		log_warn("Map renderer failure");
		return;
	}

	// Map size
	x += border;
	y += border;
	width -= 2 * border;
	height -= 2 * border;

	// Center map on current position
	posX = floorf((float) Map::lon2pixel(zoom, data.position().lon)) - (x1_ * TILESIZE);
	posY = floorf((float) Map::lat2pixel(zoom, data.position().lat)) - (y1_ * TILESIZE);

	posX *= divider;
	posY *= divider;

	offsetX = posX - (width / 2);
	offsetY = posY - (height / 2);

	if (offsetX < lim_x1_)
		offsetX = lim_x1_;
	if (offsetY < lim_y1_)
		offsetY = lim_y1_;
	if (offsetX > lim_x2_)
		offsetX = lim_x2_;
	if (offsetY > lim_y2_)
		offsetY = lim_y2_;

	// Map image over
	mapbuf_->specmod().x = x - offsetX;
	mapbuf_->specmod().y = y - offsetY;
	OIIO::ImageBufAlgo::over(*frame, *mapbuf_, *frame, OIIO::ROI(x, x + width, y, y + height));

	// Track image over
	trackbuf_->specmod().x = x - offsetX;
	trackbuf_->specmod().y = y - offsetY;
	OIIO::ImageBufAlgo::over(*frame, *trackbuf_, *frame, OIIO::ROI(x, x + width, y, y + height));

	// Draw track
	// ...

	// Draw picto
	if (marker_size > 0) {
		drawPicto(*frame, x - offsetX + x_end_, y - offsetY + y_end_, OIIO::ROI(x, x + width, y, y + height), "./assets/marker/end.png", marker_size);
		drawPicto(*frame, x - offsetX + x_start_, y - offsetY + y_start_, OIIO::ROI(x, x + width, y, y + height), "./assets/marker/start.png", marker_size);
	
		if (data.valid())
			drawPicto(*frame, x - offsetX + posX, y - offsetY + posY, OIIO::ROI(x, x + width, y, y + height), "./assets/marker/position.png", marker_size);
	}
}


void Map::downloadProgress(Map::Tile &tile, double dltotal, double dlnow) {
	char buf[64];
	const char *label = "Download tile";

	Map& map = tile.map();

	int percent = (dltotal > 0) ? (int) (dlnow * 100 / dltotal) : 0;

	memset(buf, '.', 50);          
	memset(buf, '#', percent / 2); 
	buf[50] = '\0';

	if (percent == 100) 
		printf("\r  %s %d / %d [%s] DONE      ",          
				label, map.nbr_downloads_, (unsigned int) map.tiles_.size(), buf);
	else
		printf("\r  %s %d / %d [%s] %3d %%",
				label, map.nbr_downloads_, (unsigned int) map.tiles_.size(), buf, percent);
}


void Map::downloadComplete(Map::Tile &tile) {
	char buf[64];
	const char *label = "Download tile";

	Map& map = tile.map();

	memset(buf, '#', 50);          
	buf[50] = '\0';

	printf("\r  %s %d / %d [%s] DONE      ",          
			label, map.nbr_downloads_, (unsigned int) map.tiles_.size(), buf);

	map.nbr_downloads_++;

	if (map.nbr_downloads_ <= (unsigned int) map.tiles_.size())
		return;

	printf("\n");

	// Now build full map)
	map.build();
}


Map::Tile::Tile(Map &map, int zoom, int x, int y)
	: map_(map)
	, zoom_(zoom)
	, x_(x)
	, y_(y) {
	fp_ = NULL;
	evtaskh_ = NULL;

	uri_ = map_.buildURI(zoom_, x_, y_);
	path_ = map_.buildPath(zoom_, x_, y_);
	filename_ = map_.buildFilename(zoom_, x_, y_);
}


Map::Tile::~Tile() {
}


Map& Map::Tile::map(void) {
	return map_;
}


const std::string& Map::Tile::uri(void) {
	return uri_;
}


const std::string& Map::Tile::path(void) {
	return path_;
}


const std::string& Map::Tile::filename(void) {
	return filename_;
}


int Map::Tile::downloadDebug(CURL *curl, curl_infotype type, char *ptr, size_t size, void *userdata) {
	(void) curl;
	(void) type;
	(void) userdata;

	fwrite(ptr, size, 1, stdout);

	return 0;
}


int Map::Tile::downloadProgress(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow) {
	Map::Tile *tile = (Map::Tile *) clientp;

	(void) ultotal;
	(void) ulnow;

	Map::downloadProgress(*tile, dltotal, dlnow);

	return 0;
}


size_t Map::Tile::downloadWrite(char *ptr, size_t size, size_t nmemb, void *userdata) {
	Map::Tile *tile = (Map::Tile *) userdata;

	if (ptr == NULL)
		return 0;

	std::string output = tile->path_ + "/" + tile->filename_;

	// Open output tile file
	if (tile->fp_ == NULL) {
		tile->fp_ = std::fopen(output.c_str(), "w+");

		if (tile->fp_ == NULL)
			return 0;
	}

	fwrite(ptr, size, nmemb, tile->fp_);

	return size * nmemb;
}


void Map::Tile::downloadComplete(EVCurlTask *evtaskh, CURLcode result, void *userdata) {
	Map::Tile *tile = (Map::Tile *) userdata;

	log_call();

	(void) evtaskh;
	(void) result;

	if (tile->fp_)
		std::fclose(tile->fp_);

	if (result != CURLE_OK) {
		std::string output = tile->path_ + "/" + tile->filename_;

		log_error("\nDownload tile failure: %s", tile->uri().c_str());

		unlink(output.c_str());
	}

	tile->fp_ = NULL;
	tile->evtaskh_ = NULL;

	Map::downloadComplete(*tile);
}


bool Map::Tile::download(void) {
	struct stat st;

	std::string output = path_ + "/" + filename_;

	log_call();

	::mkpath(path_, 0700);

	// Check if file exists in cache
	if (access(output.c_str(), F_OK) == 0) {
		// Check file content
		stat(output.c_str(), &st);

		if (st.st_size > 0) {
			Map::downloadComplete(*this);
			return true;
		}
	}
	
	// Download
	evtaskh_ = map_.evcurl()->download(uri_.c_str(), downloadComplete, this);

//	evtaskh_->setOption(CURLOPT_VERBOSE, 1L);
//	evtaskh_->setOption(CURLOPT_DEBUGFUNCTION, downloadDebug);
//	evtaskh_->setOption(CURLOPT_DEBUGDATA, this);

	evtaskh_->setOption(CURLOPT_NOPROGRESS, 0L);
	evtaskh_->setOption(CURLOPT_PROGRESSFUNCTION, downloadProgress);
	evtaskh_->setOption(CURLOPT_PROGRESSDATA, this);
	
	evtaskh_->setOption(CURLOPT_WRITEFUNCTION, downloadWrite);
	evtaskh_->setOption(CURLOPT_WRITEDATA, this);

	evtaskh_->setOption(CURLOPT_FOLLOWLOCATION, 1L);

	evtaskh_->setHeader("User-Agent: gpx2video");

	evtaskh_->perform();

	return true;
}

