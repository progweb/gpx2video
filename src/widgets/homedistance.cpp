#include "homedistance.h"


/**
 * Text shape
 */

void HomeDistanceTextShape::initialize(void) {
	setSize(theme().height());

	setPadding(
		theme().border() + theme().padding(VideoWidget::Theme::PaddingLeft),
		theme().border() + theme().padding(VideoWidget::Theme::PaddingRight),
   		theme().border() + theme().padding(VideoWidget::Theme::PaddingTop),
   		theme().border() + theme().padding(VideoWidget::Theme::PaddingBottom));
}


void HomeDistanceTextShape::draw(cairo_t *cr, const TelemetryData &data) {
	char s[128];

	TextShape::Font font;

	double homedistance = data.homedistance();

	// Format data
	no_value_ = !data.hasValue(TelemetryData::DataHomeDistance);

	if (widget_->valueUnit() == VideoWidget::UnitKm) {
		homedistance /= 1000.0;
	}
	else if (widget_->valueUnit() == VideoWidget::UnitMeter) {
	}
	else if (widget_->valueUnit() == VideoWidget::UnitFeet) {
		homedistance *= 3.28084;
	}
	else {
		homedistance /= 1000.0;
		homedistance *= 0.6213711922;
	}

	if (!no_value_) {
		const char *format;

		if (homedistance < 10)
			format = "%.2f %s";
		else if (homedistance < 100)
			format = "%.1f %s";
		else
			format = "%.0f %s";

		sprintf(s, format, homedistance, widget_->getFriendlyName(widget_->valueUnit()).c_str());
	}
	else
		sprintf(s, "-- %s", widget_->getFriendlyName(widget_->valueUnit()).c_str());

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


HomeDistanceWidget::HomeDistanceWidget(GPXApplication &app)
	: VideoWidget(app, VideoWidget::WidgetHomeDistance)
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

	ADD_UNIT(VideoWidget::UnitMiles);
	ADD_UNIT(VideoWidget::UnitKm);
	ADD_UNIT(VideoWidget::UnitMeter);
	ADD_UNIT(VideoWidget::UnitFeet);

	setShape(VideoWidget::ShapeText);
}

