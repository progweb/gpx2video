#include "duration.h"


/**
 * Text shape
 */

void DurationTextShape::initialize(void) {
	setSize(theme().height());

	setPadding(
		theme().border() + theme().padding(VideoWidget::Theme::PaddingLeft),
		theme().border() + theme().padding(VideoWidget::Theme::PaddingRight),
   		theme().border() + theme().padding(VideoWidget::Theme::PaddingTop),
   		theme().border() + theme().padding(VideoWidget::Theme::PaddingBottom));
}


void DurationTextShape::draw(cairo_t *cr, const TelemetryData &data) {
	char s[128];

	TextShape::Font font;

	int hours = 0;
	int minutes = 0;
	int seconds = 0;

	int duration;

	// Compute duration
	duration = data.duration();

	// Format data
	no_value_ = !data.hasValue(TelemetryData::DataDuration);

	if (duration > 0) {
		seconds = duration % 60;
		duration = duration / 60;
		minutes = duration % 60;
		hours = duration / 60;
	}

	if (!no_value_)
		sprintf(s, "%d:%02d:%02d", hours, minutes, seconds);
	else
		sprintf(s, "--:--:--");

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


DurationWidget::DurationWidget(GPXApplication &app)
	: VideoWidget(app, VideoWidget::WidgetDuration) 
	, ShapeBase(VideoWidget::theme())
	, shape_(NULL) {

#define ADD_SHAPE(shape) \
	shapes_supported_.push_back((VideoWidget::ListItem) { \
		shape, VideoWidget::getFriendlyName(shape), VideoWidget::shape2string(shape) \
	})

	ADD_SHAPE(VideoWidget::ShapeText);

	setShape(VideoWidget::ShapeText);
}

