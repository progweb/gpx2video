#include "gforce.h"


/**
 * Text shape
 */

void GForceTextShape::initialize(void) {
	setSize(theme().height());

	setPadding(
		theme().border() + theme().padding(VideoWidget::Theme::PaddingLeft),
		theme().border() + theme().padding(VideoWidget::Theme::PaddingRight),
   		theme().border() + theme().padding(VideoWidget::Theme::PaddingTop),
   		theme().border() + theme().padding(VideoWidget::Theme::PaddingBottom));
}


void GForceTextShape::draw(cairo_t *cr, const TelemetryData &data) {
	char s[128];

	TextShape::Font font;

	double gforce;

	// Compute data
	// acceleration m/s²
	// gforce g (= 9,80665 m/s²)
	gforce = data.acceleration();
   
	// Format data
	no_value_ = !data.hasValue(TelemetryData::DataAcceleration);

	if (widget_->valueUnit() == VideoWidget::UnitG)
		gforce /= 9.81;

	if (!no_value_)
		sprintf(s, "%.2f %s", gforce, widget_->getFriendlyName(widget_->valueUnit()).c_str());
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


GForceWidget::GForceWidget(GPXApplication &app)
	: VideoWidget(app, VideoWidget::WidgetGForce) 
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

	ADD_UNIT(VideoWidget::UnitMeterPerSec2);
	ADD_UNIT(VideoWidget::UnitG);
	
	setShape(VideoWidget::ShapeText);
}

