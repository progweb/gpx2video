#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <locale>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <math.h>

#include <OpenImageIO/imageio.h>
#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>

#include <cairo.h>

#include "utils.h"
#include "log_i.h"
#include "evcurl.h"
#include "oiioutils.h"
#include "videoparams.h"
#include "telemetrymedia.h"
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



MapSettings::MapSettings() 
	: TrackSettings() {
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
//		return "https://wxs.ign.fr/essentiels/geoportail/wmts?SERVICE=WMTS&REQUEST=GetTile&VERSION=1.0.0&LAYER=GEOGRAPHICALGRIDSYSTEMS.PLANIGNV2&TILEMATRIXSET=PM&TILEMATRIX=#Z&TILECOL=#X&TILEROW=#Y&STYLE=normal&FORMAT=image/png";
//		return "https://wmts.geopf.fr/wmts?layer=GEOGRAPHICALGRIDSYSTEMS.PLANIGNV2&style=normal&tilematrixset=PM&Service=WMTS&Request=GetTile&Version=1.0.0&Format=image%2Fpng&TileMatrix=#Z&TileCol=#X&TileRow=#Y";
		return "https://data.geopf.fr/wmts?layer=GEOGRAPHICALGRIDSYSTEMS.PLANIGNV2&style=normal&tilematrixset=PM&Service=WMTS&Request=GetTile&Version=1.0.0&Format=image%2Fpng&TileMatrix=#Z&TileCol=#X&TileRow=#Y";
	case MapSettings::SourceIGNEssentielPhoto:
//		return "https://wxs.ign.fr/essentiels/geoportail/wmts?SERVICE=WMTS&REQUEST=GetTile&VERSION=1.0.0&LAYER=ORTHOIMAGERY.ORTHOPHOTOS&TILEMATRIXSET=PM&TILEMATRIX=#Z&TILECOL=#X&TILEROW=#Y&STYLE=normal&FORMAT=image/jpeg";
//		return "https://wmts.geopf.fr/wmts?layer=ORTHOIMAGERY.ORTHOPHOTOS&style=normal&tilematrixset=PM&Service=WMTS&Request=GetTile&Version=1.0.0&Format=image%2Fjpeg&TileMatrix=#Z&TileCol=#X&TileRow=#Y";
		return "https://data.geopf.fr/wmts?layer=ORTHOIMAGERY.ORTHOPHOTOS&style=normal&tilematrixset=PM&Service=WMTS&Request=GetTile&Version=1.0.0&Format=image%2Fjpeg&TileMatrix=#Z&TileCol=#X&TileRow=#Y";
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




Map::Map(GPXApplication &app, const MapSettings &map_settings, TelemetrySource *telemetry_source, struct event_base *evbase)
	: Track(app, map_settings, VideoWidget::WidgetMap, telemetry_source, evbase)
	, map_settings_(map_settings)
	, nbr_downloads_(0) {
	log_call();

	bg_buf_ = NULL;
	fg_buf_ = NULL;
	mapbuf_ = NULL;

	filename_ = "";

	refresh_is_required_ = false;

	evcurl_ = EVCurl::init(evbase);
	
	evcurl_->setOption(CURLMOPT_MAXCONNECTS, 1);
	evcurl_->setOption(CURLMOPT_MAX_HOST_CONNECTIONS, 1);
	evcurl_->setOption(CURLMOPT_MAX_PIPELINE_LENGTH, 1);
}


Map::~Map() {
	log_call();

	if (filename_ != "")
		::unlink(filename_.c_str());

	if (mapbuf_ != NULL)
		delete mapbuf_;
	if (bg_buf_)
		delete bg_buf_;
	if (fg_buf_)
		delete fg_buf_;

	delete evcurl_;
}


MapSettings& Map::settings(void) {
	log_call();

	return map_settings_;
}


const MapSettings& Map::settings(void) const {
	log_call();

	return map_settings_;
}


Map * Map::create(GPXApplication &app, const MapSettings &map_settings, TelemetrySource *telemetry_source) {
	Map *map;

	log_call();

	map = new Map(app, map_settings, telemetry_source,  app.evbase());

	return map;
}


void Map::setSize(int width, int height) {
	Track::setSize(width, height); 

	map_settings_.setSize(width, height);
}


std::string Map::buildURI(int zoom, int x, int y) {
	char s[16];

	int max_zoom = MapSettings::getMaxZoom(settings().source());

	std::string uri = MapSettings::getRepoURI(settings().source());

	log_call();

	if (std::strstr(uri.c_str(), URI_MARKER_X)) {
		snprintf(s, sizeof(s), "%d", x);
		uri = Utils::replace(uri, URI_MARKER_X, s);
	}

	if (std::strstr(uri.c_str(), URI_MARKER_Y)) {
		snprintf(s, sizeof(s), "%d", y);
		uri = Utils::replace(uri, URI_MARKER_Y, s);
	}

	if (std::strstr(uri.c_str(), URI_MARKER_Z)) {
		snprintf(s, sizeof(s), "%d", zoom);
		uri = Utils::replace(uri, URI_MARKER_Z, s);
	}

	if (std::strstr(uri.c_str(), URI_MARKER_S)) {
		snprintf(s, sizeof(s), "%d", max_zoom-zoom);
		uri = Utils::replace(uri, URI_MARKER_S, s);
	}

	if (std::strstr(uri.c_str(), URI_MARKER_Q)) {
//		map_convert_coords_to_quadtree_string(map, x, y, zoom, location, 't', "qrts");
//		uri = Utils::replace(uri, URI_MARKER_Q, location);
	}

	if (std::strstr(uri.c_str(), URI_MARKER_Q0)) {
//		map_convert_coords_to_quadtree_string(map, x, y, zoom, location, '\0', "0123");
//		uri = Utils::replace(uri, URI_MARKER_Q0, location);
	}

	if (std::strstr(uri.c_str(), URI_MARKER_YS)) {
	}

	if (std::strstr(uri.c_str(), URI_MARKER_R)) {
		snprintf(s, sizeof(s), "%d", (int) (random() % 4));
		uri = Utils::replace(uri, URI_MARKER_R, s);
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

	struct no_separator : std::numpunct<char> {
	public:
		no_separator(std::locale& base_locale) :
			np(std::use_facet< std::numpunct<char> >(base_locale)) {
		}

	protected:
		string_type do_grouping() const override { return ""; }
//		char_type do_decimal_point() const override { return np.decimal_point(); }
//		char_type do_thousands_sep() const override { return np.thousands_sep(); }
//		string_type do_truename() const override { return np.truename(); }
//		string_type do_falsename() const override { return np.truename(); }

		const std::numpunct<char> & np;
	};

	(void) zoom;

	std::locale locale("");

	stream.imbue(std::locale(locale, new no_separator(locale)));
	stream << "tile_" << y << "_" << x << ".png";

	return stream.str();
}


void Map::init(void) {
	int zoom;

	double space;

	std::string uri;

	Tile *tile;

	int offsetx, offsety;

	int width, height;
	int width_available, height_available;

	int data_width, data_height;

	int padding_horizontal, padding_vertical;

	log_call();

	zoom = settings().zoom();
	divider_ = settings().divider();

	// Update track settings
	Track::setSettings(settings());

	// Track compute
	Track::init();

	if (!isInitialized())
		return;

	// Track can change divider value as zoomfit is enabled
	divider_ = Track::divider_;

	// lat/lon to tile index
	vx1_ = floorf((float) pvx1_ / (float) TILESIZE);
	vy1_ = floorf((float) pvy1_ / (float) TILESIZE);

	vx2_ = floorf((float) pvx2_ / (float) TILESIZE);
	vy2_ = floorf((float) pvy2_ / (float) TILESIZE);

	// width x height of widget
	width = settings().width() - 2 * theme().border();
	height = settings().height() - 2 * theme().border();

	// compute padding for track
	padding_horizontal = theme().padding(VideoWidget::Theme::PaddingLeft) + theme().padding(VideoWidget::Theme::PaddingRight);
	padding_vertical = theme().padding(VideoWidget::Theme::PaddingTop) + theme().padding(VideoWidget::Theme::PaddingBottom);

	// Apply padding
	width_available = width - padding_horizontal;
	height_available = height - padding_vertical;

	switch (settings().view()) {
	// Apply zoom fit - center track
	case MapSettings::ViewZoomFit:
		// Append tile so as width tiles sum is enough
		while (((vx2_ - vx1_) * TILESIZE * divider_) < width) {
			vx1_ -= 1;
			vx2_ += 1;
		}

		// Append tile so as height tiles sum is enough
		while (((vy2_ - vy1_) * TILESIZE * divider_) < height) {
			vy1_ -= 1;
			vy2_ += 1;
		}
		break;

	// Center & lock view on the current position
	case MapSettings::ViewLockCenter:
		offsetx = theme().padding(VideoWidget::Theme::PaddingLeft) - theme().padding(VideoWidget::Theme::PaddingRight);
		offsety = theme().padding(VideoWidget::Theme::PaddingTop) - theme().padding(VideoWidget::Theme::PaddingBottom);

		space = ((width + offsetx) / 2.0) - ((pvx1_ - (vx1_ * TILESIZE)) * divider_);
		vx1_ -= std::max(0.0f, ceilf((space / divider_) / TILESIZE));

		space = ((width - offsetx) / 2.0) - ((TILESIZE - (pvx2_ - (vx2_ * TILESIZE))) * divider_);
		vx2_ += std::max(0.0f, ceilf(((space / divider_) / TILESIZE)));

		space = ((height + offsety) / 2.0) - ((pvy1_ - (vy1_ * TILESIZE)) * divider_);
		vy1_ -= std::max(0.0f, ceilf((space / divider_) / TILESIZE));

		space = ((height - offsety) / 2.0) - ((TILESIZE - (pvy2_ - (vy2_ * TILESIZE))) * divider_);
		vy2_ += std::max(0.0f, ceilf((space / divider_) / TILESIZE));

		break;

	// Default
	case MapSettings::ViewDefault:
	default:
		offsetx = theme().padding(VideoWidget::Theme::PaddingLeft) - theme().padding(VideoWidget::Theme::PaddingRight);
		offsety = theme().padding(VideoWidget::Theme::PaddingTop) - theme().padding(VideoWidget::Theme::PaddingBottom);

		// width x height of data area
		data_width = (lim_px2_ - lim_px1_) * divider_;
		data_height = (lim_py2_ - lim_py1_) * divider_;

		// Track fit in width
		if (data_width > width) {
			if ((width_available / 2) > (pvx1_ - lim_px1_))
				space = theme().padding(VideoWidget::Theme::PaddingLeft) + (((vx1_ * TILESIZE) - lim_px1_) * divider_);
			else if ((width_available / 2) > (lim_px2_ - pvx2_))
				space = (width - theme().padding(VideoWidget::Theme::PaddingRight)) - ((lim_px2_ - (vx1_ * TILESIZE)) * divider_);
			else
				space = (width / 2.0) - ((pvx1_ - (vx1_ * TILESIZE)) * divider_);
			vx1_ -= std::max(0.0f, ceilf((space / divider_) / TILESIZE));

			if ((width_available / 2) > (lim_px2_ - pvx2_))
				space = theme().padding(VideoWidget::Theme::PaddingRight) + ((lim_px2_ - ((vx2_ * TILESIZE) + TILESIZE)) * divider_);
			else if ((width_available / 2) > (pvx1_ - lim_px1_))
				space = (width - theme().padding(VideoWidget::Theme::PaddingLeft)) - ((((vx2_ * TILESIZE) + TILESIZE) - lim_px1_) * divider_);
			else
				space = (width / 2.0) - (TILESIZE - (pvx2_ - (vx2_ * TILESIZE)) * divider_);
			vx2_ += std::max(0.0f, ceilf(((space / divider_) / TILESIZE)));
		}
		else {
			space = ((width - data_width) / 2.0) + (((vx1_ * TILESIZE) - lim_px1_) * divider_);
			vx1_ -= std::max(0.0f, ceilf((space / divider_) / TILESIZE));

			space = ((width - data_width) / 2.0) + ((lim_px2_ - ((vx2_ * TILESIZE) + TILESIZE)) * divider_);
			vx2_ += std::max(0.0f, ceilf(((space / divider_) / TILESIZE)));
		}

		// Track fit in height
		if (data_height > height) {
			if ((height_available / 2) > (pvy1_ - lim_py1_))
				space = theme().padding(VideoWidget::Theme::PaddingTop) + (((vy1_ * TILESIZE) - lim_py1_) * divider_);
			else if ((height_available / 2) > (lim_py2_ - pvy2_))
				space = (height - theme().padding(VideoWidget::Theme::PaddingBottom)) - ((lim_py2_ - (vy1_ * TILESIZE)) * divider_);
			else
				space = (height / 2.0) - ((pvy1_ - (vy1_ * TILESIZE)) * divider_);
			vy1_ -= std::max(0.0f, ceilf((space / divider_) / TILESIZE));

			if ((height_available / 2) > (lim_py2_ - pvy2_))
				space = theme().padding(VideoWidget::Theme::PaddingBottom) + ((lim_py2_ - ((vy2_ * TILESIZE) + TILESIZE)) * divider_);
			else if ((height_available / 2) > (pvy1_ - lim_py1_))
				space = (height - theme().padding(VideoWidget::Theme::PaddingTop)) - ((((vy2_ * TILESIZE) + TILESIZE) - lim_py1_) * divider_);
			else
				space = (height / 2.0) - (TILESIZE - (pvy2_ - (vy2_ * TILESIZE)) * divider_);
			vy2_ += std::max(0.0f, ceilf((space / divider_) / TILESIZE));
		}
		else {
			space = ((height - data_height) / 2.0) + (((vy1_ * TILESIZE) - lim_py1_) * divider_);
			vy1_ -= std::max(0.0f, ceilf((space / divider_) / TILESIZE));

			space = ((height - data_height) / 2.0) + ((lim_py2_ - ((vy2_ * TILESIZE) + TILESIZE)) * divider_);
			vy2_ += std::max(0.0f, ceilf(((space / divider_) / TILESIZE)));
		}

		break;
	}

	// Drop old tiles
	while (!tiles_.empty()) {
		Tile *tile = tiles_.front();
		tiles_.pop_front();
		delete tile;
	}

	// Build each tile
	for (int y=vy1_; y<=vy2_; y++) {
		for (int x=vx1_; x<=vx2_; x++) {
			tile = new Tile(*this, zoom, x, y);
			tiles_.push_back(tile);
		}
	}

	// Donwload tile is required
	refresh_is_required_ = true;
}


void Map::download(void) {
	std::string uri;

	log_call();

	if (tiles_.empty() || !refresh_is_required_) {
		complete();

		goto done;
	}

	log_notice("Download map from %s (zoom: %d)...", 
			MapSettings::getFriendlyName(settings().source()).c_str(), 
			settings().zoom());

	nbr_downloads_ = 1;

	// Build & download each tile
	for (Tile *tile : tiles_) {
//		log_info("Download tile: %s", tile->uri().c_str());

		if (tile->download() == false)
			log_error("Download failure!");
	}

done:
	refresh_is_required_ = false;
}


void Map::build(void) {
	int fd;

	int width, height;

	const char *template_name = "/tmp/map-XXXXXX";

	log_call();

	log_notice("Build map...");

	// Map size
	width = (vx2_ - vx1_ + 1) * TILESIZE;
	height = (vy2_ - vy1_ + 1) * TILESIZE;

	// Build map
	if ((width > 0) && (height > 0)) {
		// Filename is output if user builds map/track
		if (app_.command() == GPXApplication::CommandMap) {
			filename_ = app_.settings().outputfile();
		}
		else if (filename_ == "") {
			char *s;

			// Make tmp filename
			s = strdup(template_name);
			fd = mkstemp(s);

			filename_ = s;

			close(fd);
			free(s);
		}

		// Create image buffer
		OIIO::ImageSpec outspec(width, height, 4);
		OIIO::ImageBuf image_buffer(outspec); 

		// Create map
		std::unique_ptr<OIIO::ImageOutput> out = OIIO::ImageOutput::create("map.png");

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
			img->read_image(img->current_subimage(), img->current_miplevel(), 0, -1, type, buf.localpixels());

			// Add alpha channel
			int channelorder[] = { 0, 1, 2, -1 /*use a float value*/ };
			float channelvalues[] = { 0 /*ignore*/, 0 /*ignore*/, 0 /*ignore*/, 1.0 };
			std::string channelnames[] = { "", "", "", "A" };

			OIIO::ImageBuf outbuf = OIIO::ImageBufAlgo::channels(buf, 4, channelorder, channelvalues, channelnames);

			// Write tile
			outbuf.specmod().x = (tile->x() - vx1_) * TILESIZE;
			outbuf.specmod().y = (tile->y() - vy1_) * TILESIZE;
			OIIO::ImageBufAlgo::over(image_buffer, outbuf, image_buffer, outbuf.roi());
		}

		// Write map file
		if (out->open(filename_, image_buffer.spec()) == false) {
			log_error("Build map failure, can't open '%s' file", filename_.c_str());
			goto error;
		}

		out->write_image(image_buffer.spec().format, image_buffer.localpixels());
		out->close();

		// User requests track draw
		if (app_.command() == GPXApplication::CommandTrack)
			draw();
	}
	else {
		log_warn("No data, can't build map");
	}

error:
	// Done
	complete();
}


void Map::draw(void) {
	std::string filename = "track.png";

	log_call();

	// Filename is output if user builds map/track
	if (app_.command() == GPXApplication::CommandTrack) {
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

	// Draw icons
	if (icon_start_buf_)
		icon(buf, *icon_start_buf_, x_start_, y_start_, OIIO::ROI());
	if (icon_end_buf_)
		icon(buf, *icon_end_buf_, x_end_, y_end_, OIIO::ROI());

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
	if (mapbuf_)
		return true;

	// Update track settings
	Track::setSettings(settings());

	if (Track::load() == false)
		return false;

	double divider = divider_;

	log_call();

	if (filename_.empty() || !trackbuf_)
		return true;

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
	img->read_image(img->current_subimage(), img->current_miplevel(), 0, -1, type, buf.localpixels());

	// Resize map
	mapbuf_ = new OIIO::ImageBuf(OIIO::ImageSpec(spec.width * divider, spec.height * divider, spec.nchannels, type)); //, OIIO::InitializePixels::No);
	OIIO::ImageBufAlgo::resize(*mapbuf_, buf);

	// Background track image over
	trackbuf_->specmod().x = (pevx1_ - (vx1_ * TILESIZE)) * divider_;
	trackbuf_->specmod().y = (pevy1_ - (vy1_ * TILESIZE)) * divider_;
	OIIO::ImageBufAlgo::over(*mapbuf_, *trackbuf_, *mapbuf_);

	return (mapbuf_ != NULL);
}


OIIO::ImageBuf * Map::render(const TelemetryData &data, bool &is_update) {
	bool is_move = true;

	int x, y;

	int w, width;
	int h, height;

	int width_available;
	int height_available;

	int data_width, data_height;

	int posX, posY;
	int offsetX, offsetY;

	int pos_lim_x1, pos_lim_y1;
	int pos_lim_x2, pos_lim_y2;

	int padding_vertical;
	int padding_horizontal;

	int zoom = settings().zoom();

	cairo_t *cairo;

	// Load map & track
	if (this->load() == false)
		log_warn("Map renderer failure");

//	// Check map & track buffers
//	if ((mapbuf_ == NULL) || (trackbuf_ == NULL)) {
//		is_update = false;
//		return NULL;
//	}

	// Refresh dynamic info
	if ((fg_buf_ != NULL) && (data.type() == TelemetryData::TypeUnchanged)) {
		is_update = false;
		goto skip;
	}

	// Compute position on map
	if (data.timestamp() > ts_end_) {
		posX = x_end_;
		posY = y_end_;
	}
	else if (data.timestamp() > ts_start_) {
		posX = floorf((float) Track::lon2pixel(zoom, data.longitude())) - pevx1_;
		posY = floorf((float) Track::lat2pixel(zoom, data.latitude())) - pevy1_;

		posX *= divider_;
		posY *= divider_;
	}
	else {
		posX = x_start_;
		posY = y_start_;
	}

	if ((last_posX_ != -1) && (last_posY_ != -1)) {
		// Smooth
		posX = (posX + last_posX_) / 2;
		posY = (posY + last_posY_) / 2;

		// Move ?
		is_move = !((posX == last_posX_) && (posY == last_posY_));

		if (!is_move) {
			if (settings().follow() != TrackSettings::FollowHeading) {
				is_update = false;
				goto skip;
			}
		}
	}

	// map position
	x = theme().border();
	y = theme().border();

	// width x height of track
	w = (pevx2_ - pevx1_) * divider_;
	h = (pevy2_ - pevy1_) * divider_;

	// width x height of widget
	width = settings().width() - 2 * theme().border();
	height = settings().height() - 2 * theme().border();

	// compute padding for track
	padding_horizontal = theme().padding(VideoWidget::Theme::PaddingLeft) + theme().padding(VideoWidget::Theme::PaddingRight);
	padding_vertical = theme().padding(VideoWidget::Theme::PaddingTop) + theme().padding(VideoWidget::Theme::PaddingBottom);

	// Apply padding
	width_available = width - padding_horizontal;
	height_available = height - padding_vertical;

	switch (settings().view()) {
	// Apply zoom fit - center track
	case MapSettings::ViewZoomFit:
		offsetX = theme().padding(VideoWidget::Theme::PaddingLeft) + (width_available - w) / 2;
		offsetY = theme().padding(VideoWidget::Theme::PaddingTop) + (height_available - h) / 2;

		break;

	// Center & lock view on the current position
	case MapSettings::ViewLockCenter:
		// Add offset to center on the current position
		offsetX = theme().padding(VideoWidget::Theme::PaddingLeft);
		offsetY = theme().padding(VideoWidget::Theme::PaddingTop);

		offsetX += (width_available / 2) - posX;
		offsetY += (height_available / 2) - posY;

		break;

	// Default mode
	case MapSettings::ViewDefault:
	default:
		// width x height of data area
		data_width = (lim_px2_ - lim_px1_) * divider_;
		data_height = (lim_py2_ - lim_py1_) * divider_;

		// Compute lim1 (top-left)
		pos_lim_x1 = (lim_px1_ - pevx1_) * divider_;
		pos_lim_y1 = (lim_py1_ - pevy1_) * divider_;

		// Compute lim2 (bottom-right)
		pos_lim_x2 = (lim_px2_ - pevx1_) * divider_;
		pos_lim_y2 = (lim_py2_ - pevy1_) * divider_;

		// Compute offset
		offsetX = theme().padding(VideoWidget::Theme::PaddingLeft);
		offsetY = theme().padding(VideoWidget::Theme::PaddingTop);

		offsetX += (width_available / 2) - posX;
		offsetY += (height_available / 2) - posY;

		if (data_width > width_available) {
			if ((posX - pos_lim_x1) < (width_available / 2))
				offsetX += posX - pos_lim_x1 - (width_available / 2);
			else if ((pos_lim_x2 - posX) < (width_available / 2))
				offsetX += posX - pos_lim_x2 + (width_available / 2);
		}
		else {
			offsetX = theme().padding(VideoWidget::Theme::PaddingLeft) + (width_available - w) / 2;
		}

		if (data_height > height_available) {
			if ((pos_lim_y2 - posY) < (height_available / 2))
				offsetY += posY - pos_lim_y2 + (height_available / 2);
			else if ((posY - pos_lim_y1) < (height_available / 2))
				offsetY += posY - pos_lim_y1 - (height_available / 2);
		}
		else {
			offsetY = theme().padding(VideoWidget::Theme::PaddingTop) + (height_available - h) / 2;
		}

		break;
	}

	// Image buffer
	if (fg_buf_ != NULL)
		delete fg_buf_;

	// Draw
	this->createBox(&fg_buf_, theme().width(), theme().height());

	// Cairo context
	cairo = this->createCairoContext(fg_buf_);

	// Draw
	draw(cairo, data);

	// Data bytes
	this->renderCairoContext(fg_buf_, cairo);

	// Release
	this->destroyCairoContext(cairo);

	if (mapbuf_ != NULL) {
		// Update path progress
		if (is_move) {
			mapbuf_->specmod().x = -(pevx1_ - (vx1_ * TILESIZE)) * divider_;
			mapbuf_->specmod().y = -(pevy1_ - (vy1_ * TILESIZE)) * divider_;
			path(*mapbuf_, data, divider_);
		}

		// Map & track image over
		mapbuf_->specmod().x = x + offsetX - ((pevx1_ - (vx1_ * TILESIZE)) * divider_);
		mapbuf_->specmod().y = y + offsetY - ((pevy1_ - (vy1_ * TILESIZE)) * divider_);
		OIIO::ImageBufAlgo::over(*fg_buf_, *mapbuf_, *fg_buf_, OIIO::ROI(x, x + width, y, y + height));

		// Draw picto
		if (icon_start_buf_ && theme().hasFlag(VideoWidget::Theme::FlagIconStart))
			icon(*fg_buf_, *icon_start_buf_, x + offsetX + x_start_, y + offsetY + y_start_, OIIO::ROI(x, x + width, y, y + height));
		if (icon_end_buf_ && theme().hasFlag(VideoWidget::Theme::FlagIconEnd))
			icon(*fg_buf_, *icon_end_buf_, x + offsetX + x_end_, y + offsetY + y_end_, OIIO::ROI(x, x + width, y, y + height));
	
		if (icon_position_buf_ && data.hasValue(TelemetryData::DataFix) && theme().hasFlag(VideoWidget::Theme::FlagIconPosition)) {
			if (settings().follow() != TrackSettings::FollowNone) {
				double angle = (settings().follow() == TrackSettings::FollowCourse) ? data.course() : data.heading();

				OIIO::ImageBuf position = OIIO::ImageBufAlgo::rotate(*icon_position_buf_, angle * M_PI / 180.0);

				icon(*fg_buf_, position, x + offsetX + posX, y + offsetY + posY, OIIO::ROI(x, x + width, y, y + height));
			}
			else 
				icon(*fg_buf_, *icon_position_buf_, x + offsetX + posX, y + offsetY + posY, OIIO::ROI(x, x + width, y, y + height));
		}
	}

	// Save last position
	last_posX_ = posX;
	last_posY_ = posY;

	is_update = true;
skip:
	return fg_buf_;
}


bool Map::updated(const TelemetryData &data) const {
	return Track::updated(data);
}


void Map::draw(cairo_t *cr, const TelemetryData &data) {
	(void) data;

	// Draw
	background(cr);
}


void Map::clear(void) {
	Track::clear();

	if (bg_buf_)
		delete bg_buf_;

	if (fg_buf_)
		delete fg_buf_;

	if (mapbuf_)
		delete mapbuf_;

	bg_buf_ = NULL;
	fg_buf_ = NULL;
	mapbuf_ = NULL;
}


void Map::downloadProgress(Map::Tile &tile, curl_off_t dltotal, curl_off_t dlnow) {
	char buf[64];
	const char *label = "Download tile";

	time_t now = time(NULL);

	Map& map = tile.map();

	int percent = (dltotal > 0) ? (int) (dlnow * 100 / dltotal) : 0;

	memset(buf, '.', 50);          
	memset(buf, '#', percent / 2); 
	buf[50] = '\0';

	if (percent == 100) 
		printf("\r  %s %d / %d [%s] DONE      ",          
				label, map.nbr_downloads_, (unsigned int) map.tiles_.size(), buf);
	else if (tile.last_update_ <= now + 1) {
		printf("\r  %s %d / %d [%s] %3d %%",
				label, map.nbr_downloads_, (unsigned int) map.tiles_.size(), buf, percent);

		tile.last_update_ = now;
	}
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

	last_update_ = 0;

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


int Map::Tile::downloadProgress(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
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

	Utils::mkpath(path_, 0700);

	// Check if file exists in cache
	if (access(output.c_str(), F_OK) == 0) {
		// Check file content
		stat(output.c_str(), &st);

		if (st.st_size > 0) {
			Map::downloadComplete(*this);
			return true;
		}
	}

	// Downloading?
	if (evtaskh_ != NULL)
		return true;
	
	// Download
	evtaskh_ = map_.evcurl()->download(uri_.c_str(), downloadComplete, this);

//	evtaskh_->setOption(CURLOPT_VERBOSE, 1L);
//	evtaskh_->setOption(CURLOPT_DEBUGFUNCTION, downloadDebug);
//	evtaskh_->setOption(CURLOPT_DEBUGDATA, this);

	evtaskh_->setOption(CURLOPT_NOPROGRESS, 0L);
	evtaskh_->setOption(CURLOPT_XFERINFOFUNCTION, downloadProgress);
	evtaskh_->setOption(CURLOPT_XFERINFODATA, this);
	
	evtaskh_->setOption(CURLOPT_WRITEFUNCTION, downloadWrite);
	evtaskh_->setOption(CURLOPT_WRITEDATA, this);

	evtaskh_->setOption(CURLOPT_FOLLOWLOCATION, 1L);

	evtaskh_->setHeader("User-Agent: gpx2video");

	evtaskh_->perform();

	return true;
}

