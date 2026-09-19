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

	setSize(theme().width(), theme().height());

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
			.linespace = 0.0,
		};

		extents(cr, font, TextShape::TextAlpha, widget_->label().c_str(), x, y, width, height);

		setLabelExtents(x, y, width, height);
	}

	// Value height
	if (theme().hasFlag(VideoWidget::Theme::FlagValue)) {
		std::string txt = std::to_string(888) + "/";

		font = (TextShape::Font) {
			.size = theme().valueFontSize(),
			.border = theme().valueBorderWidth(),
			.shadow_opacity = theme().valueShadowOpacity(),
			.shadow_distance = theme().valueShadowDistance(),
			.family = theme().valueFontFamily(),
			.align = theme().valueHorizontalAlign(),
			.style = theme().valueFontStyle(),
			.weight = theme().valueFontWeight(),
			.linespace = 0.0,
		};

		extents(cr, font, TextShape::TextNumeric, txt.c_str(), x, y, width, height);
		
		setValueExtents(x, y, width, height);
	}

	// Unit height
	if (theme().hasFlag(VideoWidget::Theme::FlagUnit)) {
		std::string txt = widget_->getFriendlyName(widget_->valueUnit());

		TextShape::TextType alphanum = (theme().textOrientation() == VideoWidget::OrientationHorizontal) ? TextShape::TextNumeric : TextShape::TextAlpha;

		font = (TextShape::Font) {
			.size = theme().unitFontSize(),
			.border = theme().unitBorderWidth(),
			.shadow_opacity = theme().unitShadowOpacity(),
			.shadow_distance = theme().unitShadowDistance(),
			.family = theme().unitFontFamily(),
			.align = theme().unitHorizontalAlign(),
			.style = theme().unitFontStyle(),
			.weight = theme().unitFontWeight(),
			.linespace = 0.0,
		};

		extents(cr, font, alphanum, txt.c_str(), x, y, width, height);
		
		setUnitExtents(x, y, width, height);
	}

	is_initialized_ = true;
}


void VerticalSpeedTextShape::draw(cairo_t *cr, const TelemetryData &data) {
	char s[128];

	TextShape::Font font;

	double verticalspeed = data.verticalspeed(widget_->valueUnit());

	// Initialize
	initialize(cr);

	// Format data
	no_value_ = !data.hasValue(TelemetryData::DataVerticalSpeed);

	if (!no_value_)
		sprintf(s, "%.1f", verticalspeed);
	else
		sprintf(s, "--");

	// Draw background
	background(cr, theme().roundCorner());

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
			.linespace = 0.0,
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
			.linespace = 0.0,
		};

		value(cr, font, theme().valueColor(), theme().valueBorderColor(), s);
	}

	// Draw unit
	if (theme().hasFlag(VideoWidget::Theme::FlagUnit)) {
		std::string u = widget_->getFriendlyName(widget_->valueUnit());

		font = (TextShape::Font) {
			.size = theme().unitFontSize(),
			.border = theme().unitBorderWidth(),
			.shadow_opacity = theme().unitShadowOpacity(),
			.shadow_distance = theme().unitShadowDistance(),
			.family = theme().unitFontFamily(),
			.align = theme().unitHorizontalAlign(),
			.style = theme().unitFontStyle(),
			.weight = theme().unitFontWeight(),
			.linespace = 0.0,
		};

		unit(cr, font, theme().unitColor(), theme().unitBorderColor(), u.c_str());
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

	int left, right;
	int top, bottom;

	int terminaison;

	char s[128];

	double value_min = 0, value_max = 0;

	TelemetrySource *source;

	TextShape::Font font;

	// Icon
	icon_filename_ = widget_->getIconFilename(theme().icon());

	// Last point
	source = widget_->telemetrySource();

	if (source) {
		TelemetryData data;

		source->retrieveLast(data);

		value_min = data.verticalspeed(widget_->valueUnit(), TelemetryData::RangeMin);
		value_max = data.verticalspeed(widget_->valueUnit(), TelemetryData::RangeMax);

		value_min = std::floor(value_min / 10.0);
		value_max = std::ceil(value_max / 10.0);

		value_min -= ((int) value_min % 2);
		value_max += ((int) value_max % 2);

		value_min *= 10.0;
		value_max *= 10.0;
	}

	// Range auto disabled
	if (!theme().valueMinAuto())
		value_min = theme().valueMin();

	if (!theme().valueMaxAuto())
		value_max = theme().valueMax();

	// Size
	setOrientation(theme().gaugeOrientation());
	setSize(theme().width(), theme().height());

	// Data range
	setValueRange(value_min, value_max);

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
			.linespace = 0.0,
		};

		extents(cr, font, BarShape::TextAlpha, widget_->label().c_str(), x, y, width, height);

		setLabelExtents(x, y, width, height);
	}

	// Tick
	tick_step_ = 10;
	tick_mstep_ = 10;

	tick_label_width_ = 0;
	tick_label_height_ = 0;

	// Tick label space
	if (theme().hasFlag(VideoWidget::Theme::FlagTickLabel)) {
		int min = value_min;
		int max = value_max;

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

			if (value < value_min)
				continue;

			font = (TextShape::Font) {
				.size = theme().valueFontSize() * factor,
				.border = theme().valueBorderWidth(),
				.shadow_opacity = theme().valueShadowOpacity(),
				.shadow_distance = theme().valueShadowDistance(),
				.family = theme().valueFontFamily(),
				.align = VideoWidget::Theme::AlignCenter,
				.style = theme().valueFontStyle(),
				.weight = theme().valueFontWeight(),
				.linespace = 0.0,
			};

			this->extents(cr, font, BarShape::TextAlpha, s, txtx, txty, txtw, txth);

			tick_label_width_ = std::max(tick_label_width_, txtw - txtx);
			tick_label_height_ = std::max(tick_label_height_, txth - txty);

			first = false;
		}
	}

	// Bar with round terminaison
	terminaison = (theme().gaugeCap() == VideoWidget::Theme::GaugeCapRound) ? size2pixels(theme().gaugeWidth() / 2) : 0;

	// Padding
	if (theme().gaugeOrientation() == VideoWidget::OrientationHorizontal) {
		left = std::max(tick_label_width_ / 2, terminaison);
		right = std::max(tick_label_width_ / 2, terminaison);
		top = 0;
		bottom = 0;
	}
	else {
		left = 0;
		right = 0;
		top = std::max(tick_label_height_ / 2, terminaison);
		bottom = std::max(tick_label_height_ / 2, terminaison);
	}

	setPadding(
		theme().border() + theme().padding(VideoWidget::Theme::PaddingLeft) + left,
		theme().border() + theme().padding(VideoWidget::Theme::PaddingRight) + right,
   		theme().border() + theme().padding(VideoWidget::Theme::PaddingTop) + top,
   		theme().border() + theme().padding(VideoWidget::Theme::PaddingBottom) + bottom);

	is_initialized_ = true;
}


void VerticalSpeedBarShape::ticklenwidth(int value, double *offset, double *len, double *width) {
	int length;
	int ticksize = theme().tickSize();

	VideoWidget::Theme::Align align = theme().tickAlign();

	if (value % 10 == 0) {
		*len = ticksize;
		*width = size() / 64;
		*offset = 0;
	}
	else if (value % 5 == 0) {
		length = ticksize - (ticksize / 6);

		*len = length;
		*width = size() / 64;
		if (align == VideoWidget::Theme::AlignLeft)
			*offset = -(ticksize - length) / 2;
		else if (align == VideoWidget::Theme::AlignRight)
			*offset = (ticksize - length) / 2;
		else
			*offset = 0;
	}
	else {
		length = ticksize - (ticksize / 3);

		*len = length;
		*width = size() / 128;
		if (align == VideoWidget::Theme::AlignLeft)
			*offset = -(ticksize - length) / 2;
		else if (align == VideoWidget::Theme::AlignRight)
			*offset = (ticksize - length) / 2;
		else
			*offset = 0;
	}
}


void VerticalSpeedBarShape::draw(cairo_t *cr, const TelemetryData &data) {
	char s[128];

	BarShape::Font font;

	int rotate = 0;

	double value_min, value_max;

	double xb1, xb2;

	double verticalspeed = data.verticalspeed(widget_->valueUnit());

	std::string unit = theme().hasFlag(VideoWidget::Theme::FlagUnit) ?
		widget_->getFriendlyName(widget_->valueUnit()) : "";

	// Initialize
	initialize(cr);

	// Range
	getValueRange(value_min, value_max);

	// Format data
	no_value_ = !data.hasValue(TelemetryData::DataVerticalSpeed);

	// Compute gauge position
	setOffset(theme().gaugeOffset());

	// Draw background
	background(cr, theme().roundCorner());

	// Draw gauge background
	if (theme().hasFlag(VideoWidget::Theme::FlagGauge)) {
		bar(cr, 0, 1, theme().gaugeWidth(), theme().gaugeBorder(),
				theme().gaugeBackgroundColor(), theme().gaugeBorderColor());
	}

	// Draw gauge
	if (theme().hasFlag(VideoWidget::Theme::FlagGauge) && (verticalspeed >= value_min)) {
		double to = std::min(verticalspeed, (double) value_max);
		double from = (to > 0) ? std::max(0.0, value_min) : std::min(0.0, value_max);

		xb1 = scale(value_min, value_max, from, rotate);
		xb2 = scale(value_min, value_max, to, rotate);

		bar(cr, xb1, xb2, theme().gaugeWidth() - (2 * theme().gaugeBorder()), 0,
				theme().gaugePrimaryColor());
	}

	// Draw tick lines on bar
	if (theme().hasFlag(VideoWidget::Theme::FlagTick)) {
		for (int value = value_min; value < value_max + tick_step_; value = value + tick_step_) {
			double ticklen;
			double tickwidth;
			double tickoffset;

			double xb = scale(value_min, value_max, value, rotate);

			ticklenwidth(value / tick_mstep_, &tickoffset, &ticklen, &tickwidth);

			line(cr, xb, tickoffset, tickoffset + ticklen, tickwidth, theme().tickColor());
		}
	}

	// Draw tick label
	if (theme().hasFlag(VideoWidget::Theme::FlagTickLabel)) {
		int min = value_min;
		int max = value_max;

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
			double xb = scale(value_min, value_max, value, rotate);

			double factor = (double) theme().tickLabelFontSize() / (double) theme().valueFontSize();

			if (theme().hasFlag(VideoWidget::Theme::FlagUnit) && (first || (value >= max)))
				sprintf(s, "%d %s", value, unit.c_str());
			else
				sprintf(s, "%d", value);

			if (value < value_min)
				continue;

			font = (TextShape::Font) {
				.size = theme().valueFontSize() * factor,
				.border = theme().valueBorderWidth(),
				.shadow_opacity = theme().valueShadowOpacity(),
				.shadow_distance = theme().valueShadowDistance(),
				.family = theme().valueFontFamily(),
				.align = VideoWidget::Theme::AlignCenter,
				.style = theme().valueFontStyle(),
				.weight = theme().valueFontWeight(),
				.linespace = 0.0,
			};

			ticklabel(cr, xb, distance, font, theme().tickLabelColor(), theme().tickLabelBorderColor(), s);

			first = false;
		}
	}

	// Draw cursor
	if (theme().hasFlag(VideoWidget::Theme::FlagCursor) && ((verticalspeed >= value_min) && (verticalspeed <= value_max))) {
		double xb = scale(value_min, value_max, verticalspeed, rotate);

		cursor(cr, xb, theme().cursorWidth(), theme().cursorColor());
	}

	// Value
	std::string str = std::to_string((int) std::round(verticalspeed));

	font = (TextShape::Font) {
		.size = theme().valueFontSize(),
		.border = theme().valueBorderWidth(),
		.shadow_opacity = theme().valueShadowOpacity(),
		.shadow_distance = theme().valueShadowDistance(),
		.family = theme().valueFontFamily(),
		.align = VideoWidget::Theme::AlignCenter,
		.style = theme().valueFontStyle(),
		.weight = theme().valueFontWeight(),
		.linespace = 0.0,
	};

	// Draw needle / icon / value
	if ((verticalspeed >= value_min) && (verticalspeed <= value_max)) {
		double xb = scale(value_min, value_max, verticalspeed, rotate);

		if (theme().hasFlag(VideoWidget::Theme::FlagNeedle))
			needle(cr, theme().needleType(),
					xb, font, str.c_str(),
					theme().needleBorder(), theme().needleBackgroundColor(), theme().needleBorderColor());

		switch (theme().needleType()) {
		case VideoWidget::Theme::NeedleTypeIcon:
			icon(cr, xb, icon_filename_, theme().iconColor());
			break;

		case VideoWidget::Theme::NeedleTypeValue:
			value(cr, xb, font, theme().valueColor(), theme().valueBorderColor(), str.c_str());
			break;

		case VideoWidget::Theme::NeedleTypeBasic:
		default:
			if (theme().hasFlag(VideoWidget::Theme::FlagValue))
				value(cr, xb, font, theme().valueColor(), theme().valueBorderColor(), str.c_str());
			break;
		}
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
			.linespace = 0.0,
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


VerticalSpeedWidget::VerticalSpeedWidget(GPXApplication &app, TelemetrySource *source)
	: VideoWidget(app, VideoWidget::WidgetVerticalSpeed, source) 
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

