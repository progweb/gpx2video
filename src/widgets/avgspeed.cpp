#include "avgspeed.h"


/**
 * Text shape
 */

void AvgSpeedTextShape::initialize(void) {
	setSize(theme().height());

	setPadding(
		theme().border() + theme().padding(VideoWidget::Theme::PaddingLeft),
		theme().border() + theme().padding(VideoWidget::Theme::PaddingRight),
   		theme().border() + theme().padding(VideoWidget::Theme::PaddingTop),
   		theme().border() + theme().padding(VideoWidget::Theme::PaddingBottom));
}


void AvgSpeedTextShape::draw(cairo_t *cr, const TelemetryData &data) {
	char s[128];

	TextShape::Font font;

	bool pace_unit = false;
	double speed = data.avgspeed();

	// Format data
	no_value_ = !data.hasValue(TelemetryData::DataAverageSpeed);

	switch (widget_->valueUnit()) {
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
		sprintf(s, "-- %s", widget_->getFriendlyName(widget_->valueUnit()).c_str());
	else if (pace_unit) {
		double pace = 60.0 / speed;
		int min = (int) pace;
		int sec = (int) round((pace - min) * 60) % 60;

		sprintf(s, "%d:%02d %s", min, sec, widget_->getFriendlyName(widget_->valueUnit()).c_str());
	} 
	else
		sprintf(s, "%.1f %s", speed, widget_->getFriendlyName(widget_->valueUnit()).c_str());


	// Draw icon
	if (theme().hasFlag(VideoWidget::Theme::FlagIcon)) {
		icon(cr, icon_filename_);
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


AvgSpeedWidget::AvgSpeedWidget(GPXApplication &app)
	: VideoWidget(app, VideoWidget::WidgetAverageSpeed) 
	, ShapeBase(VideoWidget::theme())
	, shape_(NULL) {

#define ADD_SHAPE(shape) \
	shapes_supported_.push_back((VideoWidget::ListItem) { \
		shape, VideoWidget::getFriendlyName(shape), VideoWidget::shape2string(shape) \
	})

	ADD_SHAPE(VideoWidget::ShapeText);

#define ADD_UNIT(unit) \
	units_supported_.push_back((VideoWidget::ListItem) { \
		unit, VideoWidget::getFriendlyName(unit), VideoWidget::unit2string(unit) \
	})

	ADD_UNIT(VideoWidget::UnitMPH);
	ADD_UNIT(VideoWidget::UnitKPH);
	ADD_UNIT(VideoWidget::UnitMPM);
	ADD_UNIT(VideoWidget::UnitMPK);

	setShape(VideoWidget::ShapeText);
}

