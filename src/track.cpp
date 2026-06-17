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

#include "log_i.h"
#include "utils.h"
#include "oiioutils.h"
#include "videoparams.h"
#include "telemetrymedia.h"
#include "track.h"


#define TILESIZE 256



TrackSettings::TrackSettings() {
	width_ = 320;
	height_ = 240;

	null_ = 0.0;

	zoom_ = 18;
	view_ = TrackSettings::ViewZoomFit;

	divider_ = 1.0;

	path_thick_ = 3.0;
	path_border_ = 1.4;
	path_smooth_ = 1;

	setPathBorderColor(0.0, 0.0, 0.0, 1.0);
	setPathPrimaryColor(0.9, 0.4, 0.2, 1.0);
	setPathSecondaryColor(1.0, 1.0, 1.0, 1.0);

	setFollow(TrackSettings::FollowNone);

	setIcon(TrackSettings::IconStart, TrackSettings::IconDefault);
	setIconColor(TrackSettings::IconStart, 0.0, 0.0, 0.0, 0.0);
	setIconSize(TrackSettings::IconStart, 1.0);

	setIcon(TrackSettings::IconEnd, TrackSettings::IconDefault);
	setIconColor(TrackSettings::IconEnd, 0.0, 0.0, 0.0, 0.0);
	setIconSize(TrackSettings::IconEnd, 1.0);

	setIcon(TrackSettings::IconPosition, TrackSettings::IconDefault);
	setIconColor(TrackSettings::IconPosition, 0.0, 0.0, 0.0, 0.0);
	setIconSize(TrackSettings::IconPosition, 1.0);
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


const TrackSettings::View& TrackSettings::view(void) const {
	return view_;
}


void TrackSettings::setView(const TrackSettings::View &view) {
	view_ = view;
}


const double& TrackSettings::divider(void) const {
	return divider_;
}


void TrackSettings::setDivider(const double &divider) {
	divider_ = divider;
}


const int& TrackSettings::pathSmooth(void) const {
	return path_smooth_;
}


void TrackSettings::setPathSmooth(const int &smooth) {
	path_smooth_ = smooth;
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


const TrackSettings::Follow& TrackSettings::follow(void) const {
	return follow_;
}


void TrackSettings::setFollow(const TrackSettings::Follow &follow) {
	follow_ = follow;
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


TrackSettings::View TrackSettings::string2view(std::string &s) {
	TrackSettings::View view;

	if (s.empty() || (s == "default"))
		view = TrackSettings::ViewDefault;
	else if (s == "center")
		view = TrackSettings::ViewLockCenter;
	else if (s == "zoomfit")
		view = TrackSettings::ViewZoomFit;
	else 
		view = TrackSettings::ViewUnknown;

	return view;
}


TrackSettings::Follow TrackSettings::string2follow(std::string &s) {
	TrackSettings::Follow follow;

	if (s.empty() || (s == "none"))
		follow = TrackSettings::FollowNone;
	else if (s == "course")
		follow = TrackSettings::FollowCourse;
	else if (s == "heading")
		follow = TrackSettings::FollowHeading;
	else 
		follow = TrackSettings::FollowUnknown;

	return follow;
}


TrackSettings::Icon TrackSettings::string2icon(std::string &s) {
	TrackSettings::Icon icon;

	if (s.empty() || (s == "default"))
		icon = TrackSettings::IconDefault;
	// Internal type icons
	else if (s == "internal:play")
		icon = TrackSettings::IconPlay;
	else if (s == "internal:stop")
		icon = TrackSettings::IconStop;
	else if (s == "internal:position")
		icon = TrackSettings::IconPosition;
	// Other internal icons
	else if (s == "internal:finish")
		icon = TrackSettings::IconFinish;
	else if (s == "internal:needle")
		icon = TrackSettings::IconNeedle;
	else if (s == "internal:spot")
		icon = TrackSettings::IconSpot;
	else if (s == "internal:position-play")
		icon = TrackSettings::IconPositionPlay;
	else if (s == "internal:position-stop")
		icon = TrackSettings::IconPositionStop;
	else if (s == "internal:position-bike")
		icon = TrackSettings::IconPositionBike;
	else if (s == "internal:position-drone-potensic")
		icon = TrackSettings::IconPositionDronePotensic;
	// At last user icons
	else if (Utils::starts_with(s, "file:"))
		icon = TrackSettings::IconUserFile;
	else
		icon = TrackSettings::IconUnknown;

	return icon;
}


std::string TrackSettings::view2string(View view) {
	switch (view) {
	case TrackSettings::ViewZoomFit:
		return "zoomfit";
	case TrackSettings::ViewLockCenter:
		return "center";
	case TrackSettings::ViewDefault:
		return "default";
	default:
		return "";
	}
}


std::string TrackSettings::follow2string(Follow follow) {
	switch (follow) {
	case TrackSettings::FollowCourse:
		return "course";
	case TrackSettings::FollowHeading:
		return "heading";
	case TrackSettings::FollowNone:
	default:
		return "none";
	}
}


std::string TrackSettings::icon2string(TrackSettings::Icon icon) {
	switch (icon) {
	case TrackSettings::IconDefault:
		return "default";

	// Internal type icons
	case TrackSettings::IconPlay:
		return "internal:play";
	case TrackSettings::IconStop:
		return "internal:stop";
	case TrackSettings::IconPosition:
		return "internal:position";

	// Other internal icons
	case TrackSettings::IconFinish:
		return "internal:finish";
	case TrackSettings::IconNeedle:
		return "internal:needle";
	case TrackSettings::IconSpot:
		return "internal:spot";
	case TrackSettings::IconPositionPlay:
		return "internal:position-play";
	case TrackSettings::IconPositionStop:
		return "internal:position-stop";
	case TrackSettings::IconPositionBike:
		return "internal:position-bike";
	case TrackSettings::IconPositionDronePotensic:
		return "internal:position-drone-potensic";

	// At last user icons
	case TrackSettings::IconUserFile:
		return "file:";

	default:
		return "";
	}
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

	is_init_ = false;

	assets_path_ = "";

	bg_buf_ = NULL;
	fg_buf_ = NULL;
	trackbuf_ = NULL;

	icon_end_buf_ = NULL;
	icon_start_buf_ = NULL;
	icon_position_buf_ = NULL;

	divider_ = 1.0;

	pvx1_ = pvy1_ = pvx2_ = pvy2_ = 0;
	pevx1_ = pevy1_ = pevx2_ = pevy2_ = 0;

	x_end_ = y_end_ = 0;
	x_start_ = y_start_ = 0;
	ts_start_ = ts_end_ = 0;
}


Track::~Track() {
	log_call();

	if (icon_end_buf_ != NULL)
		delete icon_end_buf_;
	if (icon_start_buf_ != NULL)
		delete icon_start_buf_;
	if (icon_position_buf_ != NULL)
		delete icon_position_buf_;

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
	bool zoomfit;

	bool ok = false;

	TelemetryData p1, p2;

	TelemetrySource *source;
   
	log_call();

	zoomfit = (settings().view() == TrackSettings::ViewZoomFit);

	// Init
	last_posX_ = last_posY_ = -1;

	pvx1_ = pvy1_ = pvx2_ = pvy2_ = 0;
	pevx1_ = pevy1_ = pevx2_ = pevy2_ = 0;

	is_init_ = false;

	last_data_ = TelemetryData();

	// Assets path
	assets_path_ = app_.assets("icons");

	// Check telemetry data
	if (app_.settings().inputfile().empty()) {
		log_warn("Track init aborted due to missing telemetry data!");
		goto abort;
	}

	// Open telemetry data file
	source = TelemetryMedia::open(app_.settings().inputfile(), telemetry_settings_, true);

	if (source == NULL) {
		log_warn("Can't read telemetry data, skip %s widget initialization", name().c_str());
		goto abort;
	}

	// Create map bounding box
	source->getBoundingBox(
			zoomfit ? TelemetrySource::RangeData : TelemetrySource::RangeView, &p1, &p2);

	// Track settings
	track_settings_.setBoundingBox(p1.latitude(), p1.longitude(), p2.latitude(), p2.longitude());

	// Get data range
	source->getBoundingBox(TelemetrySource::RangeData, &lim_p1_, &lim_p2_);

	// Completed with success
	ok = true;

abort:
	// Delete previous track buffer
	if (trackbuf_ != NULL) {
		delete trackbuf_;
		trackbuf_ = NULL;
	}

	return ok;
}


void Track::init(void) {
	int zoom;

	int space;
	int padding_vertical;
	int padding_horizontal;

	double size;

	const float *color;

	double lat1, lon1;
	double lat2, lon2;

	int w, h;
	int width = settings().width();
	int height = settings().height();

	int data_width, data_height;

	int pos_lim_x1, pos_lim_y1;
	int pos_lim_x2, pos_lim_y2;

	std::string filename;

	log_call();

	if (!preinit()) {
		log_warn("Track init failure!");
		return;
	}

	zoom = settings().zoom();
	divider_ = settings().divider();

	// Compute track area
	settings().getBoundingBox(&lat1, &lon1, &lat2, &lon2);

	// Tiles for view area:
	// +---------+---------+---------+ ..... +---------+
	// | vx1,vy1 |         |         |       | vx2,vy1 |
	// +---------+---------+---------+ ..... +---------+
	// |         |         |         |       |         |
	// +---------+---------+---------+ ..... +---------+
	// | vx1,vy2 |         |         |       | vx2,vy2 |
	// +---------+---------+---------+ ..... +---------+

	// Convert view area lat/lon to pixel
	pvx1_ = Track::lon2pixel(zoom, lon1);
	pvy1_ = Track::lat2pixel(zoom, lat1);

	pvx2_ = Track::lon2pixel(zoom, lon2);
	pvy2_ = Track::lat2pixel(zoom, lat2);

	// Convert limit lat/lon to pixel
	lim_px1_ = floorf((float) Track::lon2pixel(zoom, lim_p1_.longitude()));
	lim_py1_ = floorf((float) Track::lat2pixel(zoom, lim_p1_.latitude()));

	lim_px2_ = floorf((float) Track::lon2pixel(zoom, lim_p2_.longitude()));
	lim_py2_ = floorf((float) Track::lat2pixel(zoom, lim_p2_.latitude()));

	// Compute extended view area to match with widget size
	pevx1_ = pvx1_;
	pevy1_ = pvy1_;

	pevx2_ = pvx2_;
	pevy2_ = pvy2_;

	// Compute padding
	space = ceilf((settings().pathThick() + settings().pathBorder()) / 2.0);

	padding_vertical = 2 * theme().border()
		+ theme().padding(VideoWidget::Theme::PaddingTop)
		+ theme().padding(VideoWidget::Theme::PaddingBottom);
	padding_horizontal = 2 * theme().border()
		+ theme().padding(VideoWidget::Theme::PaddingLeft)
		+ theme().padding(VideoWidget::Theme::PaddingRight);

	switch (settings().view()) {
	// Apply zoom fit - center track
	case TrackSettings::ViewZoomFit:
		// Apply padding
		width -= padding_horizontal + 2 * space;
		height -= padding_vertical + 2 * space;

		// Compute divider to match with the size of widget
		w = ceilf((float) pevx2_ - pevx1_);
		h = ceilf((float) pevy2_ - pevy1_);

		if ((w > 0) && (h > 0)) {
			if (((float) width / w) > ((float) height / h))
				divider_ = (float) height / h;
			else
				divider_ = (float) width / w;
		}

		// Append space around track
		pevx1_ -= ceilf(space / divider_);
		pevx2_ += ceilf(space / divider_);

		pevy1_ -= ceilf(space / divider_);
		pevy2_ += ceilf(space / divider_);

		break;

	// Center & lock view on the current position
	case TrackSettings::ViewLockCenter:
		// Apply padding
		width -= 2 * theme().border();
		height -= 2 * theme().border();

		// Increase width view so as match widget size
		// as position is at point of view '1' or '2'
		pevx1_ -= ((theme().width() / 2) + theme().padding(VideoWidget::Theme::PaddingLeft)) / divider_;
		pevx2_ += ((theme().width() / 2) + theme().padding(VideoWidget::Theme::PaddingRight)) / divider_;

		// Increase height view so as match widget size
		// as position is at point of view '1' or '2'
		pevy1_ -= ((theme().height() / 2) + theme().padding(VideoWidget::Theme::PaddingTop)) / divider_;
		pevy2_ += ((theme().height() / 2) + theme().padding(VideoWidget::Theme::PaddingBottom)) / divider_;

		// Adjust limit to append space around track
		lim_px1_ -= ceilf(space / divider_);
		lim_px2_ += ceilf(space / divider_);

		lim_py1_ -= ceilf(space / divider_);
		lim_py2_ += ceilf(space / divider_);

		// Crop view size
		pevx1_ = std::max(lim_px1_, pevx1_);
		pevx2_ = std::min(lim_px2_, pevx2_);

		pevy1_ = std::max(lim_py1_, pevy1_);
		pevy2_ = std::min(lim_py2_, pevy2_);

		break;

	// Default mode
	case TrackSettings::ViewDefault:
	default:
		// width x height of data area
		data_width = (lim_px2_ - lim_px1_) * divider_;
		data_height = (lim_py2_ - lim_py1_) * divider_;

		// Apply padding
		width -= padding_horizontal + 2 * space;
		height -= padding_vertical + 2 * space;

//		// Apply padding
//		width -= 2 * theme().border();
//		height -= 2 * theme().border();

		// Adjust limit to append space around track
		lim_px1_ -= ceilf(space / divider_);
		lim_px2_ += ceilf(space / divider_);

		lim_py1_ -= ceilf(space / divider_);
		lim_py2_ += ceilf(space / divider_);

		// Compute lim1 (top-left)
		pos_lim_x1 = (lim_px1_ - pevx1_) * divider_;
		pos_lim_y1 = (lim_py1_ - pevy1_) * divider_;

		// Compute lim2 (bottom-right)
		pos_lim_x2 = (lim_px2_ - pevx1_) * divider_;
		pos_lim_y2 = (lim_py2_ - pevy1_) * divider_;

		// Track fit in width
		if (data_width > width) {
			// Increase width view so as match widget size
			// as position is at point of view '1' or '2'
			if ((width / 2) > -pos_lim_x1)
				pevx1_ = lim_px1_;
			else if ((width / 2) > pos_lim_x2)
				pevx1_ -= (theme().width() - theme().padding(VideoWidget::Theme::PaddingRight)) / divider_;
			else
				pevx1_ -= (theme().width() / 2) / divider_;

			if ((width / 2) > pos_lim_x2)
				pevx2_ = lim_px2_;
			else if ((width / 2) > -pos_lim_x1)
				pevx2_ += (theme().width() - theme().padding(VideoWidget::Theme::PaddingLeft)) / divider_;
			else
				pevx2_ += (theme().width() / 2) / divider_;
		}
		else {
			// Use limit
			pevx1_ = lim_px1_;
			pevx2_ = lim_px2_;
		}

		// Track fit in height
		if (data_height > height) {
			// Increase height view so as match widget size
			// as position is at point of view '1' or '2'
			if ((height / 2) > -pos_lim_y1)
				pevy1_ = lim_py1_;
			else if ((height / 2) > -pos_lim_y2)
				pevy1_ -= (theme().height() - theme().padding(VideoWidget::Theme::PaddingBottom)) / divider_;
			else
				pevy1_ -= (theme().height() / 2) / divider_;

			if ((height / 2) > pos_lim_y2)
				pevy2_ = lim_py2_;
			else if ((height / 2) > -pos_lim_y1)
				pevy2_ += (theme().height() - theme().padding(VideoWidget::Theme::PaddingTop)) / divider_;
			else
				pevy2_ += (theme().height() / 2) / divider_;
		}
		else {
			// Use limit
			pevy1_ = lim_py1_;
			pevy2_ = lim_py2_;
		}

		// Crop view size
		pevx1_ = std::max(lim_px1_, pevx1_);
		pevx2_ = std::min(lim_px2_, pevx2_);

		pevy1_ = std::max(lim_py1_, pevy1_);
		pevy2_ = std::min(lim_py2_, pevy2_);

		break;
	}

	// Delete old buffers
	if (icon_end_buf_ != NULL)
		delete icon_end_buf_;
	if (icon_start_buf_ != NULL)
		delete icon_start_buf_;
	if (icon_position_buf_ != NULL)
		delete icon_position_buf_;

	// Load icons
	size = settings().iconSize(TrackSettings::IconEnd);
	color = settings().iconColor(TrackSettings::IconEnd);
	filename = getIconFilename(settings().icon(TrackSettings::IconEnd), TrackSettings::IconEnd);
	icon_end_buf_ = OIIOUtils::loadsvg(filename.c_str(), size, color);

	size = settings().iconSize(TrackSettings::IconStart);
	color = settings().iconColor(TrackSettings::IconStart);
	filename = getIconFilename(settings().icon(TrackSettings::IconStart), TrackSettings::IconStart);
	icon_start_buf_ = OIIOUtils::loadsvg(filename.c_str(), size, color);

	size = settings().iconSize(TrackSettings::IconPosition);
	color = settings().iconColor(TrackSettings::IconPosition);
	filename = getIconFilename(settings().icon(TrackSettings::IconPosition), TrackSettings::IconPosition);
	icon_position_buf_ = OIIOUtils::loadsvg(filename.c_str(), size, color);

	// Init done
	is_init_ = true;
}


void Track::path(OIIO::ImageBuf &outbuf, TelemetrySource *source, double divider) {
	int n;

	int zoom;
	int stride;
	double path_thick;
	double path_border;
	unsigned char *bytes;

	int path_smooth;

	const float *fill;
	const float *outline;

	int x = 0, y = 0;

	TelemetryData wpt;

	enum TelemetrySource::Data result;

	log_call();

	zoom = settings().zoom();
	path_thick = settings().pathThick();
	path_border = settings().pathBorder();
	path_smooth = settings().pathSmooth();

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
		cairo_set_source_rgba(cairo, outline[0], outline[1], outline[2], outline[3]); // BGR #0000000
		cairo_set_line_width(cairo, path_border + path_thick); //4.4); //40.96);
		cairo_set_line_join(cairo, CAIRO_LINE_JOIN_ROUND);

		// Draw each WPT
		for (n = 0, result = source->retrieveFirst(wpt); result != TelemetrySource::DataEof; result = source->retrieveNext(wpt)) {
			if ((path_smooth > 1) && ((n++ % path_smooth) != 0))
				continue;

			x = floorf((float) Track::lon2pixel(zoom, wpt.longitude())) - pevx1_;
			y = floorf((float) Track::lat2pixel(zoom, wpt.latitude())) - pevy1_;

			x *= divider;
			y *= divider;

			cairo_line_to(cairo, x, y);
		}

		// Cairo draw
		cairo_stroke(cairo);
	}

	// Draw each WPT with secondary color
	cairo_set_source_rgba(cairo, fill[0], fill[1], fill[2], fill[3]); // BGR #669df600
	cairo_set_line_width(cairo, path_thick); //3.0); //40.96);
	cairo_set_line_join(cairo, CAIRO_LINE_JOIN_ROUND);

	for (n = 0, result = source->retrieveFirst(wpt); result != TelemetrySource::DataEof; result = source->retrieveNext(wpt)) {
		if ((path_smooth > 1) && ((n++ % path_smooth) != 0))
			continue;

		x = floorf((float) Track::lon2pixel(zoom, wpt.longitude())) - pevx1_;
		y = floorf((float) Track::lat2pixel(zoom, wpt.latitude())) - pevy1_;

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
	int path_smooth;
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
	path_smooth = settings().pathSmooth();

	fill = settings().pathPrimaryColor();

	// Start or continue path ?
	if (last_data_.type() == TelemetryData::TypeUnknown) {
		int n = 0;

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
			if ((path_smooth > 1) && ((n++ % path_smooth) != 0))
				continue;

			x = floorf((float) Track::lon2pixel(zoom, wpt.longitude())) - pevx1_;
			y = floorf((float) Track::lat2pixel(zoom, wpt.latitude())) - pevy1_;

			x *= divider;
			y *= divider;

			cairo_line_to(cairo, x, y);

			if (wpt.timestamp() >= data.timestamp())
				break;
		}

		cairo_stroke(cairo);
	}
	else if (!data.hasValue(TelemetryData::DataFix)) {
		goto skip;
	}
	else if (!last_data_.hasValue(TelemetryData::DataFix)) {
		// Save current position
		last_data_ = data;

		goto skip;
	}
	else {
		int x1, y1;
		int x2, y2;

		int width, height;

		const OIIO::ImageSpec& spec = outbuf.spec();

		OIIO::TypeDesc::BASETYPE type = (OIIO::TypeDesc::BASETYPE) spec.format.basetype;

		// Last point
		x1 = floorf((float) Track::lon2pixel(zoom, last_data_.longitude())) - pevx1_;
		y1 = floorf((float) Track::lat2pixel(zoom, last_data_.latitude())) - pevy1_;

		x1 *= divider;
		y1 *= divider;

		// Last point
		x1 = last_posX_;
		y1 = last_posY_;

		// Current point
		x2 = floorf((float) Track::lon2pixel(zoom, data.longitude())) - pevx1_;
		y2 = floorf((float) Track::lat2pixel(zoom, data.latitude())) - pevy1_;

		x2 *= divider;
		y2 *= divider;

		if (last_data_.type() != TelemetryData::TypeUnknown) {
			// Smooth
			x2 = (x1 + x2) / 2;
			y2 = (y1 + y2) / 2;

			// Move ?
			if ((x1 == x2) && (y1 == y2))
				goto skip;
		}

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

	if (!isInitialized())
		return true;

	// Compute track size
	width = ceilf((pevx2_ - pevx1_) * divider_);
	height = ceilf((pevy2_ - pevy1_) * divider_);

	// Load telemetry data
	TelemetrySource *source = TelemetryMedia::open(filename, telemetry_settings_, true);

	if (source != NULL) {
		// Create background track buffer
		trackbuf_ = new OIIO::ImageBuf(OIIO::ImageSpec(width, height, 4, OIIO::TypeDesc::UINT8)); //, OIIO::InitializePixels::No);

		// Draw background path
		path(*trackbuf_, source, divider_);

		// Compute begin
		source->retrieveFirst(wpt);

		x_start_ = floorf((float) Track::lon2pixel(zoom, wpt.longitude())) - pevx1_;
		y_start_ = floorf((float) Track::lat2pixel(zoom, wpt.latitude())) - pevy1_;

		x_start_ *= divider_;
		y_start_ *= divider_;
		ts_start_ = source->beginTimestamp();

		// Compute end
		source->retrieveLast(wpt);

		x_end_ = floorf((float) Track::lon2pixel(zoom, wpt.longitude())) - pevx1_;
		y_end_ = floorf((float) Track::lat2pixel(zoom, wpt.latitude())) - pevy1_;

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


OIIO::ImageBuf * Track::render(const TelemetryData &data, bool &is_update) {
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

	// Load track
	if (this->load() == false)
		log_warn("Track renderer failure");

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
		if ((posX == last_posX_) && (posY == last_posY_)) {
			is_update = false;
			goto skip;
		}
	}

	// track position
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
	case TrackSettings::ViewZoomFit:
		// Apply padding
		offsetX = theme().padding(VideoWidget::Theme::PaddingLeft);
		offsetY = theme().padding(VideoWidget::Theme::PaddingTop);

		// Add offset to center the track
		offsetX += (width_available - w) / 2;
		offsetY += (height_available - h) / 2;

		break;

	// Center & lock view on the current position
	case TrackSettings::ViewLockCenter:
		// Add offset to center on the current position
		offsetX = theme().padding(VideoWidget::Theme::PaddingLeft);
		offsetY = theme().padding(VideoWidget::Theme::PaddingTop);

		offsetX += (width_available / 2) - posX;
		offsetY += (height_available / 2) - posY;

		break;

	// Default mode
	case TrackSettings::ViewDefault:
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

		if (data_width > width) {
			if ((posX - pos_lim_x1) < (width_available / 2))
				offsetX += posX - pos_lim_x1 - (width_available / 2);
			else if ((pos_lim_x2 - posX) < (width_available / 2))
				offsetX += posX - pos_lim_x2 + (width_available / 2);
		}
		else
			offsetX = (width_available - w) / 2;

		if (data_height > height) {
			if ((pos_lim_y2 - posY) < (height_available / 2))
				offsetY += posY - pos_lim_y2 + (height_available / 2);
			else if ((posY - pos_lim_y1) < (height_available / 2))
				offsetY += posY - pos_lim_y1 - (height_available / 2);
		}
		else
			offsetY = (height_available - h) / 2;

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

	// Path
	if (trackbuf_ != NULL) {
		// Update path progress
		trackbuf_->specmod().x = 0;
		trackbuf_->specmod().y = 0;
		path(*trackbuf_, data, divider_);

		// Draw track image over
		trackbuf_->specmod().x = x + offsetX;
		trackbuf_->specmod().y = y + offsetY;
		OIIO::ImageBufAlgo::over(*fg_buf_, *trackbuf_, *fg_buf_, OIIO::ROI(x, x + width, y, y + height));

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
			else {
				icon(*fg_buf_, *icon_position_buf_, x + offsetX + posX, y + offsetY + posY, OIIO::ROI(x, x + width, y, y + height));
			}
		}
	}

	// Save last position
	last_posX_ = posX;
	last_posY_ = posY;

	is_update = true;
skip:
	return fg_buf_;
}


bool Track::updated(const TelemetryData &data) const {
	int posX, posY;

	int zoom = settings().zoom();

	// Check track buffer
	if (trackbuf_ == NULL)
		return false;

	// Refresh dynamic info
	if ((fg_buf_ != NULL) && (data.type() == TelemetryData::TypeUnchanged))
		return false;

	// Current position
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

	// Move ?
	if ((posX == last_posX_) && (posY == last_posY_))
		return false;

	return true;
}


void Track::draw(cairo_t *cr, const TelemetryData &data) {
	(void) data;

	// Draw
	background(cr, theme().roundCorner());
}


void Track::clear(void) {
	last_posX_= -1;
	last_posY_= -1;

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


bool Track::icon(OIIO::ImageBuf &map, OIIO::ImageBuf &icon, int x, int y, OIIO::ROI roi) {
	bool result;

	// Icon position
	x -= icon.spec().width / 2;
	y -= icon.spec().height / 2;

	// Image over
	icon.specmod().x = x;
	icon.specmod().y = y;
	result = OIIO::ImageBufAlgo::over(map, icon, map, roi);

	if (!result)
		log_error("ImageBufAlgo::over failure");

	return result;
}


std::string Track::getIconFilename(TrackSettings::Icon icon, TrackSettings::Icon bydefault) {
	log_call();

	std::string path = GPXApplication::assets("marker");

	if (icon == TrackSettings::IconDefault)
		icon = bydefault;
	else if (icon == TrackSettings::IconUserFile)
		return settings().iconFile(bydefault);
	
	switch (icon) {
	// Internal type icons
	case TrackSettings::IconPlay:
		return path + "/play.svg";
	case TrackSettings::IconStop:
		return path + "/stop.svg";
	case TrackSettings::IconPosition:
	// Other internal icons
		return path + "/position.svg";
	case TrackSettings::IconFinish:
		return path + "/finish.svg";
	case TrackSettings::IconNeedle:
		return path + "/needle.svg";
	case TrackSettings::IconSpot:
		return path + "/spot.svg";
	case TrackSettings::IconPositionPlay:
		return path + "/position-play.svg";
	case TrackSettings::IconPositionStop:
		return path + "/position-stop.svg";
	case TrackSettings::IconPositionBike:
		return path + "/position-bike.svg";
	case TrackSettings::IconPositionDronePotensic:
		return path + "/position-drone-potensic.svg";
	default:
		return "";
	}
}


void Track::xmlwrite(std::ostream &os) {
	VideoWidget::xmlwrite(os);

	ShapeBase::xmlwrite(os);

	os << "<with-icon-start>" << VideoWidget::bool2string(theme().hasFlag(VideoWidget::Theme::FlagIconStart)) << "</with-icon-start>" << std::endl;
	os << "<with-icon-end>" << VideoWidget::bool2string(theme().hasFlag(VideoWidget::Theme::FlagIconEnd)) << "</with-icon-end>" << std::endl;
	os << "<with-icon-position>" << VideoWidget::bool2string(theme().hasFlag(VideoWidget::Theme::FlagIconPosition)) << "</with-icon-position>" << std::endl;

	os << "<view>" << TrackSettings::view2string(settings().view()) << "</view>" << std::endl;
	os << "<factor>" << settings().divider() << "</factor>" << std::endl;

	os << "<path-smooth>" << settings().pathSmooth() << "</path-smooth>" << std::endl;
	os << "<path-thick>" << settings().pathThick() << "</path-thick>" << std::endl;
	os << "<path-border>" << settings().pathBorder() << "</path-border>" << std::endl;
	os << "<path-border-color>" << VideoWidget::Theme::color2hex(settings().pathBorderColor()) << "</path-border-color>" << std::endl;
	os << "<path-primary-color>" << VideoWidget::Theme::color2hex(settings().pathPrimaryColor()) << "</path-primary-color>" << std::endl;
	os << "<path-secondary-color>" << VideoWidget::Theme::color2hex(settings().pathSecondaryColor()) << "</path-secondary-color>" << std::endl;

	os << "<follow>" << TrackSettings::follow2string(settings().follow()) << "</follow>" << std::endl;

	os << "<icon-start-name>" << TrackSettings::icon2string(settings().icon(TrackSettings::IconStart)) << settings().iconFile(TrackSettings::IconStart) << "</icon-start-name>" << std::endl;
	os << "<icon-start-color>" << VideoWidget::Theme::color2hex(settings().iconColor(TrackSettings::IconStart)) << "</icon-start-color>" << std::endl;
	os << "<icon-start-size>" << settings().iconSize(TrackSettings::IconStart) << "</icon-start-size>" << std::endl;

	os << "<icon-end-name>" << TrackSettings::icon2string(settings().icon(TrackSettings::IconEnd)) << settings().iconFile(TrackSettings::IconEnd) << "</icon-end-name>" << std::endl;
	os << "<icon-end-color>" << VideoWidget::Theme::color2hex(settings().iconColor(TrackSettings::IconEnd)) << "</icon-end-color>" << std::endl;
	os << "<icon-end-size>" << settings().iconSize(TrackSettings::IconEnd) << "</icon-end-size>" << std::endl;

	os << "<icon-position-name>" << TrackSettings::icon2string(settings().icon(TrackSettings::IconPosition)) << settings().iconFile(TrackSettings::IconPosition) << "</icon-position-name>" << std::endl;
	os << "<icon-position-color>" << VideoWidget::Theme::color2hex(settings().iconColor(TrackSettings::IconPosition)) << "</icon-position-color>" << std::endl;
	os << "<icon-position-size>" << settings().iconSize(TrackSettings::IconPosition) << "</icon-position-size>" << std::endl;
}

