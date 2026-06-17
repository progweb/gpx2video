#include "duration.h"


/**
 * Text shape
 */

bool DurationTextShape::updated(const TelemetryData &data) const {
	int duration;

	// Compute duration
	duration = data.duration();

	if (is_initialized_) {
		if (duration == last_duration_)
			return false;

		if (no_value_ && !data.hasValue(TelemetryData::DataDuration))
			return false;
	}

	return true;
}


void DurationTextShape::initialize(cairo_t *cr) {
	if (is_initialized_)
		return;

	int x, y;
	int width, height;

	TextShape::Font font;

	setSize(theme().height());

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
		std::string txt = "88:88:88";

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

	is_initialized_ = true;
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

	// Initialize
	initialize(cr);

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

	// Save last value
	last_duration_ = duration;
}


void DurationTextShape::clear(void) {
	TextShape::clear();

	no_value_ = false;

	last_duration_ = 0;

	if (bg_buf_)
		delete bg_buf_;

	if (fg_buf_)
		delete fg_buf_;

	bg_buf_ = NULL;
	fg_buf_ = NULL;
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

