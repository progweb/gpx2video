#include "verticalspeed.h"


/**
 * Text shape
 */

bool VerticalSpeedTextShape::updated(const TelemetryData &data) const {
	if (is_initialized_) {
		if ((data.type() == TelemetryData::TypeUnchanged))
			return false;

		if (no_value_ && !data.hasValue(TelemetryData::DataVerticalSpeed))
			return false;
	}

	return true;
}


void VerticalSpeedTextShape::initialize(cairo_t *cr) {
	if (is_initialized_)
		return;

	int x, y;
	int width, height;

	TextShape::Font font;

	setSize(theme().height());

	setPadding(
		theme().border() + theme().padding(VideoWidget::Theme::PaddingLeft),
		theme().border() + theme().padding(VideoWidget::Theme::PaddingRight),
   		theme().border() + theme().padding(VideoWidget::Theme::PaddingTop),
   		theme().border() + theme().padding(VideoWidget::Theme::PaddingBottom));

	// Icon
	icon_filename_ = widget_->getIconFilename(theme().icon());

	// Label height
	if (theme().hasFlag(VideoWidget::Theme::FlagLabel)) {
		font = (TextShape::Font) {
			.size = theme().labelFontSize(),
			.border = theme().labelBorderWidth(),
			.shadow_opacity = theme().labelShadowOpacity(),
			.shadow_distance = theme().labelShadowDistance(),
			.family = theme().labelFontFamily(),
			.align = theme().labelHorizontalAlign(),
			.style = theme().labelFontStyle(),
			.weight = theme().labelFontWeight(),
		};

		extents(cr, font, widget_->label().c_str(), x, y, width, height);

		setLabelExtents(x, y, width, height);
	}

	// Value height
	if (theme().hasFlag(VideoWidget::Theme::FlagValue)) {
		std::string txt = std::to_string(888.8);

		std::string unit = theme().hasFlag(VideoWidget::Theme::FlagUnit) ?
			widget_->getFriendlyName(widget_->valueUnit()) : "";

		font = (TextShape::Font) {
			.size = theme().valueFontSize(),
			.border = theme().valueBorderWidth(),
			.shadow_opacity = theme().valueShadowOpacity(),
			.shadow_distance = theme().valueShadowDistance(),
			.family = theme().valueFontFamily(),
			.align = theme().valueHorizontalAlign(),
			.style = theme().valueFontStyle(),
			.weight = theme().valueFontWeight(),
		};

		txt = txt + unit;

		extents(cr, font, txt.c_str(), x, y, width, height);
		
		setValueExtents(x, y, width, height);
	}

	is_initialized_ = true;
}


void VerticalSpeedTextShape::draw(cairo_t *cr, const TelemetryData &data) {
	char s[128];

	TextShape::Font font;

	double verticalspeed = data.verticalspeed(widget_->valueUnit());

	std::string unit = theme().hasFlag(VideoWidget::Theme::FlagUnit) ?
		widget_->getFriendlyName(widget_->valueUnit()) : "";

	// Initialize
	initialize(cr);

	// Format data
	no_value_ = !data.hasValue(TelemetryData::DataVerticalSpeed);

	if (!no_value_)
		sprintf(s, "%.1f %s", verticalspeed, unit.c_str());
	else
		sprintf(s, "-- %s", unit.c_str());

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
			.align = theme().labelHorizontalAlign(),
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
			.align = theme().valueHorizontalAlign(),
			.style = theme().valueFontStyle(),
			.weight = theme().valueFontWeight(),
		};

		value(cr, font, theme().valueColor(), theme().valueBorderColor(), s);
	}
}


void VerticalSpeedTextShape::clear(void) {
	TextShape::clear();

	no_value_ = false;

	if (bg_buf_)
		delete bg_buf_;

	if (fg_buf_)
		delete fg_buf_;

	bg_buf_ = NULL;
	fg_buf_ = NULL;
}


/**
 * Bar shape
 */

bool VerticalSpeedBarShape::updated(const TelemetryData &data) const {
	if (is_initialized_) {
		if ((data.type() == TelemetryData::TypeUnchanged))
			return false;

		if (no_value_ && !data.hasValue(TelemetryData::DataVerticalSpeed))
			return false;
	}

	return true;
}


void VerticalSpeedBarShape::initialize(cairo_t *cr) {
	if (is_initialized_)
		return;

	int x, y;
	int width, height;

	TextShape::Font font;

	width_ = theme().width();
	height_ = theme().height();

	size_ = std::min(width_, height_);

	// Size
	setSize(width_, height_, size_); //, size_);

	// Padding
	setPadding(
		theme().border() + theme().padding(VideoWidget::Theme::PaddingLeft),
		theme().border() + theme().padding(VideoWidget::Theme::PaddingRight),
   		theme().border() + theme().padding(VideoWidget::Theme::PaddingTop),
   		theme().border() + theme().padding(VideoWidget::Theme::PaddingBottom));

	// Label height
	if (theme().hasFlag(VideoWidget::Theme::FlagLabel)) {
		font = (TextShape::Font) {
			.size = theme().labelFontSize(),
			.border = theme().labelBorderWidth(),
			.shadow_opacity = theme().labelShadowOpacity(),
			.shadow_distance = theme().labelShadowDistance(),
			.family = theme().labelFontFamily(),
			.align = theme().labelHorizontalAlign(),
			.style = theme().labelFontStyle(),
			.weight = theme().labelFontWeight(),
		};

		extents(cr, font, widget_->label().c_str(), x, y, width, height);

		setLabelExtents(x, y, width, height);
	}

	// Tick
	tick_step_ = 10;
	tick_mstep_ = 10;

	is_initialized_ = true;
}


void VerticalSpeedBarShape::ticklenwidth(int value, int *offset, int *len, int *width) {
	int size = theme().tickSize(); // size_ / 8

	VideoWidget::Theme::Align align = theme().tickAlign();

	if (value % 10 == 0) {
		*len = size + (size_ / 25);
		*width = size_ / 64;
		*offset = 0;
	}
	else if (value % 5 == 0) {
		*len = size;
		*width = size_ / 64;
		if (align == VideoWidget::Theme::AlignCenter)
			*offset = (size_ / 25) / 2;
		else if (align == VideoWidget::Theme::AlignRight)
			*offset = (size_ / 25);
		else
			*offset = 0;
	}
	else {
		*len = size - (size_ / 25);
		*width = size_ / 128;
		if (align == VideoWidget::Theme::AlignCenter)
			*offset = (size_ / 25);
		else if (align == VideoWidget::Theme::AlignRight)
			*offset = 2 * (size_ / 25);
		else
			*offset = 0;
	}
}


void VerticalSpeedBarShape::draw(cairo_t *cr, const TelemetryData &data) {
	char s[128];

	BarShape::Font font;

	int offset = 0;
	int rotate = 0;

	int top = 0, bottom = 0;

	int amin, amax;

	double xb1, xb2;

	double verticalspeed = data.verticalspeed(widget_->valueUnit());

	std::string unit = theme().hasFlag(VideoWidget::Theme::FlagUnit) ?
		widget_->getFriendlyName(widget_->valueUnit()) : "";

	// Initialize
	initialize(cr);

	// Limit
	amin = theme().valueMin();
	amax = theme().valueMax();

	// Format data
	no_value_ = !data.hasValue(TelemetryData::DataVerticalSpeed);

	// Apply padding
	if (theme().hasFlag(VideoWidget::Theme::FlagValue))
		offset = (size_ / 14);
	else if (theme().hasFlag(VideoWidget::Theme::FlagTickLabel))
		offset = (size_ / 14) / 2;
	else
		offset = 0;

	if (theme().hasFlag(VideoWidget::Theme::FlagLabel))
		bottom = size_ / 10;

	setPadding(
		theme().padding(VideoWidget::Theme::PaddingLeft),
		theme().padding(VideoWidget::Theme::PaddingRight),
		theme().padding(VideoWidget::Theme::PaddingTop) + offset + top,
		theme().padding(VideoWidget::Theme::PaddingBottom) + offset + bottom
	);

	// Compute gauge position
	offset = ((size_ / 8) + (size_ / 25)); // gauge width
	offset /= 2;
	setOffset(-offset);

	// Draw background
	background(cr);

	// Draw gauge background
	bar(cr, 0, 1, (size_ / 8) + (size_ / 25), theme().gaugeBorder(),
			theme().gaugeBackgroundColor(), theme().gaugeBorderColor());

	// Draw gauge
	if (theme().hasFlag(VideoWidget::Theme::FlagGauge) && (verticalspeed >= amin)) {
		double to = std::min(verticalspeed, (double) amax);
		double from = (to > 0) ? std::max(0, amin) : std::min(0, amax);

		xb1 = scale(amin, amax, from, rotate);
		xb2 = scale(amin, amax, to, rotate);

		bar(cr, xb1, xb2, size_ / 8 + size_ / 25, 0,
				theme().gaugePrimaryColor());
	}

	// Draw cursor
	if (theme().hasFlag(VideoWidget::Theme::FlagCursor) && ((verticalspeed >= amin) && (verticalspeed <= amax))) {
		double xb = scale(amin, amax, verticalspeed, rotate);

		cursor(cr, xb, size_ / 8 + size_ / 25, theme().cursorColor());
	}

	// Draw tick lines on bar
	if (theme().hasFlag(VideoWidget::Theme::FlagTick)) {
		for (int value = amin; value < amax + tick_step_; value = value + tick_step_) {
			int ticklen;
			int tickwidth;
			int tickoffset;

			double xb = scale(amin, amax, value, rotate);

			ticklenwidth(value / tick_mstep_, &tickoffset, &ticklen, &tickwidth);

			line(cr, xb, tickoffset, tickoffset + ticklen, tickwidth, theme().tickColor());
		}
	}

	// Draw tick label
	if (theme().hasFlag(VideoWidget::Theme::FlagTickLabel)) {
		int min = amin; // + (mstep_ * step_);
		int max = amax;

		bool first = true;

		int step = tick_mstep_ * tick_step_;

		min /= step;
		min *= step;

		max /= step;
		max *= step;

		for (int value = min; value < max + step; value = value + step) {
			double xb = scale(amin, amax, value, rotate);

			double factor = (double) theme().tickLabelFontSize() / (double) theme().valueFontSize();

			if (first || (value >= max))
				sprintf(s, "%d %s", value, unit.c_str());
			else
				sprintf(s, "%d", value);

			if (value < amin)
				continue;

			font = (TextShape::Font) {
				.size = (int) (theme().valueFontSize() * factor),
				.border = (int) (theme().valueBorderWidth() * factor),
				.shadow_opacity = theme().valueShadowOpacity(),
				.shadow_distance = (int) (theme().valueShadowDistance() * factor),
				.family = theme().valueFontFamily(),
				.align = VideoWidget::Theme::AlignCenter,
				.style = theme().valueFontStyle(),
				.weight = theme().valueFontWeight(),
			};

			ticklabel(cr, xb, size_ / 2.75, font, theme().tickLabelColor(), theme().tickLabelBorderColor(), s);

			first = false;
		}
	}

//	// Draw marker value
//	if (theme().hasFlag(VideoWidget::Theme::FlagValue) && ((verticalspeed >= amin) && (verticalspeed <= amax))) {
//		double xb = scale(amin, amax, verticalspeed, rotate);
//
//		std::string value = std::to_string((int) std::round(verticalspeed));
//
//		marker(cr, xb, theme().valueBorderWidth(), theme().valueColor(), theme().valueBackgroundColor(), theme().valueBorderColor(), value);
//	}

	// Draw label
	if (theme().hasFlag(VideoWidget::Theme::FlagLabel)) {
		font = (TextShape::Font) {
			.size = theme().labelFontSize(),
			.border = theme().labelBorderWidth(),
			.shadow_opacity = theme().labelShadowOpacity(),
			.shadow_distance = theme().labelShadowDistance(),
			.family = theme().labelFontFamily(),
			.align = theme().labelHorizontalAlign(),
			.style = theme().labelFontStyle(),
			.weight = theme().labelFontWeight(),
		};

		label(cr, font, theme().labelColor(), theme().labelBorderColor(), widget_->label().c_str());
	}
}


void VerticalSpeedBarShape::clear(void) {
	BarShape::clear();

	no_value_ = false;

	if (bg_buf_)
		delete bg_buf_;

	if (fg_buf_)
		delete fg_buf_;

	bg_buf_ = NULL;
	fg_buf_ = NULL;
}


VerticalSpeedWidget::VerticalSpeedWidget(GPXApplication &app)
	: VideoWidget(app, VideoWidget::WidgetVerticalSpeed) 
	, ShapeBase(VideoWidget::theme())
	, shape_(NULL) {

#define ADD_SHAPE(shape) \
	shapes_supported_.push_back((VideoWidget::ListItem) { \
		shape, VideoWidget::getFriendlyName(shape), VideoWidget::shape2string(shape) \
	})

	ADD_SHAPE(VideoWidget::ShapeBar);
	ADD_SHAPE(VideoWidget::ShapeText);

#define ADD_UNIT(unit) \
	units_supported_.push_back((VideoWidget::ListItem) { \
		unit, VideoWidget::getFriendlyName(unit), VideoWidget::unit2string(unit) \
	})

	ADD_UNIT(TelemetryData::UnitMilesPerSec);
	ADD_UNIT(TelemetryData::UnitMeterPerSec);
	ADD_UNIT(TelemetryData::UnitFeetPerSec);

	setShape(VideoWidget::ShapeText);
}

