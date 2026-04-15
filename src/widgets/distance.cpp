#include "distance.h"


/**
 * Text shape
 */

void DistanceTextShape::initialize(void) {
	setSize(theme().height());

	setPadding(
		theme().border() + theme().padding(VideoWidget::Theme::PaddingLeft),
		theme().border() + theme().padding(VideoWidget::Theme::PaddingRight),
   		theme().border() + theme().padding(VideoWidget::Theme::PaddingTop),
   		theme().border() + theme().padding(VideoWidget::Theme::PaddingBottom));
}


void DistanceTextShape::draw(cairo_t *cr, const TelemetryData &data) {
	char s[128];

	TextShape::Font font;

	double distance = data.distance();

	// Format data
	no_value_ = !data.hasValue(TelemetryData::DataDistance);

	if (widget_->unit() == VideoWidget::UnitKm) {
		distance /= 1000.0;
	}
	else if (widget_->unit() == VideoWidget::UnitMeter) {
	}
	else if (widget_->unit() == VideoWidget::UnitFoot) {
		distance *= 3.28084;
	}
	else {
		distance /= 1000.0;
		distance *= 0.6213711922;
	}

	if (!no_value_) {
		const char *format;

		if (distance < 10)
			format = "%.2f %s";
		else if (distance < 100)
			format = "%.1f %s";
		else
			format = "%.0f %s";

		sprintf(s, format, distance, widget_->unit2string(widget_->unit()).c_str());
	}
	else
		sprintf(s, "-- %s", widget_->unit2string(widget_->unit()).c_str());

	// Draw icon
	if (theme().hasFlag(VideoWidget::Theme::FlagIcon)) {
		icon(cr, "./assets/picto/DataOverlay_icn_distance.svg");
	}

	// Draw label
	if (theme().hasFlag(VideoWidget::Theme::FlagLabel)) {
		font = (TextShape::Font) {
			.size = theme().labelFontSize(),
			.border = theme().labelBorderWidth(),
			.shadow_opacity = theme().labelShadowOpacity(),
			.shadow_distance = theme().labelShadowDistance(),
			.family = theme().labelFontFamily(),
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
			.style = theme().valueFontStyle(),
			.weight = theme().valueFontWeight(),
		};

		value(cr, font, theme().valueColor(), theme().valueBorderColor(), s);
	}
}

