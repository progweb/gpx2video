#include "speed.h"


/**
 * Text shape
 */

bool SpeedTextShape::updated(const TelemetryData &data) const {
	if (is_initialized_) {
		if ((data.type() == TelemetryData::TypeUnchanged))
			return false;

		if (no_value_ && !data.hasValue(TelemetryData::DataSpeed))
			return false;
	}

	return true;
}


void SpeedTextShape::initialize(cairo_t *cr) {
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
		};

		extents(cr, font, txt.c_str(), x, y, width, height);
		
		setValueExtents(x, y, width, height);
	}

	// Unit height
	if (theme().hasFlag(VideoWidget::Theme::FlagUnit)) {
		std::string u = widget_->getFriendlyName(widget_->valueUnit());

		font = (ArcShape::Font) {
			.size = theme().unitFontSize(),
			.border = theme().valueBorderWidth(),
			.shadow_opacity = theme().valueShadowOpacity(),
			.shadow_distance = theme().valueShadowDistance(),
			.family = theme().valueFontFamily(),
			.align = theme().valueHorizontalAlign(),
			.style = theme().valueFontStyle(),
			.weight = theme().valueFontWeight(),
		};

		extents(cr, font, u.c_str(), x, y, width, height);
		
		setUnitExtents(x, y, width, height);
	}

	is_initialized_ = true;
}


void SpeedTextShape::draw(cairo_t *cr, const TelemetryData &data) {
	char s[128];

	TextShape::Font font;

	bool pace_unit = false;
	double speed = data.speed(widget_->valueUnit());

	// Initialize
	initialize(cr);

	// Format data
	no_value_ = !data.hasValue(TelemetryData::DataSpeed);

	switch (widget_->valueUnit()) {
	case TelemetryData::UnitMinPerMile:
	case TelemetryData::UnitMinPerKm:
		pace_unit = true;
		break;
	default:
		pace_unit = false;
		break;
	}

	if (pace_unit) {
		if (speed <= 0.0)
			no_value_ = true;
	}

	if (no_value_)
		sprintf(s, "--");
	else if (pace_unit) {
		int min = (int) speed;
		int sec = (int) round((speed - min) * 60) % 60;

		sprintf(s, "%d:%02d", min, sec);
	} 
	else
		sprintf(s, "%.1f", speed);

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


void SpeedTextShape::clear(void) {
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
 * Arc shape
 */

bool SpeedArcShape::updated(const TelemetryData &data) const {
	if (is_initialized_) {
		if ((data.type() == TelemetryData::TypeUnchanged))
			return false;

		if (no_value_ && !data.hasValue(TelemetryData::DataSpeed))
			return false;
	}

	return true;
}


void SpeedArcShape::initialize(cairo_t *cr) {
	if (is_initialized_)
		return;

	int x, y;
	int width, height;

	ArcShape::Font font;

	(void) cr;

	setSize(theme().width(), theme().height());

	setPadding(
		theme().border() + theme().padding(VideoWidget::Theme::PaddingLeft),
		theme().border() + theme().padding(VideoWidget::Theme::PaddingRight),
   		theme().border() + theme().padding(VideoWidget::Theme::PaddingTop),
   		theme().border() + theme().padding(VideoWidget::Theme::PaddingBottom));

	// Value height
	if (theme().hasFlag(VideoWidget::Theme::FlagValue)) {
		std::string txt = std::to_string(888);

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

	// Unit height
	if (theme().hasFlag(VideoWidget::Theme::FlagUnit)) {
		std::string u = widget_->getFriendlyName(widget_->valueUnit());

		font = (ArcShape::Font) {
			.size = theme().unitFontSize(),
			.border = theme().valueBorderWidth(),
			.shadow_opacity = theme().valueShadowOpacity(),
			.shadow_distance = theme().valueShadowDistance(),
			.family = theme().valueFontFamily(),
			.align = theme().valueHorizontalAlign(),
			.style = theme().valueFontStyle(),
			.weight = theme().valueFontWeight(),
		};

		extents(cr, font, u.c_str(), x, y, width, height);
		
		setUnitExtents(x, y, width, height);
	}

	is_initialized_ = true;
}


void SpeedArcShape::tickinit(int min, int max) {
	int n;

	double range;

	range = end() - start();

	// Compute arc step
	// n markers for range [start:end] (ex. 12 [30:330]
	n = 12 * range / 300.0;

	tick_step_ = ((max - min) / n) / 4;
	tick_step_ = std::ceil(tick_step_ / 5.0) * 5;
	tick_step_ = std::max(1, tick_step_);

	if (((max - min) / tick_step_) < (10 * range / 300.0))
		tick_mstep_ = 1;
	else if (((max - min) / tick_step_) < (30 * range / 300.0))
		tick_mstep_ = 2;
	else
		tick_mstep_ = 4;
}


void SpeedArcShape::ticklenwidth(int value, double *len, double *width) {
	double ticksize = theme().tickSize();

	if (value % 10 == 0) {
		*len = ticksize;
		*width = size() / 128;
	}
	else if (value % 5 == 0) {
		*len = ticksize - (ticksize / 3);
		*width = size() / 128;
	}
	else {
		*len = ticksize - (ticksize / 6);
		*width = size() / 256;
	}
}


void SpeedArcShape::draw(cairo_t *cr, const TelemetryData &data) {
	int rotate = 180;

	ArcShape::Font font;

	double xa1, xa2;

	bool pace_unit = false;

	int vmin, vmax;

	double speed = data.speed(widget_->valueUnit());
	double avgspeed = data.avgridespeed(widget_->valueUnit());
	double maxspeed = data.maxspeed(widget_->valueUnit());

	// Initialize
	initialize(cr);

	// Limit
	vmin = theme().valueMin();
	vmax = theme().valueMax();

	// Arc size
	int arc_size = theme().gaugeAngle();
	int arc_rotate = theme().gaugeRotation();

	rotate += arc_rotate;

	setArcRange(180 - (arc_size / 2), 180 + (arc_size / 2));

	// Tick init
	tickinit(vmin, vmax);

	// Format data
	no_value_ = !data.hasValue(TelemetryData::DataSpeed);

	switch (widget_->valueUnit()) {
	case TelemetryData::UnitMinPerMile:
	case TelemetryData::UnitMinPerKm:
		pace_unit = true;
		break;
	default:
		pace_unit = false;
		break;
	}

	if (pace_unit) {
		if (speed <= 0.0)
			no_value_ = true;
	}

	// Draw arc background
	pieslice(cr, 0, 360, theme().border(),
			theme().backgroundColor(), theme().borderColor());

	// Draw gauge background
	if (theme().hasFlag(VideoWidget::Theme::FlagGauge)) {
		double width = theme().gaugeWidth();
		double border = theme().gaugeBorder();

		double width1, width2;

		VideoWidget::Theme::GaugeCap cap = theme().gaugeCap();

		xa1 = scale(vmin, vmax, 0, rotate);
		xa2 = scale(vmin, vmax, vmax, rotate);

		switch (cap) {
		case VideoWidget::Theme::GaugeCapRound:
			width1 = (theme().gaugePrimaryColor()[3] > 0) ? width - border : 0;
			width2 = (theme().gaugeSecondaryColor()[3] > 0) ? width - border : 0;

			// Draw background
			arc(cr, xa1, xa2, border / 2.0, width, border,
					theme().gaugeBackgroundColor(), theme().gaugeBorderColor());

			// Draw color gauge (max speed) - width / 2.6
			if ((width2 > 0) && data.hasValue(TelemetryData::DataMaxSpeed)) {
				xa1 = scale(vmin, vmax, 0, rotate);
				xa2 = scale(vmin, vmax, maxspeed, rotate);

				arc(cr, xa1, xa2, border, width2, 0.0,
						theme().gaugeSecondaryColor());
			}

			if ((width1 > 0) && theme().hasFlag(VideoWidget::Theme::FlagNeedle)) {
				// Draw color gauge (avg speed)
				if (data.hasValue(TelemetryData::DataAverageRideSpeed)) {
					xa1 = scale(vmin, vmax, 0, rotate);
					xa2 = scale(vmin, vmax, avgspeed, rotate);

					arc(cr, xa1, xa2, border, width1, 0.0,
							theme().gaugePrimaryColor());
				}
			}
			else {
				// Draw color gauge (speed)
				if (data.hasValue(TelemetryData::DataSpeed)) {
					xa1 = scale(vmin, vmax, 0, rotate);
					xa2 = scale(vmin, vmax, speed, rotate);

					arc(cr, xa1, xa2, border, width1, 0.0,
							theme().gaugePrimaryColor());
				}
			}

			break;

		case VideoWidget::Theme::GaugeCapSquare:
		default:
			if ((theme().gaugePrimaryColor()[3] > 0) && (theme().gaugeSecondaryColor()[3] > 0)) {
				width1 = width - (border / 2.0);
				width2 = (width / 2.6) - (border / 2.0);
			}
			else if (theme().gaugePrimaryColor()[3] > 0) {
				width1 = width - border;
				width2 = 0;
			}
			else if (theme().gaugeSecondaryColor()[3] > 0) {
				width1 = 0;
				width2 = width - border;
			}
			else {
				width1 = 0;
				width2 = 0;
			}

			// Draw background
			arc(cr, xa1, xa2, (border / 2.0), width, border,
					theme().gaugeBackgroundColor(), theme().gaugeBorderColor());

			if (theme().hasFlag(VideoWidget::Theme::FlagNeedle)) {
				// Draw color gauge (avg speed) - width / 1.625
				if ((width1 > 0) && data.hasValue(TelemetryData::DataAverageRideSpeed)) {
					xa1 = scale(vmin, vmax, 0, rotate);
					xa2 = scale(vmin, vmax, avgspeed, rotate);

					arc(cr, xa1, xa2, border, width1 - (border / 2.0), 0.0,
							theme().gaugePrimaryColor());
				}
			}
			else {
				// Draw color gauge (speed) - width / 1.625
				if ((width1 > 0) && data.hasValue(TelemetryData::DataSpeed)) {
					xa1 = scale(vmin, vmax, 0, rotate);
					xa2 = scale(vmin, vmax, speed, rotate);

					arc(cr, xa1, xa2, border, width1, 0.0,
							theme().gaugePrimaryColor());
				}
			}

			// Draw color gauge (max speed) - width / 2.6
			if ((width2 > 0) && data.hasValue(TelemetryData::DataMaxSpeed)) {
				xa1 = scale(vmin, vmax, 0, rotate);
				xa2 = scale(vmin, vmax, maxspeed, rotate);

				arc(cr, xa1, xa2, border, width2, 0.0,
						theme().gaugeSecondaryColor());
			}

			break;
		}
	}

	// Draw tick lines around arc line
	if (theme().hasFlag(VideoWidget::Theme::FlagTick)) {
		for (int value = vmin; value < vmax + tick_step_; value = value + tick_step_) {
			double ticklen;
			double tickwidth;

			double xa = scale(vmin, vmax, value, rotate);

			if (xa > (end() + rotate))
				break;

			ticklenwidth(value, &ticklen, &tickwidth);

			line(cr, xa, 0, ticklen, tickwidth, theme().tickColor());
		}
	}

	// Draw tick label
	if (theme().hasFlag(VideoWidget::Theme::FlagTickLabel)) {
		double distance = std::max(theme().gaugeWidth() / 2.0, theme().tickSize() / 2.0);
		
		distance += theme().tickLabelDistance();

		for (int value = vmin; value < vmax + (tick_mstep_ * tick_step_); value = value + (tick_mstep_ * tick_step_)) {
			double xa = scale(vmin, vmax, value, rotate);

			double factor = (double) theme().tickLabelFontSize() / (double) theme().valueFontSize();

			if (xa > (end() + rotate))
				break;

			std::string str = std::to_string(value);

			font = (ArcShape::Font) {
				.size = theme().valueFontSize() * factor,
				.border = theme().valueBorderWidth(),
				.shadow_opacity = theme().valueShadowOpacity(),
				.shadow_distance = theme().valueShadowDistance(),
				.family = theme().valueFontFamily(),
				.align = VideoWidget::Theme::AlignCenter,
				.style = theme().valueFontStyle(),
				.weight = theme().valueFontWeight(),
			};

			ticklabel(cr, xa, distance, font, theme().tickLabelColor(), theme().tickLabelBorderColor(), str.c_str());
		}
	}

	// Write speed value
	if (theme().hasFlag(VideoWidget::Theme::FlagValue)) {
		char s[128];

		if (no_value_)
			sprintf(s, "--");
		else if (pace_unit) {
			int min = (int) speed;
			int sec = (int) round((speed - min) * 60) % 60;

			sprintf(s, "%d:%02d", min, sec);
		} 
		else
			sprintf(s, "%d", int(std::round(speed)));

		font = (ArcShape::Font) {
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

	// Write speed unit
	if (theme().hasFlag(VideoWidget::Theme::FlagUnit)) {
		std::string u = widget_->getFriendlyName(widget_->valueUnit());

		double factor = (double) theme().unitFontSize() / (double) theme().valueFontSize();

		font = (ArcShape::Font) {
			.size = theme().valueFontSize() * factor,
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

	// Draw needle
	if (theme().hasFlag(VideoWidget::Theme::FlagNeedle)) {
		double xa = scale(vmin, vmax, !no_value_ ? speed : 0, rotate);

		needle(cr, theme().needleType(), xa, 0, 
				true, theme().needlePrimaryColor(), theme().needleSecondaryColor());
	}
}


void SpeedArcShape::clear(void) {
	ArcShape::clear();

	no_value_ = false;

	if (bg_buf_)
		delete bg_buf_;

	if (fg_buf_)
		delete fg_buf_;

	bg_buf_ = NULL;
	fg_buf_ = NULL;
}


SpeedWidget::SpeedWidget(GPXApplication &app, TelemetrySource *source)
	: VideoWidget(app, VideoWidget::WidgetSpeed, source) 
	, ShapeBase(VideoWidget::theme())
	, shape_(NULL) {

#define ADD_SHAPE(shape) \
	shapes_supported_.push_back((VideoWidget::ListItem) { \
		shape, VideoWidget::getFriendlyName(shape), VideoWidget::shape2string(shape) \
	})

	ADD_SHAPE(VideoWidget::ShapeArc);
	ADD_SHAPE(VideoWidget::ShapeText);

#define ADD_UNIT(unit) \
	units_supported_.push_back((VideoWidget::ListItem) { \
		unit, VideoWidget::getFriendlyName(unit), VideoWidget::unit2string(unit) \
	})

	ADD_UNIT(TelemetryData::UnitMilesPerHour);
	ADD_UNIT(TelemetryData::UnitKmPerHour);
	ADD_UNIT(TelemetryData::UnitMeterPerHour);
	ADD_UNIT(TelemetryData::UnitMinPerMile);
	ADD_UNIT(TelemetryData::UnitMinPerKm);

	setShape(VideoWidget::ShapeText);
}
