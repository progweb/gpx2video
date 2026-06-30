#include "heartrate.h"


/**
 * Text shape
 */

bool HeartRateTextShape::updated(const TelemetryData &data) const {
	if (is_initialized_) {
		if ((data.type() == TelemetryData::TypeUnchanged))
			return false;

		if (no_value_ && !data.hasValue(TelemetryData::DataHeartrate))
			return false;
	}

	return true;
}


void HeartRateTextShape::initialize(cairo_t *cr) {
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

		extents(cr, font, TextShape::TextAlpha, widget_->label().c_str(), x, y, width, height);

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
		
		extents(cr, font, TextShape::TextNumeric, txt.c_str(), x, y, width, height);
		
		setValueExtents(x, y, width, height);
	}

	// Unit height
	if (theme().hasFlag(VideoWidget::Theme::FlagUnit)) {
		std::string txt = widget_->getFriendlyName(widget_->valueUnit());

		TextShape::TextType alphanum = (theme().textOrientation() == VideoWidget::OrientationHorizontal) ? TextShape::TextNumeric : TextShape::TextAlpha;

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

		extents(cr, font, alphanum, txt.c_str(), x, y, width, height);
		
		setUnitExtents(x, y, width, height);
	}

	is_initialized_ = true;
}


void HeartRateTextShape::draw(cairo_t *cr, const TelemetryData &data) {
	char s[128];

	TextShape::Font font;

	int heartrate = data.heartrate(widget_->valueUnit());

	// Initialize
	initialize(cr);

	// Format data
	no_value_ = !data.hasValue(TelemetryData::DataHeartrate);

	if (!no_value_)
		sprintf(s, "%d", heartrate);
	else
		sprintf(s, "--");

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


void HeartRateTextShape::clear(void) {
	TextShape::clear();

	no_value_ = false;

	if (bg_buf_)
		delete bg_buf_;

	if (fg_buf_)
		delete fg_buf_;

	bg_buf_ = NULL;
	fg_buf_ = NULL;
}


HeartRateWidget::HeartRateWidget(GPXApplication &app, TelemetrySource *source)
	: VideoWidget(app, VideoWidget::WidgetHeartRate, source) 
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

	ADD_UNIT(TelemetryData::UnitBPM);

	setShape(VideoWidget::ShapeText);
}

