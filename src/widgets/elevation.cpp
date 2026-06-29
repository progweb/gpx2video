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
		};

		extents(cr, font, widget_->label().c_str(), x, y, width, height);

		setLabelExtents(x, y, width, height);
	}

	// Value height
	if (theme().hasFlag(VideoWidget::Theme::FlagValue)) {
		std::string txt = std::to_string(888);

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

		extents(cr, font, txt.c_str(), x, y, width, height);
		
		setValueExtents(x, y, width, height);
	}


	// Unit height
	if (theme().hasFlag(VideoWidget::Theme::FlagUnit)) {
		std::string txt = widget_->getFriendlyName(widget_->valueUnit());

		font = (TextShape::Font) {
			.size = theme().unitFontSize(),
			.border = theme().valueBorderWidth(),
			.shadow_opacity = theme().valueShadowOpacity(),
			.shadow_distance = theme().valueShadowDistance(),
			.family = theme().valueFontFamily(),
			.align = theme().valueHorizontalAlign(),
			.style = theme().valueFontStyle(),
			.weight = theme().valueFontWeight(),
		};

		extents(cr, font, txt.c_str(), x, y, width, height);
		
		setUnitExtents(x, y, width, height);
	}
	is_initialized_ = true;
}


void ElevationTextShape::draw(cairo_t *cr, const TelemetryData &data) {
	char s[128];

	TextShape::Font font;

	double elevation = data.elevation(widget_->valueUnit());

	// Initialize
	initialize(cr);

	// Format data
	no_value_ = !data.hasValue(TelemetryData::DataElevation);

 	if (!no_value_)
 		sprintf(s, "%.0f", elevation);
 	else
 		sprintf(s, "--");

	// Restore surface
	if (!restoreCairoSurface(cr)) {
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
			};

			label(cr, font, theme().labelColor(), theme().labelBorderColor(), widget_->label().c_str());
		}

		// Save surface
		saveCairoSurface(cr);
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

	// Draw unit
	if (theme().hasFlag(VideoWidget::Theme::FlagUnit)) {
		std::string u = widget_->getFriendlyName(widget_->valueUnit());

		font = (TextShape::Font) {
			.size = theme().unitFontSize(),
			.border = theme().valueBorderWidth(),
			.shadow_opacity = theme().valueShadowOpacity(),
			.shadow_distance = theme().valueShadowDistance(),
			.family = theme().valueFontFamily(),
			.align = theme().valueHorizontalAlign(),
			.style = theme().valueFontStyle(),
			.weight = theme().valueFontWeight(),
		};

		unit(cr, font, theme().valueColor(), theme().valueBorderColor(), u.c_str());
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

	int terminaison;

	char s[128];

	TextShape::Font font;

	// Icon
	icon_filename_ = widget_->getIconFilename(theme().icon());

	// Size
	setOrientation(theme().gaugeOrientation());
	setSize(theme().width(), theme().height());

	// Label width / height
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
				.size = theme().valueFontSize() * factor,
				.border = theme().valueBorderWidth(),
				.shadow_opacity = theme().valueShadowOpacity(),
				.shadow_distance = theme().valueShadowDistance(),
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


void ElevationBarShape::ticklenwidth(int value, double *offset, double *len, double *width) {
	double length;
	double ticksize = theme().tickSize();

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


void ElevationBarShape::draw(cairo_t *cr, const TelemetryData &data) {
	char s[128];

	BarShape::Font font;

	int rotate = 0;

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

	// Compute gauge position
	setOffset(theme().gaugeOffset());

	// Restore surface
	if (!restoreCairoSurface(cr)) {
		// Draw background
		background(cr, theme().roundCorner());

		// Draw gauge background
		if (theme().hasFlag(VideoWidget::Theme::FlagGauge)) {
			bar(cr, 0, 1, theme().gaugeWidth(), theme().gaugeBorder(),
					theme().gaugeBackgroundColor(), theme().gaugeBorderColor());
		}


		// Draw label
		if (theme().hasFlag(VideoWidget::Theme::FlagLabel)) {
			font = (BarShape::Font) {
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

		// Save surface
		saveCairoSurface(cr);
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

	// Draw tick lines on bar
	if (theme().hasFlag(VideoWidget::Theme::FlagTick)) {
		for (int value = amin; value < amax + tick_step_; value = value + tick_step_) {
			double ticklen;
			double tickwidth;
			double tickoffset;

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

			font = (BarShape::Font) {
				.size = theme().valueFontSize() * factor,
				.border = theme().valueBorderWidth(),
				.shadow_opacity = theme().valueShadowOpacity(),
				.shadow_distance = theme().valueShadowDistance(),
				.family = theme().valueFontFamily(),
				.align = VideoWidget::Theme::AlignCenter,
				.style = theme().valueFontStyle(),
				.weight = theme().valueFontWeight(),
			};

			ticklabel(cr, xb, distance, font, theme().tickLabelColor(), theme().tickLabelBorderColor(), s);

			first = false;
		}
	}

	// Draw cursor
	if (theme().hasFlag(VideoWidget::Theme::FlagCursor) && ((elevation >= amin) && (elevation <= amax))) {
		double xb = scale(amin, amax, elevation, rotate);

		cursor(cr, xb, theme().cursorWidth(), theme().cursorColor());
	}

	// Value
	std::string str = std::to_string((int) std::round(elevation));

	font = (BarShape::Font) {
		.size = theme().valueFontSize(),
		.border = theme().valueBorderWidth(),
		.shadow_opacity = theme().valueShadowOpacity(),
		.shadow_distance = theme().valueShadowDistance(),
		.family = theme().valueFontFamily(),
		.align = VideoWidget::Theme::AlignCenter,
		.style = theme().valueFontStyle(),
		.weight = theme().valueFontWeight(),
	};

	// Draw needle / icon / value
	if ((elevation >= amin) && (elevation <= amax)) {
		double xb = scale(amin, amax, elevation, rotate);

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


/**
 * Chart shape
 */

bool ElevationChartShape::updated(const TelemetryData &data) const {
	if (is_initialized_) {
		if ((data.type() == TelemetryData::TypeUnchanged))
			return false;

		if (no_value_ && !data.hasValue(TelemetryData::DataElevation))
			return false;
	}

	return true;
}


void ElevationChartShape::initialize(cairo_t *cr) {
	if (is_initialized_)
		return;
	
	int left, right;
	int top, bottom;

	double x_min = 0, x_max = 0;
	double y_min = 0, y_max = 0;

	TelemetrySource *source;

	(void) cr;

	// Icon
	icon_filename_ = widget_->getIconFilename(theme().icon());

	// Last point
	source = widget_->telemetrySource();

	if (source) {
		TelemetryData data;

		source->retrieveLast(data);

		x_min = data.distance(TelemetryData::UnitKm, TelemetryData::RangeMin);
		x_max = data.distance(TelemetryData::UnitKm, TelemetryData::RangeMax);

		y_min = data.elevation(TelemetryData::UnitMeter, TelemetryData::RangeMin);
		y_max = data.elevation(TelemetryData::UnitMeter, TelemetryData::RangeMax);
	}

	// Size
	setSize(theme().width(), theme().height());

	// Data range
	setXRange(x_min, x_max);
	setYRange(y_min, y_max);

	// Padding
	left = right = 0;
	top = bottom = 0;

	setPadding(
		theme().border() + theme().padding(VideoWidget::Theme::PaddingLeft) + left,
		theme().border() + theme().padding(VideoWidget::Theme::PaddingRight) + right,
   		theme().border() + theme().padding(VideoWidget::Theme::PaddingTop) + top,
   		theme().border() + theme().padding(VideoWidget::Theme::PaddingBottom) + bottom);

	is_initialized_ = true;
}


void ElevationChartShape::draw(cairo_t *cr, const TelemetryData &data) {
	double x, y;

	ChartShape::Font font;

	double x_min, x_max;
	double y_min, y_max;

	double thick = size2pixels(theme().curveThick());
	double border = size2pixels(theme().curveBorder());

	const float *color = theme().curveColor();
	const float *outline = theme().curveBorderColor();
	const float *fill = theme().curveFillColor();

	struct curve *curve;

	TelemetryData wpt;

	TelemetrySource *source;

	enum TelemetrySource::Data result;

	double elevation = data.elevation(widget_->valueUnit());

	// Initialize
	initialize(cr);

	// Range
	getXRange(x_min, x_max);
	getYRange(y_min, y_max);

	// Restore surface
	if (!restoreCairoSurface(cr)) {
		// Draw background
		background(cr, theme().roundCorner());

		// Draw axis
		if (theme().hasFlag(VideoWidget::Theme::FlagAxis))
			axis(cr, theme().axisThick(), theme().axisBorder(), theme().axisColor(), theme().axisBorderColor());

		// Draw curve
		source = widget_->telemetrySource();

		curve_create(&curve, cr, thick, border, color, outline, fill);

		if (source) {
			for (result = source->retrieveFirst(wpt); result != TelemetrySource::DataEof; result = source->retrieveNext(wpt)) {
				double d = wpt.distance(TelemetryData::UnitKm);
				double e = wpt.elevation(widget_->valueUnit());

				// Compute position
				x = scale(x_min, x_max, d);
				y = scale(y_min, y_max, e);

				curve_point(curve, x, y);
			}
		}

		curve_stroke(curve);
		curve_free(curve);

		// Draw unit
		if (theme().hasFlag(VideoWidget::Theme::FlagUnit)) {
			double factor = (double) theme().unitFontSize() / (double) theme().valueFontSize();

			std::string str = theme().hasFlag(VideoWidget::Theme::FlagUnit) ?
				widget_->getFriendlyName(widget_->valueUnit()) : "";

			font = (ChartShape::Font) {
				.size = theme().valueFontSize() * factor,
				.border = theme().valueBorderWidth(),
				.shadow_opacity = theme().valueShadowOpacity(),
				.shadow_distance = theme().valueShadowDistance(),
				.family = theme().valueFontFamily(),
				.align = theme().valueHorizontalAlign(),
				.style = theme().valueFontStyle(),
				.weight = theme().valueFontWeight(),
			};

			unit(cr, font, theme().valueColor(), theme().valueBorderColor(), str.c_str());
		}

		// Save surface
		saveCairoSurface(cr);
	}

	// Draw cursor
	if (theme().hasFlag(VideoWidget::Theme::FlagCursor) && ((elevation >= y_min) && (elevation <= y_max))) {
		// Compute position
		x = scale(x_min, x_max, data.distance(TelemetryData::UnitKm));
		y = scale(y_min, y_max, data.elevation(widget_->valueUnit()));

		cursor(cr, x, y, theme().cursorWidth(), theme().cursorColor());
	}

	// Value
	std::string str = std::to_string((int) std::round(elevation));

	font = (BarShape::Font) {
		.size = theme().valueFontSize(),
		.border = theme().valueBorderWidth(),
		.shadow_opacity = theme().valueShadowOpacity(),
		.shadow_distance = theme().valueShadowDistance(),
		.family = theme().valueFontFamily(),
		.align = VideoWidget::Theme::AlignCenter,
		.style = theme().valueFontStyle(),
		.weight = theme().valueFontWeight(),
	};

	if (theme().hasFlag(VideoWidget::Theme::FlagValue)) {
		y = scale(y_min, y_max, data.elevation(widget_->valueUnit()));

		value(cr, y, font, theme().valueColor(), theme().valueBorderColor(), str.c_str());
	}

	// Draw needle / icon / value
	if (theme().hasFlag(VideoWidget::Theme::FlagNeedle)) {
		if ((elevation >= y_min) && (elevation <= y_max)) {
			// Compute position
			x = scale(x_min, x_max, data.distance(TelemetryData::UnitKm));
			y = scale(y_min, y_max, data.elevation(widget_->valueUnit()));

			needle(cr, theme().needleType(),
					x, y, font, str.c_str(),
					theme().needleBorder(), theme().needleBackgroundColor(), theme().needleBorderColor());

			switch (theme().needleType()) {
			case VideoWidget::Theme::NeedleTypeIcon:
				icon(cr, x, y, icon_filename_, theme().iconColor());
				break;

			case VideoWidget::Theme::NeedleTypeValue:
				value(cr, x, y, font, theme().valueColor(), theme().valueBorderColor(), str.c_str());
				break;

			case VideoWidget::Theme::NeedleTypeBasic:
			default:
				if (theme().hasFlag(VideoWidget::Theme::FlagValue))
					value(cr, x, y, font, theme().valueColor(), theme().valueBorderColor(), str.c_str());
				break;
			}
		}
	}
}


void ElevationChartShape::clear(void) {
	ChartShape::clear();

	no_value_ = false;

	if (bg_buf_)
		delete bg_buf_;

	if (fg_buf_)
		delete fg_buf_;

	bg_buf_ = NULL;
	fg_buf_ = NULL;
}


ElevationWidget::ElevationWidget(GPXApplication &app, TelemetrySource *source)
	: VideoWidget(app, VideoWidget::WidgetElevation, source) 
	, ShapeBase(VideoWidget::theme())
	, shape_(NULL) {

#define ADD_SHAPE(shape) \
	shapes_supported_.push_back((VideoWidget::ListItem) { \
		shape, VideoWidget::getFriendlyName(shape), VideoWidget::shape2string(shape) \
	})

	ADD_SHAPE(VideoWidget::ShapeBar);
	ADD_SHAPE(VideoWidget::ShapeChart);
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

