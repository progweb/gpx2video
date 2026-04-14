#include "speed.h"


/**
 * Text shape
 */

void SpeedTextShape::initialize(void) {
	setSize(theme().height());

	setPadding(
		theme().border() + theme().padding(VideoWidget::Theme::PaddingLeft),
		theme().border() + theme().padding(VideoWidget::Theme::PaddingRight),
   		theme().border() + theme().padding(VideoWidget::Theme::PaddingTop),
   		theme().border() + theme().padding(VideoWidget::Theme::PaddingBottom));
}


void SpeedTextShape::draw(cairo_t *cr, const TelemetryData &data) {
	char s[128];

	TextShape::Font font;

	bool pace_unit = false;
	double speed = data.speed();

	// Format data
	no_value_ = !data.hasValue(TelemetryData::DataSpeed);

	switch (widget_->unit()) {
	case VideoWidget::UnitMPH:
		speed *= 0.6213711922;
		break;
	case VideoWidget::UnitKPH:
		break;
	case VideoWidget::UnitMPM:
		speed *= 0.6213711922;
		pace_unit = true;
		break;
	case VideoWidget::UnitMPK:
		pace_unit = true;
		break;
	default:
		break;
	}

	if (pace_unit) {
		if (speed <= 0.0)
			no_value_ = true;
	}

	if (no_value_)
		sprintf(s, "-- %s", widget_->unit2string(widget_->unit()).c_str());
	else if (pace_unit) {
		double pace = 60.0 / speed;
		int min = (int) pace;
		int sec = (int) round((pace - min) * 60) % 60;

		sprintf(s, "%d:%02d %s", min, sec, widget_->unit2string(widget_->unit()).c_str());
	} 
	else
		sprintf(s, "%.1f %s", speed, widget_->unit2string(widget_->unit()).c_str());

	// Draw icon
	if (theme().hasFlag(VideoWidget::Theme::FlagIcon)) {
		icon(cr, "./assets/picto/DataOverlay_icn_speed.svg");
	}

	// Draw label
	if (theme().hasFlag(VideoWidget::Theme::FlagLabel)) {
		font = (TextShape::Font) {
			.size = theme().labelFontSize(),
			.border = theme().labelBorderWidth(),
			.shadow_opacity = theme().labelShadowOpacity(),
			.shadow_distance = theme().labelShadowDistance(),
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
			.style = theme().valueFontStyle(),
			.weight = theme().valueFontWeight(),
		};

		value(cr, font, theme().valueColor(), theme().valueBorderColor(), s);
	}
}


/**
 * Arc shape
 */

void SpeedArcShape::initialize(void) {
//	double start = 105; //30 - 330
//	double end = 255;

	width_ = theme().width();
	height_ = theme().height();

	size_ = std::min(width_, height_);

//	arc_start_ = 150;
//	arc_end_ = 290;

	init(fontface_, width_, height_, size_);
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


void SpeedArcShape::ticklenwidth(int value, int *len, int *width) {
	if (value % 10 == 0) {
		*len = size_ / 15 + size_ / 51;
		*width = size_ / 128;
	}
	else {
		*len = size_ / 15 - size_ / 51;
		*width = size_ / 256;
	}
}


void SpeedArcShape::draw(cairo_t *cr, const TelemetryData &data) {
	int rotate = 180;

	double xa1, xa2;

	bool pace_unit = false;

	int vmin, vmax;

	double speed = data.speed();
	double avgspeed = data.avgridespeed();
	double maxspeed = data.maxspeed();

	// Limit
	vmin = theme().valueMin();
	vmax = theme().valueMax();

	// Arc size
	// 30:330 = 300
	int arc_size = 300; //180; //300; //60;
	int arc_rotate = 0;

	rotate += arc_rotate;

	setArcRange(180 - (arc_size / 2), 180 + (arc_size / 2));

//50 : 310 = 260	

	// Tick init
	tickinit(vmin, vmax);

	// Format data
	no_value_ = !data.hasValue(TelemetryData::DataSpeed);

	switch (widget_->unit()) {
	case VideoWidget::UnitMPH:
		speed *= 0.6213711922;
		avgspeed *= 0.6213711922;
		maxspeed *= 0.6213711922;
		break;
	case VideoWidget::UnitKPH:
		break;
	case VideoWidget::UnitMPM:
		speed *= 0.6213711922;
		avgspeed *= 0.6213711922;
		maxspeed *= 0.6213711922;
		pace_unit = true;
		break;
	case VideoWidget::UnitMPK:
		pace_unit = true;
		break;
	default:
		break;
	}

	if (pace_unit) {
		if (speed <= 0.0)
			no_value_ = true;
	}

	// Draw arc background
	pieslice(cr, 0, 360, 0);

	// Draw gauge background
	if (theme().hasFlag(VideoWidget::Theme::FlagGauge)) {
		xa1 = scale(vmin, vmax, 0, rotate);
		xa2 = scale(vmin, vmax, vmax, rotate);

		arc(cr, xa1, xa2, size_ / 150.0, size_ / 21.0 + size_ / 33.0, theme().gaugeBorder(),
				theme().gaugeBackgroundColor(), theme().gaugeBorderColor());

		// Draw color gauge (avg speed)
		if (data.hasValue(TelemetryData::DataAverageRideSpeed)) {
			xa1 = scale(vmin, vmax, 0, rotate);
			xa2 = scale(vmin, vmax, avgspeed, rotate);

			arc(cr, xa1, xa2, size_ / 150.0 + size_ / 33.0, size_ / 21.0, 0.0,
					theme().gaugeColor(0));
		}

		// Draw color gauge (max speed)
		if (data.hasValue(TelemetryData::DataMaxSpeed)) {
			xa1 = scale(vmin, vmax, 0, rotate);
			xa2 = scale(vmin, vmax, maxspeed, rotate);

			arc(cr, xa1, xa2, size_ / 150.0, size_ / 33.0, 0.0,
					theme().gaugeColor(1));
		}
	}

	// Draw tick lines around arc line
	if (theme().hasFlag(VideoWidget::Theme::FlagTick)) {
		for (int value = vmin; value < vmax + tick_step_; value = value + tick_step_) {
			int ticklen;
			int tickwidth;

			double xa = scale(vmin, vmax, value, rotate);

			if (xa > (end() + rotate))
				break;

			ticklenwidth(value, &ticklen, &tickwidth);

			line(cr, xa, 0, ticklen, tickwidth, theme().tickColor());
		}
	}

	// Draw arc label
	if (theme().hasFlag(VideoWidget::Theme::FlagTickLabel)) {
		for (int value = vmin; value < vmax + (tick_mstep_ * tick_step_); value = value + (tick_mstep_ * tick_step_)) {
			double xa = scale(vmin, vmax, value, rotate);

			if (xa > (end() + rotate))
				break;

			std::string str = std::to_string(value);

			text(cr, xa, size_ / 6.5, theme().tickLabelColor(), str);
		}
	}

	// Draw needle
	if (theme().hasFlag(VideoWidget::Theme::FlagNeedle)) {
		double xa = scale(vmin, vmax, !no_value_ ? speed : 0, rotate);

		struct ArcShape::point p;
	   
		const float *fill = theme().needlePrimaryColor();

		cairo_save(cr);
		cairo_set_source_rgba(cr, fill[0], fill[1], fill[2], fill[3]);

		p = locate(xa - 0, 0);
		cairo_move_to(cr, p.x, p.y);

		p = locate(xa - 90, (size_ / 2) - 8);
		cairo_line_to(cr, p.x, p.y);

		p = locate(xa - 180, (size_ / 2) - 8);
		cairo_line_to(cr, p.x, p.y);

		p = locate(xa + 90, (size_ / 2) - 8);
		cairo_line_to(cr, p.x, p.y);

		cairo_close_path(cr);
		cairo_fill_preserve(cr);
		cairo_stroke(cr);

		cairo_restore(cr);
	}

	// Write speed
	if (theme().hasFlag(VideoWidget::Theme::FlagValue)) {
		char value[128];

		cairo_text_extents_t extents;

		const float *fill = theme().valueColor();

		std::string unit = widget_->unit2string(widget_->unit());

		if (no_value_)
			sprintf(value, "--");
		else if (pace_unit) {
			double pace = 60.0 / speed;
			int min = (int) pace;
			int sec = (int) round((pace - min) * 60) % 60;

			sprintf(value, "%d:%02d", min, sec);
		} 
		else
			sprintf(value, "%d", int(std::round(speed)));

		cairo_save(cr);
		cairo_set_font_face(cr, fontface_);
		cairo_set_font_size(cr, size_ / 10);
		cairo_text_extents(cr, value, &extents);
		cairo_set_source_rgba(cr, fill[0], fill[1], fill[2], fill[3]);
		cairo_move_to(cr, size_ / 2.0 - (extents.width / 2), size_ - (extents.height / 2) - (size_ / 20.0));
		cairo_show_text(cr, value);
		cairo_stroke(cr);
		cairo_set_font_size(cr, size_ / 20);
		cairo_text_extents(cr, unit.c_str(), &extents);
		cairo_set_source_rgba(cr, fill[0], fill[1], fill[2], fill[3]);
		cairo_move_to(cr, size_ / 2.0 - (extents.width / 2), size_ - (extents.height / 2));
		cairo_show_text(cr, unit.c_str());
		cairo_stroke(cr);
		cairo_restore(cr);
	}
}
