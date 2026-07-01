#include "maxspeed.h"


/**
 * Text shape
 */

bool MaxSpeedTextShape::updated(const TelemetryData &data) const {
	if (is_initialized_) {
		if ((data.type() == TelemetryData::TypeUnchanged))
			return false;

		if (no_value_ && !data.hasValue(TelemetryData::DataMaxSpeed))
			return false;
	}

	return true;
}


void MaxSpeedTextShape::initialize(cairo_t *cr) {
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
			.linespace = 0.0,
		};

		extents(cr, font, TextShape::TextAlpha, widget_->label().c_str(), x, y, width, height);

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
			.linespace = 0.0,
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
			.border = theme().unitBorderWidth(),
			.shadow_opacity = theme().unitShadowOpacity(),
			.shadow_distance = theme().unitShadowDistance(),
			.family = theme().unitFontFamily(),
			.align = theme().unitHorizontalAlign(),
			.style = theme().unitFontStyle(),
			.weight = theme().unitFontWeight(),
			.linespace = 0.0,
		};

		extents(cr, font, alphanum, txt.c_str(), x, y, width, height);
		
		setUnitExtents(x, y, width, height);
	}

	is_initialized_ = true;
}


void MaxSpeedTextShape::draw(cairo_t *cr, const TelemetryData &data) {
	char s[128];

	TextShape::Font font;

	bool pace_unit = false;
	double speed = data.maxspeed(widget_->valueUnit());

	// Initialize
	initialize(cr);

	// Format data
	no_value_ = !data.hasValue(TelemetryData::DataMaxSpeed);

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

	if (!no_value_)
		sprintf(s, "%.0f", speed);
	else if (pace_unit) {
		int min = (int) speed;
		int sec = (int) round((speed - min) * 60) % 60;

		sprintf(s, "%d:%02d", min, sec);
	} 
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
			.linespace = 0.0,
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
			.linespace = 0.0,
		};

		value(cr, font, theme().valueColor(), theme().valueBorderColor(), s);
	}

	// Draw unit
	if (theme().hasFlag(VideoWidget::Theme::FlagUnit)) {
		std::string u = widget_->getFriendlyName(widget_->valueUnit());

		font = (TextShape::Font) {
			.size = theme().unitFontSize(),
			.border = theme().unitBorderWidth(),
			.shadow_opacity = theme().unitShadowOpacity(),
			.shadow_distance = theme().unitShadowDistance(),
			.family = theme().unitFontFamily(),
			.align = theme().unitHorizontalAlign(),
			.style = theme().unitFontStyle(),
			.weight = theme().unitFontWeight(),
			.linespace = 0.0,
		};

		unit(cr, font, theme().unitColor(), theme().unitBorderColor(), u.c_str());
	}
}


void MaxSpeedTextShape::clear(void) {
	TextShape::clear();

	no_value_ = false;

	if (bg_buf_)
		delete bg_buf_;

	if (fg_buf_)
		delete fg_buf_;

	bg_buf_ = NULL;
	fg_buf_ = NULL;
}


MaxSpeedWidget::MaxSpeedWidget(GPXApplication &app, TelemetrySource *source)
	: VideoWidget(app, VideoWidget::WidgetMaxSpeed, source) 
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

	ADD_UNIT(TelemetryData::UnitMilesPerHour);
	ADD_UNIT(TelemetryData::UnitKmPerHour);
	ADD_UNIT(TelemetryData::UnitMeterPerHour);
	ADD_UNIT(TelemetryData::UnitMinPerMile);
	ADD_UNIT(TelemetryData::UnitMinPerKm);

	setShape(VideoWidget::ShapeText);
}

