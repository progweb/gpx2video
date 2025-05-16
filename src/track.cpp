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


Track::Track(GPXApplication &app, const TelemetrySettings &telemetry_settings, const TrackSettings &track_settings, struct event_base *evbase)
	: VideoWidget(app, "track")
	, app_(app)
	, track_settings_(track_settings)
	, telemetry_settings_(telemetry_settings)
	, evbase_(evbase) {
	log_call();

	VideoWidget::setFlags(VideoWidget::FlagNone);
	VideoWidget::setSize(settings().width(), settings().height());

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


const TrackSettings& Track::settings() const {
	log_call();

	return track_settings_;
}


Track * Track::create(GPXApplication &app, const TelemetrySettings &telemetry_settings, const TrackSettings &track_settings) {
	Track *track;

	log_call();

	track = new Track(app, telemetry_settings, track_settings, app.evbase());

	return track;
}


void Track::setSize(int width, int height) {
	VideoWidget::setSize(width, height); 

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


void Track::init(bool zoomfit) {
	int zoom;
	int padding;

	double lat1, lon1;
	double lat2, lon2;

	int w, h;
	int width = settings().width(); // 800;
	int height = settings().height(); // 500;

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
	padding = this->border() + settings().markerSize();

	width -= 2 * padding;
	height -= 2 * padding;

	if (zoomfit) {
		double divider;

		// Compute divider to match with the size of widget
		w = ceilf((float) px2_ - px1_);
		h = ceilf((float) py2_ - py1_);

		if (((float) width / w) > ((float) height / h))
			divider = (float) height / h;
		else
			divider = (float) width / w;

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
	unsigned char *data;

	int x = 0, y = 0;

	TelemetryData wpt;

	enum TelemetrySource::Data result;

	log_call();

	zoom = settings().zoom();
	path_thick = settings().pathThick();
	path_border = settings().pathBorder();

	// Cairo buffer
	OIIO::ImageBuf buf(outbuf.spec());

	// Create the cairo destination surface
	cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, outbuf.spec().width, outbuf.spec().height);

	// Cairo context
	cairo_t *cairo = cairo_create(surface);

	// Path border
	if (path_border > 0) {
		cairo_set_source_rgb(cairo, 0.0, 0.0, 0.0); // BGR #000000
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

	// Path color
	cairo_set_source_rgb(cairo, 0.9, 0.4, 0.2); // BGR #669df6
	cairo_set_line_width(cairo, path_thick); //3.0); //40.96);
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

	// Create track buffer
	trackbuf_ = new OIIO::ImageBuf(OIIO::ImageSpec(width * divider_, height * divider_, 4, OIIO::TypeDesc::UINT8)); //, OIIO::InitializePixels::No);

	TelemetrySource *source = TelemetryMedia::open(filename, telemetry_settings_);

	if (source != NULL) {
//		// Telemetry data limits
//		source->setFrom(app_.settings().from());
//		source->setTo(app_.settings().to());
		//gpx->setTimeOffset(app_.settings().offset());

		// Draw path
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

	return (trackbuf_ != NULL);
}


OIIO::ImageBuf * Track::prepare(bool &is_update) {
	if (bg_buf_ != NULL) {
		is_update = false;
		goto skip;
	}

	this->createBox(&bg_buf_, this->width(), this->height());
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
		log_warn("Track renderer failure");
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

	// Image buffer
	if (fg_buf_ != NULL)
		delete fg_buf_;

	this->createBox(&fg_buf_, this->width(), this->height());

	// Image over
	trackbuf_->specmod().x = x - offsetX;
	trackbuf_->specmod().y = y - offsetY;
	OIIO::ImageBufAlgo::over(*fg_buf_, *trackbuf_, *fg_buf_, OIIO::ROI(x, x + width, y, y + height));

	// Draw track
	// ...

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


