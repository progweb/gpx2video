#include "avgridespeed.h"


/**
 * Text shape
 */

void AvgRideSpeedTextShape::initialize(void) {
	setSize(theme().height());

	setPadding(
		theme().border() + theme().padding(VideoWidget::Theme::PaddingLeft),
		theme().border() + theme().padding(VideoWidget::Theme::PaddingRight),
   		theme().border() + theme().padding(VideoWidget::Theme::PaddingTop),
   		theme().border() + theme().padding(VideoWidget::Theme::PaddingBottom));
}


void AvgRideSpeedTextShape::draw(cairo_t *cr, const TelemetryData &data) {
	char s[128];

	TextShape::Font font;

	bool pace_unit = false;
	double speed = data.avgridespeed();

	// Format data
	no_value_ = !data.hasValue(TelemetryData::DataAverageRideSpeed);

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
		icon(cr, "./assets/picto/DataOverlay_icn_avgspeed.svg");
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

