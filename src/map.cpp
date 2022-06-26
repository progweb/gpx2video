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
	width_ = 320;
	height_ = 240;
	divider_ = 2.0;
	source_ = MapSettings::SourceNull;
}


MapSettings::~MapSettings() {
}


const int& MapSettings::width(void) const {
	return width_;
}


const int& MapSettings::height(void) const {
	return height_;
}


void MapSettings::setSize(const int &width, const int &height) {
	width_ = width;
	height_ = height;
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


const double& MapSettings::divider(void) const {
	return divider_;
}


void MapSettings::setDivider(const double &divider) {
	divider_ = divider;
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




Map::Map(GPX2Video &app, const MapSettings &settings, struct event_base *evbase)
	: VideoWidget(app, "map")
	, app_(app)
	, settings_(settings)
	, evbase_(evbase)
	, nbr_downloads_(0) {
	log_call();

	VideoWidget::setSize(settings_.width(), settings_.height()); 

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

	return map;
}


void Map::setSize(int width, int height) {
	VideoWidget::setSize(width, height); 

	settings_.setSize(width, height);
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


void Map::init(void) {
	int zoom;

	double lat1, lon1;
	double lat2, lon2;

	std::string uri;

	Tile *tile;

	double divider = settings().divider();

	log_call();

	zoom = settings().zoom();
	settings().getBoundingBox(&lat1, &lon1, &lat2, &lon2);

	// Tiles:
	// +-------+-------+-------+ ..... +-------+
	// | x1,y1 |       |       |       | x2,y1 |
	// +-------+-------+-------+ ..... +-------+
	// |       |       |       |       |       |
	// +-------+-------+-------+ ..... +-------+
	// | x1,y2 |       |       |       | x2,y2 |
	// +-------+-------+-------+ ..... +-------+

	x1_ = floorf((float) Map::lon2pixel(zoom, lon1) / (float) TILESIZE);
	y1_ = floorf((float) Map::lat2pixel(zoom, lat1) / (float) TILESIZE);

	x2_ = floorf((float) Map::lon2pixel(zoom, lon2) / (float) TILESIZE) + 1;
	y2_ = floorf((float) Map::lat2pixel(zoom, lat2) / (float) TILESIZE) + 1;

	// Append tile so as width tiles sum is enough
	while ((x2_ - x1_) * TILESIZE * divider < 2 * settings().width()) {
		x1_ -= 1;
		x2_ += 1;
	}

	// Append tile so as height tiles sum is enough
	while ((y2_ - y1_) * TILESIZE * divider  < 2 * settings().height()) {
		y1_ -= 1;
		y2_ += 1;
	}

	// Build each tile
	for (int y=y1_; y<y2_; y++) {
		for (int x=x1_; x<x2_; x++) {
			tile = new Tile(*this, zoom, x, y);
			tiles_.push_back(tile);
		}
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

	// Draw markers
	drawPicto(buf, x_start_, y_start_, OIIO::ROI(), "./assets/marker/start.png", 0.3);
	drawPicto(buf, x_end_, y_end_, OIIO::ROI(), "./assets/marker/end.png", 0.3);

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


void Map::path(OIIO::ImageBuf &outbuf, GPX *gpx, double divider) {
	int zoom;
	int stride;
	unsigned char *data;

	int x = 0, y = 0;

	GPXData wpt;

	log_call();

	zoom = settings().zoom();

	// Cairo buffer
	OIIO::ImageBuf buf(outbuf.spec());

	// Create the cairo destination surface
	cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, outbuf.spec().width, outbuf.spec().height);

	// Cairo context
	cairo_t *cairo = cairo_create(surface);

	// Path border
	cairo_set_source_rgb(cairo, 0.0, 0.0, 0.0); // BGR #000000
	cairo_set_line_width(cairo, 4.4); //40.96);
	cairo_set_line_join(cairo, CAIRO_LINE_JOIN_ROUND);

	// Draw each WPT
	for (gpx->retrieveFirst(wpt); wpt.valid(); gpx->retrieveNext(wpt)) {
		x = floorf((float) Map::lon2pixel(zoom, wpt.position().lon)) - (x1_ * TILESIZE);
		y = floorf((float) Map::lat2pixel(zoom, wpt.position().lat)) - (y1_ * TILESIZE);

		x *= divider;
		y *= divider;

		cairo_line_to(cairo, x, y);
	}

	// Cairo draw
	cairo_stroke(cairo);

	// Path color
	cairo_set_source_rgb(cairo, 0.9, 0.4, 0.2); // BGR #669df6
	cairo_set_line_width(cairo, 3.0); //40.96);
	cairo_set_line_join(cairo, CAIRO_LINE_JOIN_ROUND);

	// Draw each WPT
	for (gpx->retrieveFirst(wpt); wpt.valid(); gpx->retrieveNext(wpt)) {
		x = floorf((float) Map::lon2pixel(zoom, wpt.position().lon)) - (x1_ * TILESIZE);
		y = floorf((float) Map::lat2pixel(zoom, wpt.position().lat)) - (y1_ * TILESIZE);

		x *= divider;
		y *= divider;

		cairo_line_to(cairo, x, y);
	}

	// Cairo draw
	cairo_stroke (cairo);

	data = cairo_image_surface_get_data(surface);
	stride = cairo_image_surface_get_stride(surface);

	// Cairo to OIIO
	buf.set_pixels(OIIO::ROI(),
		buf.spec().format,
		data, 
		OIIO::AutoStride,
		stride);

	// Cairo over
	OIIO::ImageBufAlgo::over(outbuf, buf, outbuf);

	// Release
	cairo_surface_destroy(surface);
	cairo_destroy(cairo);
}


bool Map::load(void) {
	if (mapbuf_)
		return true;

	int zoom = settings().zoom();
	double divider = settings().divider();

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

	GPX *gpx = GPX::open(filename);

	if (gpx != NULL) {
		// Draw path
		path(*mapbuf_, gpx, divider);

		// Compute begin
		gpx->retrieveFirst(wpt);

		x_start_ = floorf((float) Map::lon2pixel(zoom, wpt.position().lon)) - (x1_ * TILESIZE);
		y_start_ = floorf((float) Map::lat2pixel(zoom, wpt.position().lat)) - (y1_ * TILESIZE);

		x_start_ *= divider;
		y_start_ *= divider;

		// Compute end
		gpx->retrieveLast(wpt);

		x_end_ = floorf((float) Map::lon2pixel(zoom, wpt.position().lon)) - (x1_ * TILESIZE);
		y_end_ = floorf((float) Map::lat2pixel(zoom, wpt.position().lat)) - (y1_ * TILESIZE);

		x_end_ *= divider;
		y_end_ *= divider;
	}
	else {
		log_warn("Can't open '%s' GPX data file", filename.c_str());
	}

	if (gpx != NULL)
		delete gpx;

	return (mapbuf_ != NULL);
}


void Map::prepare(OIIO::ImageBuf *buf) {
	int x = this->x(); // 1700;
	int y = this->y(); // 900;
	int width = settings().width(); // 800;
	int height = settings().height(); // 500;

	if (this->load() == false)
		log_warn("Map renderer failure");

	// Background
	float color[4] = { 0.0, 0.0, 0.0, 1.0 }; // #000000
	OIIO::ImageBufAlgo::fill(*buf, color, OIIO::ROI(x - 2, x + width + 2, y - 2, y + height + 2));
}


void Map::render(OIIO::ImageBuf *frame, const GPXData &data) {
	int x = this->x(); // 1700;
	int y = this->y(); // 900;
	int width = settings().width(); // 800;
	int height = settings().height(); // 500;

	int posX, posY;
	int offsetX, offsetY;

	int zoom = settings().zoom();
	double divider = settings().divider();

	int offsetMaxX = ((x2_ - x1_) * TILESIZE * divider) - width;
	int offsetMaxY = ((y2_ - y1_) * TILESIZE * divider) - height;

	// Check map buffer
	if (mapbuf_ == NULL) {
		log_warn("Map renderer failure");
		return;
	}

	// Center map
	posX = floorf((float) Map::lon2pixel(zoom, data.position().lon)) - (x1_ * TILESIZE);
	posY = floorf((float) Map::lat2pixel(zoom, data.position().lat)) - (y1_ * TILESIZE);

	posX *= divider;
	posY *= divider;

	offsetX = posX - (width / 2);
	offsetY = posY - (height / 2);

	if (offsetX < 0)
		offsetX = 0;
	if (offsetY < 0)
		offsetY = 0;
	if (offsetX > offsetMaxX)
		offsetX = offsetMaxX; 
	if (offsetY > offsetMaxY)
		offsetY = offsetMaxY; 

	// Image over
	mapbuf_->specmod().x = x - offsetX;
	mapbuf_->specmod().y = y - offsetY;
	OIIO::ImageBufAlgo::over(*frame, *mapbuf_, *frame, OIIO::ROI(x, x + width, y, y + height));

	// Draw track
	// ...

	// Draw picto
	drawPicto(*frame, x - offsetX + x_start_, y - offsetY + y_start_, OIIO::ROI(x, x + width, y, y + height), "./assets/marker/start.png", 0.3);
	drawPicto(*frame, x - offsetX + x_end_, y - offsetY + y_end_, OIIO::ROI(x, x + width, y, y + height), "./assets/marker/end.png", 0.3);
	
	drawPicto(*frame, x - offsetX + posX, y - offsetY + posY, OIIO::ROI(x, x + width, y, y + height), "./assets/marker/position.png", 0.3);
}


bool Map::drawPicto(OIIO::ImageBuf &map, int x, int y, OIIO::ROI roi, const char *picto, double divider) {
	bool result;

	// Open picto
	auto img = OIIO::ImageInput::open(picto);
	const OIIO::ImageSpec& spec = img->spec();
	OIIO::TypeDesc::BASETYPE type = (OIIO::TypeDesc::BASETYPE) spec.format.basetype;

	OIIO::ImageBuf buf = OIIO::ImageBuf(OIIO::ImageSpec(spec.width, spec.height, spec.nchannels, type));
	img->read_image(type, buf.localpixels());

	// Resize picto
	OIIO::ImageBuf dst(OIIO::ImageSpec(spec.width * divider, spec.height * divider, spec.nchannels, type));
	OIIO::ImageBufAlgo::resize(dst, buf);

	// Marker position
	x -= dst.spec().width / 2;
	y -= dst.spec().height - (25 * divider);

	// Image over
	dst.specmod().x = x;
	dst.specmod().y = y;
	result = OIIO::ImageBufAlgo::over(map, dst, map, roi);

	if (!result)
		log_error("ImageBufAlgo::over failure");

	return result;
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

