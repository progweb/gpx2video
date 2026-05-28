#include "time.h"


/**
 * Text shape
 */

bool TimeTextShape::updated(const TelemetryData &data) const {
	time_t t;

	// Compute time
	t = data.datetime() / 1000;

	// Check changes
	if (is_initialized_) {
		if (t == last_time_) {
			return false;
		}
	}

	return true;
}


void TimeTextShape::initialize(void) {
	if (is_initialized_)
		return;

	setSize(theme().height());

	setPadding(
		theme().border() + theme().padding(VideoWidget::Theme::PaddingLeft),
		theme().border() + theme().padding(VideoWidget::Theme::PaddingRight),
   		theme().border() + theme().padding(VideoWidget::Theme::PaddingTop),
   		theme().border() + theme().padding(VideoWidget::Theme::PaddingBottom));

	is_initialized_ = true;
}


void TimeTextShape::draw(cairo_t *cr, const TelemetryData &data) {
	char s[128];

	TextShape::Font font;

	time_t t;
	struct tm time;

	// Initialize
	initialize();

	// Format data
	if (data.datetime() != 0) {
		// Compute time
		t = data.datetime() / 1000;

		// Format data
		// Don't use gps time, but camera time!
		// Indeed, with garmin devices, gpx time has an offset.
		localtime_r(&t, &time);

		strftime(s, sizeof(s), "%H:%M:%S", &time);
	}
	else
		strncpy(s, "--:--:--", sizeof(s));

	// Draw background
	background(cr);

	// Draw icon
	if (theme().hasFlag(VideoWidget::Theme::FlagIcon)) {
		icon(cr, icon_filename_, theme().iconColor());
	}

	// Draw label
	if (theme().hasFlag(VideoWidget::Theme::FlagLabel)) {
		font = (TextShape::Font) {
			.size = theme().labelFontSize(),
			.border = theme().labelBorderWidth(),
			.shadow_opacity = theme().labelShadowOpacity(),
			.shadow_distance = theme().labelShadowDistance(),
			.family = theme().labelFontFamily(),
			.align = theme().labelAlign(),
			.style = theme().labelFontStyle(),
			.weight = theme().labelFontWeight(),
		};

		label(cr, font, theme().labelColor(), theme().labelBorderColor(), widget_->label().c_str());
	}

	// Draw value
	if (theme().hasFlag(VideoWidget::Theme::FlagValue)) {
		font = (TextShape::Font) {
			.size = theme().valueFontSize(),
			.border = theme().valueBorderWidth(),
			.shadow_opacity = theme().valueShadowOpacity(),
			.shadow_distance = theme().valueShadowDistance(),
			.family = theme().valueFontFamily(),
			.align = theme().valueAlign(),
			.style = theme().valueFontStyle(),
			.weight = theme().valueFontWeight(),
		};

		value(cr, font, theme().valueColor(), theme().valueBorderColor(), s);
	}

	// Save last value
	last_time_ = t;
}


void TimeTextShape::clear(void) {
	is_initialized_ = false;

	if (bg_buf_)
		delete bg_buf_;

	if (fg_buf_)
		delete fg_buf_;

	bg_buf_ = NULL;
	fg_buf_ = NULL;
}


/**
 * Text shape
 */

bool TimeArcShape::updated(const TelemetryData &data) const {
	time_t t;

	// Compute time
	t = data.datetime() / 1000;

	// Check changes
	if (is_initialized_) {
		if (t == last_time_) {
			return false;
		}
	}

	return true;
}


void TimeArcShape::initialize(void) {
	width_ = theme().width();
	height_ = theme().height();

	size_ = std::min(width_, height_);

	init(fontface_, width_, height_, size_);

	setPadding(
		theme().border() + theme().padding(VideoWidget::Theme::PaddingLeft),
		theme().border() + theme().padding(VideoWidget::Theme::PaddingRight),
   		theme().border() + theme().padding(VideoWidget::Theme::PaddingTop),
   		theme().border() + theme().padding(VideoWidget::Theme::PaddingBottom));

	is_initialized_ = true;
}


void TimeArcShape::tickinit(int min, int max) {
	(void) min;
	(void) max;

	tick_step_ = 1;
	tick_mstep_ = 0;
}


void TimeArcShape::ticklenwidth(int value, int *len, int *width) {
	int size = theme().tickSize(); // size_ / 15
								   //
	if (value % 5 == 0) {
		*len = size + size_ / 51;
		*width = size_ / 96;
	}
	else {
		*len = size - size_ / 51;
		*width = size_ / 128;
	}
}


void TimeArcShape::draw(cairo_t *cr, const TelemetryData &data) {
	bool no_value = false;

	int rotate = 0;

	VideoWidget::Theme::NeedleType type;

	double xa;

	time_t t;
	struct tm time;

	int tmin, tmax;

	const float *primary_color, *secondary_color;

	// Initialize
	initialize();

	// Limit
	tmin = 0;
	tmax = 60;

	// Arc size
	int arc_size = 360;
	int arc_rotate = 0;

	rotate += arc_rotate;

	setArcRange(180 - (arc_size / 2), 180 + (arc_size / 2));

	// Format data
	if (data.datetime() != 0) {
		// Compute time
		t = data.datetime() / 1000;

		// Format data
		// Don't use gps time, but camera time!
		// Indeed, with garmin devices, gpx time has an offset.
		localtime_r(&t, &time);
	}
	else
		no_value = true;

	// Tick init
	tickinit(tmin, tmax);

	// Draw background
	pieslice(cr, 0, 360, theme().border(),
			theme().backgroundColor(), theme().borderColor());

	// Draw tick lines around arc line
	if (theme().hasFlag(VideoWidget::Theme::FlagTick)) {
		for (int value = tmin; value < tmax + tick_step_; value = value + tick_step_) {
			int ticklen;
			int tickwidth;

			xa = scale(tmin, tmax, value, rotate);

			if (xa > (end() + rotate))
				break;

			ticklenwidth(value, &ticklen, &tickwidth);

			line(cr, xa, 0, ticklen, tickwidth, theme().tickColor());
		}
	}

	// Draw arc label
	// 1 2 3 4 5 6...
	// 1 3 6...
	// 1 6...
	if (theme().hasFlag(VideoWidget::Theme::FlagTickLabel)) {
		int mstep = (tick_mstep_ > 0) ? 3 * tick_mstep_ : 1;

		double distance = theme().tickSize();
		
		distance += theme().tickLabelDistance();

		for (int value = 0; value < 12; value = value + mstep) {
			xa = scale(tmin / 5, tmax / 5, value + 1, rotate);

			if (xa > (end() + rotate))
				break;

			std::string str = std::to_string(value + 1);

			text(cr, xa, distance, theme().tickLabelColor(), str);
		}
	}

	// Draw needle
	if (!no_value && theme().hasFlag(VideoWidget::Theme::FlagNeedle)) {
		// thin, light, basic, design
		type = theme().needleType();

		// Colors
		primary_color = theme().needlePrimaryColor();
		secondary_color = theme().needleSecondaryColor();

		// Draw minute needle
		xa = scale(tmin, tmax, time.tm_min, rotate);
		needle(cr, type, xa, (size_ / 72), false, primary_color, secondary_color);

		// Draw hour needle
		xa = scale(tmin, tmax, time.tm_hour * 5, rotate);
		needle(cr, type, xa, (size_ / 4), false, primary_color, secondary_color);

		// Draw second needle
		xa = scale(tmin, tmax, time.tm_sec, rotate);
		needle(cr, VideoWidget::Theme::NeedleTypeThin, xa, 0.0,
				(type == VideoWidget::Theme::NeedleTypeLight) || (type == VideoWidget::Theme::NeedleTypeDesign),
				secondary_color, secondary_color);
	}

	// Save last value
	last_time_ = t;
}


void TimeArcShape::clear(void) {
	is_initialized_ = false;

	if (bg_buf_)
		delete bg_buf_;

	if (fg_buf_)
		delete fg_buf_;

	bg_buf_ = NULL;
	fg_buf_ = NULL;
}


TimeWidget::TimeWidget(GPXApplication &app)
	: VideoWidget(app, VideoWidget::WidgetTime) 
	, ShapeBase(VideoWidget::theme())
	, shape_(NULL) {

#define ADD_SHAPE(shape) \
	shapes_supported_.push_back((VideoWidget::ListItem) { \
		shape, VideoWidget::getFriendlyName(shape), VideoWidget::shape2string(shape) \
	})

	ADD_SHAPE(VideoWidget::ShapeArc);
	ADD_SHAPE(VideoWidget::ShapeText);

	setShape(VideoWidget::ShapeText);
}

