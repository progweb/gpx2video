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
#include "oiioutils.h"
#include "videoparams.h"
#include "telemetrymedia.h"
#include "track.h"


#define TILESIZE 256



TrackSettings::TrackSettings() {
	width_ = 320;
	height_ = 240;
	zoom_ = 12;
	marker_size_ = 60;

	path_thick_ = 3.0;
	path_border_ = 1.4;

	setPathBorderColor(0.0, 0.0, 0.0, 1.0);
	setPathPrimaryColor(0.9, 0.4, 0.2, 1.0);
	setPathSecondaryColor(1.0, 1.0, 1.0, 1.0);
}


TrackSettings::~TrackSettings() {
}


const int& TrackSettings::width(void) const {
	return width_;
}


const int& TrackSettings::height(void) const {
	return height_;
}


void TrackSettings::setSize(const int &width, const int &height) {
	width_ = width;
	height_ = height;
}


const int& TrackSettings::zoom(void) const {
	return zoom_;
}


void TrackSettings::setZoom(const int &zoom) {
	zoom_ = zoom;
}


const int& TrackSettings::markerSize(void) const {
	return marker_size_;
}


void TrackSettings::setMarkerSize(const int &size) {
	marker_size_ = size;
}


const double& TrackSettings::pathThick(void) const {
	return path_thick_;
}


void TrackSettings::setPathThick(const double &thick) {
	path_thick_ = thick;
}


const double& TrackSettings::pathBorder(void) const {
	return path_border_;
}


void TrackSettings::setPathBorder(const double &border) {
	path_border_ = border;
}


const float * TrackSettings::pathBorderColor(void) const {
	return path_border_color_;
}


bool TrackSettings::setPathBorderColor(std::string color) {
	return VideoWidget::Theme::hex2color(path_border_color_, color);
}


bool TrackSettings::setPathBorderColor(double r, double g, double b, double a) {
	path_border_color_[0] = r;
	path_border_color_[1] = g;
	path_border_color_[2] = b;
	path_border_color_[3] = a;
	return true;
}


const float * TrackSettings::pathPrimaryColor(void) const {
	return path_primary_color_;
}


bool TrackSettings::setPathPrimaryColor(std::string color) {
	return VideoWidget::Theme::hex2color(path_primary_color_, color);
}


bool TrackSettings::setPathPrimaryColor(double r, double g, double b, double a) {
	path_primary_color_[0] = r;
	path_primary_color_[1] = g;
	path_primary_color_[2] = b;
	path_primary_color_[3] = a;
	return true;
}


const float * TrackSettings::pathSecondaryColor(void) const {
	return path_secondary_color_;
}


bool TrackSettings::setPathSecondaryColor(std::string color) {
	return VideoWidget::Theme::hex2color(path_secondary_color_, color);
}


bool TrackSettings::setPathSecondaryColor(double r, double g, double b, double a) {
	path_secondary_color_[0] = r;
	path_secondary_color_[1] = g;
	path_secondary_color_[2] = b;
	path_secondary_color_[3] = a;
	return true;
}


void TrackSettings::getBoundingBox(double *lat1, double *lon1, double *lat2, double *lon2) const {
	*lat1 = lat1_;
	*lon1 = lon1_;

	*lat2 = lat2_;
	*lon2 = lon2_;
}


void TrackSettings::setBoundingBox(double lat1, double lon1, double lat2, double lon2) {
	lat1_ = lat1;
	lon1_ = lon1;

	lat2_ = lat2;
	lon2_ = lon2;
}


Track::Track(GPXApplication &app, const TelemetrySettings &telemetry_settings, const TrackSettings &track_settings, VideoWidget::Widget type, struct event_base *evbase)
	: VideoWidget(app, type)
	, ShapeBase(VideoWidget::theme())
	, app_(app)
	, track_settings_(track_settings)
	, telemetry_settings_(telemetry_settings)
	, evbase_(evbase) {
	log_call();

	setShape(VideoWidget::ShapeNone);

	theme().setFlags(VideoWidget::Theme::FlagNone);
	theme().setSize(settings().width(), settings().height());

	bg_buf_ = NULL;
	fg_buf_ = NULL;
	trackbuf_ = NULL;

	divider_ = 1.0;

	x1_ = y1_ = x2_ = y2_ = 0;
	px1_ = py1_ = px2_ = py2_ = 0;

	x_end_ = y_end_ = 0;
	x_start_ = y_start_ = 0;
	ts_start_ = ts_end_ = 0;
}


Track::~Track() {
	log_call();

	if (trackbuf_ != NULL)
		delete trackbuf_;
	if (bg_buf_)
		delete bg_buf_;
	if (fg_buf_)
		delete fg_buf_;
}


TrackSettings& Track::settings() {
	log_call();

	return track_settings_;
}


const TrackSettings& Track::settings() const {
	log_call();

	return track_settings_;
}


void Track::setSettings(const TrackSettings &settings) {
	log_call();

	track_settings_ = settings;
}


Track * Track::create(GPXApplication &app, const TelemetrySettings &telemetry_settings, const TrackSettings &track_settings) {
	Track *track;

	log_call();

	track = new Track(app, telemetry_settings, track_settings, VideoWidget::WidgetTrack, app.evbase());

	return track;
}


void Track::setSize(int width, int height) {
	theme().setSize(width, height); 

	track_settings_.setSize(width, height);
}


int Track::lat2pixel(int zoom, float lat) {
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


int Track::lon2pixel(int zoom, float lon) {
    int pixel_x;

    double lonrad = lon * M_PI / 180.0;

    // the formula is
    //
    // pixel_x = (2^zoom * TILESIZE * lon) / 2PI + (2^zoom * TILESIZE) / 2
    pixel_x = (int)(( lonrad * TILESIZE * (1 << zoom) ) / (2*M_PI)) +
        ( (1 << zoom) * (TILESIZE/2) );

    return pixel_x;
}


bool Track::preinit(void) {
	TelemetrySource *source;
   
	log_call();

	// Init
	x1_ = y1_ = x2_ = y2_ = 0;
	px1_ = py1_ = px2_ = py2_ = 0;

	last_data_ = TelemetryData();

	// Open telemetry data file
	source = TelemetryMedia::open(app_.settings().inputfile(), telemetry_settings_, true);

	if (source == NULL) {
		log_warn("Can't read telemetry data, skip %s widget initialization", name().c_str());
		return false;
	}

	// Create map bounding box
	TelemetryData p1, p2;
	source->getBoundingBox(&p1, &p2);

	// Track settings
	track_settings_.setBoundingBox(p1.latitude(), p1.longitude(), p2.latitude(), p2.longitude());

	// Delete previous track buffer
	if (trackbuf_ != NULL) {
		delete trackbuf_;
		trackbuf_ = NULL;
	}

	return true;
}


void Track::init(bool zoomfit) {
	int zoom;
	int padding;

	double lat1, lon1;
	double lat2, lon2;

	int w, h;
	int width = settings().width(); // 800;
	int height = settings().height(); // 500;

	log_call();

	if (!preinit()) {
		log_warn("Track init failure!");
		return;
	}

	// Compute track area
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

	// lat/lon to pixel
	px1_ = Track::lon2pixel(zoom, lon1);
	py1_ = Track::lat2pixel(zoom, lat1);

	px2_ = Track::lon2pixel(zoom, lon2);
	py2_ = Track::lat2pixel(zoom, lat2);

	// lat/lon to tile index
	x1_ = floor((float) px1_ / (float) TILESIZE);
	y1_ = floorf((float) py1_ / (float) TILESIZE);

	x2_ = ceilf((float) px2_ / (float) TILESIZE);
	y2_ = ceilf((float) py2_ / (float) TILESIZE);

	// Use padding (to see markers)
	padding = theme().border() + settings().markerSize();

	width -= 2 * padding;
	height -= 2 * padding;

	if (zoomfit) {
		double divider = 1.0;

		// Compute divider to match with the size of widget
		w = ceilf((float) px2_ - px1_);
		h = ceilf((float) py2_ - py1_);

		if ((w > 0) && (h > 0)) {
			if (((float) width / w) > ((float) height / h))
				divider = (float) height / h;
			else
				divider = (float) width / w;
		}

		// Save computed divider value
		divider_ = divider;
	}

	// Append tile so as padding is enough
	while (((px1_ - (x1_ * TILESIZE)) * divider_) < padding)
		x1_ -= 1;

	while (((py1_ - (y1_ * TILESIZE)) * divider_) < padding)
		y1_ -= 1;

	while ((((x2_ * TILESIZE) - px2_) * divider_) < padding)
		x2_ += 1;

	while ((((y2_ * TILESIZE) - py2_) * divider_) < padding)
		y2_ += 1;

	// Append tile so as width tiles sum is enough
	while (((x2_ - x1_) * TILESIZE * divider_) < (2 * width)) {
		x1_ -= 1;
		x2_ += 1;
	}

	// Append tile so as height tiles sum is enough
	while (((y2_ - y1_) * TILESIZE * divider_) < (2 * height)) {
		y1_ -= 1;
		y2_ += 1;
	}
}


void Track::path(OIIO::ImageBuf &outbuf, TelemetrySource *source, double divider) {
	int zoom;
	int stride;
	double path_thick;
	double path_border;
	unsigned char *bytes;

	const float *fill;
	const float *outline;

	int x = 0, y = 0;

	TelemetryData wpt;

	enum TelemetrySource::Data result;

	log_call();

	zoom = settings().zoom();
	path_thick = settings().pathThick();
	path_border = settings().pathBorder();

	fill = settings().pathSecondaryColor();
	outline = settings().pathBorderColor();

	// Cairo buffer
	OIIO::ImageBuf buf(outbuf.spec());

	// Create the cairo destination surface
	cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, outbuf.spec().width, outbuf.spec().height);

	// Cairo context
	cairo_t *cairo = cairo_create(surface);

	// Path border
	if (path_border > 0) {
//		cairo_set_source_rgb(cairo, 0.0, 0.0, 0.0); // BGR #000000
		cairo_set_source_rgba(cairo, outline[0], outline[1], outline[2], outline[3]); // BGR #0000000
		cairo_set_line_width(cairo, path_border + path_thick); //4.4); //40.96);
		cairo_set_line_join(cairo, CAIRO_LINE_JOIN_ROUND);

		// Draw each WPT
		for (result = source->retrieveFirst(wpt); result != TelemetrySource::DataEof; result = source->retrieveNext(wpt)) {
			x = floorf((float) Track::lon2pixel(zoom, wpt.longitude())) - (x1_ * TILESIZE);
			y = floorf((float) Track::lat2pixel(zoom, wpt.latitude())) - (y1_ * TILESIZE);

			x *= divider;
			y *= divider;

			cairo_line_to(cairo, x, y);
		}

		// Cairo draw
		cairo_stroke(cairo);
	}

	// Draw each WPT with secondary color
//	cairo_set_source_rgb(cairo, 0.9, 0.4, 0.2); // BGR #669df6
	cairo_set_source_rgba(cairo, fill[0], fill[1], fill[2], fill[3]); // BGR #669df600
	cairo_set_line_width(cairo, path_thick); //3.0); //40.96);
	cairo_set_line_join(cairo, CAIRO_LINE_JOIN_ROUND);

	for (result = source->retrieveFirst(wpt); result != TelemetrySource::DataEof; result = source->retrieveNext(wpt)) {
		x = floorf((float) Track::lon2pixel(zoom, wpt.longitude())) - (x1_ * TILESIZE);
		y = floorf((float) Track::lat2pixel(zoom, wpt.latitude())) - (y1_ * TILESIZE);

		x *= divider;
		y *= divider;

		cairo_line_to(cairo, x, y);
	}

	cairo_stroke(cairo);

	// Convert to image
	bytes = cairo_image_surface_get_data(surface);
	stride = cairo_image_surface_get_stride(surface);

	// Cairo to OIIO
	buf.set_pixels(OIIO::ROI(),
		buf.spec().format,
		bytes, 
		OIIO::AutoStride,
		stride);

	// BGRA => RGBA
	int channelorder[] = { 2, 1, 0, 3 };
	float channelvalues[] = { };
	std::string channelnames[] = { "B", "G", "R", "A" };

	OIIO::ImageBufAlgo::channels(buf, buf, 4, channelorder, channelvalues, channelnames);

	// Cairo over
	OIIO::ImageBufAlgo::over(outbuf, buf, outbuf);

	// Release
	cairo_surface_destroy(surface);
	cairo_destroy(cairo);
}


void Track::path(OIIO::ImageBuf &outbuf, const TelemetryData &data, double divider) {
	int zoom;
	int stride;
	double path_thick;
	unsigned char *bytes;

	const float *fill;

	int x = 0, y = 0;

	int xoff = 0;
	int yoff = 0;

	int channelorder[] = { 2, 1, 0, 3 };
	float channelvalues[] = { };
	std::string channelnames[] = { "B", "G", "R", "A" };

	OIIO::ROI roi = OIIO::ROI::All();
	OIIO::ImageBuf buf;

	cairo_t *cairo = NULL;
	cairo_surface_t *surface = NULL;

	TelemetryData wpt;

	enum TelemetrySource::Data result;

	log_call();

	zoom = settings().zoom();
	path_thick = settings().pathThick();

	fill = settings().pathPrimaryColor();

	// Start or continue path ?
	if (last_data_.type() == TelemetryData::TypeUnknown) {
		std::string filename = app_.settings().inputfile();

		TelemetrySource *source = TelemetryMedia::open(filename, telemetry_settings_, true);

		// Cairo buffer
		buf = OIIO::ImageBuf(outbuf.spec());

		// Create the cairo destination surface
		surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, outbuf.spec().width, outbuf.spec().height);

		// Cairo context
		cairo = cairo_create(surface);

		// Draw new WPT with primary color
		cairo_set_source_rgba(cairo, fill[0], fill[1], fill[2], fill[3]); // BGR #669df600
		cairo_set_line_width(cairo, path_thick); //3.0); //40.96);
		cairo_set_line_join(cairo, CAIRO_LINE_JOIN_ROUND);

		for (result = source->retrieveFirst(wpt); result != TelemetrySource::DataEof; result = source->retrieveNext(wpt)) {
			x = floorf((float) Track::lon2pixel(zoom, wpt.longitude())) - (x1_ * TILESIZE);
			y = floorf((float) Track::lat2pixel(zoom, wpt.latitude())) - (y1_ * TILESIZE);

			x *= divider;
			y *= divider;

			cairo_line_to(cairo, x, y);

			if (wpt.timestamp() >= data.timestamp())
				break;
		}

		cairo_stroke(cairo);
	}
	else {
		int x1, y1;
		int x2, y2;

		int width, height;

		const OIIO::ImageSpec& spec = outbuf.spec();

		OIIO::TypeDesc::BASETYPE type = (OIIO::TypeDesc::BASETYPE) spec.format.basetype;

		// Last point
		x1 = floorf((float) Track::lon2pixel(zoom, last_data_.longitude())) - (x1_ * TILESIZE);
		y1 = floorf((float) Track::lat2pixel(zoom, last_data_.latitude())) - (y1_ * TILESIZE);

		x1 *= divider;
		y1 *= divider;

		// Current point
		x2 = floorf((float) Track::lon2pixel(zoom, data.longitude())) - (x1_ * TILESIZE);
		y2 = floorf((float) Track::lat2pixel(zoom, data.latitude())) - (y1_ * TILESIZE);

		x2 *= divider;
		y2 *= divider;

		// Move ?
		if ((x1 == x2) && (y1 == y2))
			goto skip;

		// Move segment at origin
		xoff = std::min(x1, x2);
		yoff = std::min(y1, y2);

		x1 = x1 - xoff + path_thick;
		x2 = x2 - xoff + path_thick;
		y1 = y1 - yoff + path_thick;
		y2 = y2 - yoff + path_thick;

		// Compute area change
		width = std::max(x1, x2) + path_thick;
		height = std::max(y1, y2) + path_thick;

		// Cairo buffer
		buf = OIIO::ImageBuf(OIIO::ImageSpec(width, height, spec.nchannels, type));

		// Create the cairo destination surface
		surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);

		// Cairo context
		cairo = cairo_create(surface);

		cairo_save(cairo);
		cairo_set_source_rgba(cairo, fill[0], fill[1], fill[2], fill[3]); // BGR #669df600
		cairo_set_line_width(cairo, path_thick); //3.0); //40.96);
		cairo_set_line_join(cairo, CAIRO_LINE_JOIN_ROUND);
		cairo_move_to(cairo, x1, y1);
		cairo_line_to(cairo, x2, y2);
		cairo_stroke(cairo);
		cairo_restore(cairo);

		// Overlay offset
		xoff -= path_thick;
		yoff -= path_thick;

		roi = OIIO::ROI(xoff, xoff + width, yoff, yoff + height);
	}

	// Convert to image
	bytes = cairo_image_surface_get_data(surface);
	stride = cairo_image_surface_get_stride(surface);

	// Cairo to OIIO
	buf.set_pixels(OIIO::ROI(),
		buf.spec().format,
		bytes, 
		OIIO::AutoStride,
		stride);

	// BGRA => RGBA 
	OIIO::ImageBufAlgo::channels(buf, buf, 4, channelorder, channelvalues, channelnames);

	// Cairo over
	buf.specmod().x = xoff;
	buf.specmod().y = yoff;
	OIIO::ImageBufAlgo::over(outbuf, buf, outbuf, roi);

	// Release
	cairo_surface_destroy(surface);
	cairo_destroy(cairo);

	// Save current position
	last_data_ = data;

skip:
	return;
}


bool Track::load(void) {
	if (trackbuf_)
		return true;

	int width = settings().width();
	int height = settings().height();

	int zoom = settings().zoom();

	std::string filename = app_.settings().inputfile();

	TelemetryData wpt;

	log_call();

	// Compute track size
	width = (x2_ - x1_) * TILESIZE;
	height = (y2_ - y1_) * TILESIZE;

	if ((width == 0) || (height == 0))
		return true;

	// Create background track buffer
	trackbuf_ = new OIIO::ImageBuf(OIIO::ImageSpec(width * divider_, height * divider_, 4, OIIO::TypeDesc::UINT8)); //, OIIO::InitializePixels::No);

	TelemetrySource *source = TelemetryMedia::open(filename, telemetry_settings_, true);

	if (source != NULL) {
//		// Telemetry data limits
//		source->setFrom(app_.settings().from());
//		source->setTo(app_.settings().to());
		//gpx->setTimeOffset(app_.settings().offset());

		// Draw background path
		path(*trackbuf_, source, divider_);

		// Compute begin
		source->retrieveFirst(wpt);

		x_start_ = floorf((float) Track::lon2pixel(zoom, wpt.longitude())) - (x1_ * TILESIZE);
		y_start_ = floorf((float) Track::lat2pixel(zoom, wpt.latitude())) - (y1_ * TILESIZE);

		x_start_ *= divider_;
		y_start_ *= divider_;
		ts_start_ = source->beginTimestamp();

		// Compute end
		source->retrieveLast(wpt);

		x_end_ = floorf((float) Track::lon2pixel(zoom, wpt.longitude())) - (x1_ * TILESIZE);
		y_end_ = floorf((float) Track::lat2pixel(zoom, wpt.latitude())) - (y1_ * TILESIZE);

		x_end_ *= divider_;
		y_end_ *= divider_;
		ts_end_ = source->endTimestamp();
	}
	else {
		log_warn("Can't open '%s' telemetry data file", filename.c_str());
	}

	if (source != NULL)
		delete source;

	// Init
	last_data_ = TelemetryData();

	return (trackbuf_ != NULL);
}


OIIO::ImageBuf * Track::prepare(bool &is_update) {
	if (bg_buf_ != NULL) {
		is_update = false;
		goto skip;
	}

	this->createBox(&bg_buf_, theme().width(), theme().height());
	this->drawBorder(bg_buf_);
	this->drawBackground(bg_buf_);

	if (this->load() == false)
		log_warn("Track renderer failure");

	is_update = true;

skip:
	return bg_buf_;
}


OIIO::ImageBuf * Track::render(const TelemetryData &data, bool &is_update) {
	int x = 0; //this->x();
	int y = 0; //this->y();
	int w, width = settings().width();
	int h, height = settings().height();

	int posX, posY;
	int offsetX, offsetY;

	int zoom = settings().zoom();
	int marker_size = settings().markerSize();

	// Check track buffer
	if (trackbuf_ == NULL) {
		is_update = false;
		return NULL;
	}

	// Refresh dynamic info
	if ((fg_buf_ != NULL) && (data.type() == TelemetryData::TypeUnchanged)) {
		is_update = false;
		goto skip;
	}

	// Current position
	if (data.timestamp() > ts_end_) {
		posX = x_end_;
		posY = y_end_;
	}
	else if (data.timestamp() > ts_start_) {
		posX = floorf((float) Track::lon2pixel(zoom, data.longitude())) - (x1_ * TILESIZE);
		posY = floorf((float) Track::lat2pixel(zoom, data.latitude())) - (y1_ * TILESIZE);

		posX *= divider_;
		posY *= divider_;
	}
	else {
		posX = x_start_;
		posY = y_start_;
	}

	// width x height of track
	w = (px2_ - px1_) * divider_; // floorf((float) Track::lon2pixel(zoom, lon1)) - (x1_ * TILESIZE);
	h = (py2_ - py1_) * divider_; // floorf((float) Track::lat2pixel(zoom, lat1)) - (y1_ * TILESIZE);

	// Center track
	offsetX = px1_ - (x1_ * TILESIZE);
	offsetX *= divider_;
	offsetX -= (width - w) / 2;

	offsetY = py1_ - (y1_ * TILESIZE);
	offsetY *= divider_;
	offsetY -= (height - h) / 2;

	// Update path progress
	trackbuf_->specmod().x = 0;
	trackbuf_->specmod().y = 0;
	path(*trackbuf_, data, divider_);

	// Image buffer
	if (fg_buf_ != NULL)
		delete fg_buf_;

	// Draw
	this->createBox(&fg_buf_, theme().width(), theme().height());

	// Draw track image over
	trackbuf_->specmod().x = x - offsetX;
	trackbuf_->specmod().y = y - offsetY;
	OIIO::ImageBufAlgo::over(*fg_buf_, *trackbuf_, *fg_buf_, OIIO::ROI(x, x + width, y, y + height));

	// Draw picto
	if (marker_size > 0) {
		drawPicto(*fg_buf_, x - offsetX + x_end_, y - offsetY + y_end_, OIIO::ROI(x, x + width, y, y + height), "./assets/marker/end.png", marker_size);
		drawPicto(*fg_buf_, x - offsetX + x_start_, y - offsetY + y_start_, OIIO::ROI(x, x + width, y, y + height), "./assets/marker/start.png", marker_size);
	
		if (data.hasValue(TelemetryData::DataFix))
			drawPicto(*fg_buf_, x - offsetX + posX, y - offsetY + posY, OIIO::ROI(x, x + width, y, y + height), "./assets/marker/position.png", marker_size);
	}

	is_update = true;

skip:
	return fg_buf_;
}


void Track::clear(void) {
	if (trackbuf_)
		delete trackbuf_;

	if (bg_buf_)
		delete bg_buf_;

	if (fg_buf_)
		delete fg_buf_;

	bg_buf_ = NULL;
	fg_buf_ = NULL;
	trackbuf_ = NULL;
}


bool Track::drawPicto(OIIO::ImageBuf &map, int x, int y, OIIO::ROI roi, const char *picto, int size) {
	bool result;

	double divider;

	// Open picto
	auto img = OIIO::ImageInput::open(picto);
	const OIIO::ImageSpec& spec = img->spec();
	OIIO::TypeDesc::BASETYPE type = (OIIO::TypeDesc::BASETYPE) spec.format.basetype;

	OIIO::ImageBuf buf = OIIO::ImageBuf(OIIO::ImageSpec(spec.width, spec.height, spec.nchannels, type));
	img->read_image(img->current_subimage(), img->current_miplevel(), 0, -1, type, buf.localpixels());

	// Compute divider
	divider = (double) size / (double) spec.height;

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


