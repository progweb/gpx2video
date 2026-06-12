#include "elevation.h"


/**
 * Text shape
 */

bool ElevationTextShape::updated(const TelemetryData &data) const {
	if (is_initialized_) {
		if ((data.type() == TelemetryData::TypeUnchanged))
			return false;

		if (no_value_ && !data.hasValue(TelemetryData::DataElevation))
			return false;
	}

	return true;
}


void ElevationTextShape::initialize(cairo_t *cr) {
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


void ElevationTextShape::draw(cairo_t *cr, const TelemetryData &data) {
	char s[128];

	TextShape::Font font;

	double elevation = data.elevation(widget_->valueUnit());

	std::string unit = theme().hasFlag(VideoWidget::Theme::FlagUnit) ?
		widget_->getFriendlyName(widget_->valueUnit()) : "";

	// Initialize
	initialize(cr);

	// Format data
	no_value_ = !data.hasValue(TelemetryData::DataElevation);

 	if (!no_value_)
 		sprintf(s, "%.0f %s", elevation, unit.c_str());
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

void ElevationTextShape::clear(void) {
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

bool ElevationBarShape::updated(const TelemetryData &data) const {
	if (is_initialized_) {
		if ((data.type() == TelemetryData::TypeUnchanged))
			return false;

		if (no_value_ && !data.hasValue(TelemetryData::DataElevation))
			return false;
	}

	return true;
}


void ElevationBarShape::initialize(cairo_t *cr) {
	if (is_initialized_)
		return;

	int x, y;
	int width, height;

	int left, right;
	int top, bottom;

	char s[128];

	TextShape::Font font;

	width_ = theme().width();
	height_ = theme().height();

	size_ = std::min(width_, height_);

	// Size
	setSize(width_, height_, size_); //, size_);

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

	tick_label_width_ = 0;
	tick_label_height_ = 0;

	// Tick label space
	if (theme().hasFlag(VideoWidget::Theme::FlagTickLabel)) {
		int amin = theme().valueMin();
		int amax = theme().valueMax();

		int min = amin;
		int max = amax;

		bool first = true;

		int step = tick_mstep_ * tick_step_;

		std::string unit = theme().hasFlag(VideoWidget::Theme::FlagUnit) ?
			widget_->getFriendlyName(widget_->valueUnit()) : "";

		min /= step;
		min *= step;

		max /= step;
		max *= step;

		for (int value = min; value < max + step; value = value + step) {
			int txtx, txty;
			int txtw, txth;

			double factor = (double) theme().tickLabelFontSize() / (double) theme().valueFontSize();

			if (theme().hasFlag(VideoWidget::Theme::FlagUnit) && (first || (value >= max)))
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

			this->extents(cr, font, s, txtx, txty, txtw, txth);

			tick_label_width_ = std::max(tick_label_width_, txtw - txtx);
			tick_label_height_ = std::max(tick_label_height_, txth - txty);

			first = false;
		}
	}

	// Padding
	top = tick_label_height_ / 2;
	bottom = tick_label_height_ / 2;

	setPadding(
		theme().border() + theme().padding(VideoWidget::Theme::PaddingLeft),
		theme().border() + theme().padding(VideoWidget::Theme::PaddingRight),
   		theme().border() + theme().padding(VideoWidget::Theme::PaddingTop) + top,
   		theme().border() + theme().padding(VideoWidget::Theme::PaddingBottom) + bottom);

	is_initialized_ = true;
}


void ElevationBarShape::ticklenwidth(int value, int *offset, int *len, int *width) {
	int length;
	int size = theme().tickSize();

	VideoWidget::Theme::Align align = theme().tickAlign();

	if (value % 10 == 0) {
		*len = size;
		*width = size_ / 64;
		*offset = 0;
	}
	else if (value % 5 == 0) {
		length = size - (size / 6);

		*len = length;
		*width = size_ / 64;
		if (align == VideoWidget::Theme::AlignLeft)
			*offset = -(size - length) / 2;
		else if (align == VideoWidget::Theme::AlignRight)
			*offset = (size - length) / 2;
		else
			*offset = 0;
	}
	else {
		length = size - (size / 3);

		*len = length;
		*width = size_ / 128;
		if (align == VideoWidget::Theme::AlignLeft)
			*offset = -(size - length) / 2;
		else if (align == VideoWidget::Theme::AlignRight)
			*offset = (size - length) / 2;
		else
			*offset = 0;
	}
}


void ElevationBarShape::draw(cairo_t *cr, const TelemetryData &data) {
	char s[128];

	BarShape::Font font;

	int offset = 0;
	int rotate = 0;

//	int top = 0, bottom = 0;

	int amin, amax;

	double xb1, xb2;

	double elevation = data.elevation(widget_->valueUnit());

	std::string unit = theme().hasFlag(VideoWidget::Theme::FlagUnit) ?
		widget_->getFriendlyName(widget_->valueUnit()) : "";

	// Initialize
	initialize(cr);

	// Limit
	amin = theme().valueMin();
	amax = theme().valueMax();

	// Format data
	no_value_ = !data.hasValue(TelemetryData::DataElevation);

	// Apply padding
//	if (theme().hasFlag(VideoWidget::Theme::FlagValue))
//		offset = (size_ / 14);
//	else if (theme().hasFlag(VideoWidget::Theme::FlagTickLabel))
//		offset = (size_ / 14) / 2;
//	else
//		offset = 0;

//	// Update padding
//	setPadding(
//		theme().padding(VideoWidget::Theme::PaddingLeft),
//		theme().padding(VideoWidget::Theme::PaddingRight),
//		theme().padding(VideoWidget::Theme::PaddingTop) + offset + top,
//		theme().padding(VideoWidget::Theme::PaddingBottom) + offset + bottom
//	);

	// Compute gauge position
//	offset = ((size_ / 8) + (size_ / 25)); // gauge width
//	offset /= 2;
	setOffset(-offset);

	// Draw background
	background(cr);

	// Draw gauge background
	if (theme().hasFlag(VideoWidget::Theme::FlagGauge)) {
		bar(cr, 0, 1, theme().gaugeWidth(), theme().gaugeBorder(),
				theme().gaugeBackgroundColor(), theme().gaugeBorderColor());
	}

	// Draw gauge
	if (theme().hasFlag(VideoWidget::Theme::FlagGauge) && (elevation >= amin)) {
		double to = std::min(elevation, (double) amax);
		double from = (to > 0) ? std::max(0, amin) : std::min(0, amax);

		xb1 = scale(amin, amax, from, rotate);
		xb2 = scale(amin, amax, to, rotate);

		bar(cr, xb1, xb2, theme().gaugeWidth() - (2 * theme().gaugeBorder()), 0,
				theme().gaugePrimaryColor());
	}

	// Draw cursor
	if (theme().hasFlag(VideoWidget::Theme::FlagCursor) && ((elevation >= amin) && (elevation <= amax))) {
		double xb = scale(amin, amax, elevation, rotate);

		cursor(cr, xb, theme().gaugeWidth() - (2 * theme().gaugeBorder()), theme().cursorColor());
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
		int min = amin;
		int max = amax;

		bool first = true;

		int step = tick_mstep_ * tick_step_;

		min /= step;
		min *= step;

		max /= step;
		max *= step;

		double distance = theme().tickLabelDistance();

		int tick_width = theme().hasFlag(VideoWidget::Theme::FlagTick) ? theme().tickSize() : 0;
		int gauge_width = theme().hasFlag(VideoWidget::Theme::FlagGauge) ? theme().gaugeWidth() : 0;

		distance += std::max(tick_width / 2, gauge_width / 2);

		for (int value = min; value < max + step; value = value + step) {
			double xb = scale(amin, amax, value, rotate);

			double factor = (double) theme().tickLabelFontSize() / (double) theme().valueFontSize();

			if (theme().hasFlag(VideoWidget::Theme::FlagUnit) && (first || (value >= max)))
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

			ticklabel(cr, xb, distance, font, theme().tickLabelColor(), theme().tickLabelBorderColor(), s);

			first = false;
		}
	}

	// Draw needle & value
	if ((elevation >= amin) && (elevation <= amax)) {
		double xb = scale(amin, amax, elevation, rotate);

		std::string str = std::to_string((int) std::round(elevation));

		font = (TextShape::Font) {
			.size = theme().valueFontSize(),
			.border = theme().valueBorderWidth(),
			.shadow_opacity = theme().valueShadowOpacity(),
			.shadow_distance = theme().valueShadowDistance(),
			.family = theme().valueFontFamily(),
			.align = VideoWidget::Theme::AlignCenter,
			.style = theme().valueFontStyle(),
			.weight = theme().valueFontWeight(),
		};

		value(cr, xb, font, theme().valueColor(), theme().valueBorderColor(), str.c_str());
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
}


void ElevationBarShape::clear(void) {
	BarShape::clear();

	no_value_ = false;

	if (bg_buf_)
		delete bg_buf_;

	if (fg_buf_)
		delete fg_buf_;

	bg_buf_ = NULL;
	fg_buf_ = NULL;
}


ElevationWidget::ElevationWidget(GPXApplication &app)
	: VideoWidget(app, VideoWidget::WidgetElevation) 
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

	ADD_UNIT(TelemetryData::UnitMiles);
	ADD_UNIT(TelemetryData::UnitMeter);
	ADD_UNIT(TelemetryData::UnitFeet);

	setShape(VideoWidget::ShapeText);
}

