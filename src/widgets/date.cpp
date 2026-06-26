#include <time.h>

#include "date.h"


/**
 * Text shape
 */

bool DateTextShape::updated(const TelemetryData &data) const {
	time_t t;

#define TIMESTAMP2DAY(x)  (x / (24 * 60 * 60))

	// Convert time to sec
	t = data.datetime() / 1000;

	if (is_initialized_) {
		if (TIMESTAMP2DAY(t) == TIMESTAMP2DAY(last_time_))
			return false;
	}

	return true;
}


void DateTextShape::initialize(cairo_t *cr) {
	if (is_initialized_)
		return;

	int x, y;
	int width, height;

	char s[128];

	time_t now = time(NULL);

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

		extents(cr, font, widget_->label().c_str(), x, y, width, height);

		setLabelExtents(x, y, width, height);
	}

	// Value height
	if (theme().hasFlag(VideoWidget::Theme::FlagValue)) {
		struct tm tm;

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

		localtime_r(&now, &tm);
		strftime(s, sizeof(s), widget_->valueFormat().c_str(), &tm);

		extents(cr, font, s, x, y, width, height);
		
		setValueExtents(x, y, width, height);
	}

	// Done
	is_initialized_ = true;
}


void DateTextShape::draw(cairo_t *cr, const TelemetryData &data) {
	char s[128];

	TextShape::Font font;

	time_t t;
	struct tm time;

	// Initialize
	initialize(cr);

	// Compute time
	t = data.datetime() / 1000;

	// Format data
	// Don't use gps time, but camera time!
	// Indeed, with garmin devices, gpx time has an offset.
	localtime_r(&t, &time);

	strftime(s, sizeof(s), widget_->valueFormat().c_str(), &time);

	// Format data
	if (data.datetime() == 0) {
		for (size_t i=0; i<strlen(s); i++) 
			if (std::isdigit(static_cast<unsigned char>(s[i])))
				s[i] = '-';
	}

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
	last_time_ = t;
}


void DateTextShape::clear(void) {
	TextShape::clear();

	if (bg_buf_)
		delete bg_buf_;

	if (fg_buf_)
		delete fg_buf_;

	bg_buf_ = NULL;
	fg_buf_ = NULL;
}


DateWidget::DateWidget(GPXApplication &app, TelemetrySource *source)
	: VideoWidget(app, VideoWidget::WidgetDate, source) 
	, ShapeBase(VideoWidget::theme())
   	, shape_(NULL) {

#define ADD_SHAPE(shape) \
	shapes_supported_.push_back((VideoWidget::ListItem) { \
		shape, VideoWidget::getFriendlyName(shape), VideoWidget::shape2string(shape) \
	})

	ADD_SHAPE(VideoWidget::ShapeText);

	// Append format supported
	formats_supported_.push_back((VideoWidget::ListItem) { 0, "YYYY-MM-DD", "%Y-%m-%d" });
	formats_supported_.push_back((VideoWidget::ListItem) { 1, "YYYY/MM/DD", "%Y/%m/%d" });
	formats_supported_.push_back((VideoWidget::ListItem) { 2, "DD-MM-YYYY", "%d-%m-%Y" });
	formats_supported_.push_back((VideoWidget::ListItem) { 3, "DD/MM/YYYY", "%d/%m/%Y" });

	setShape(VideoWidget::ShapeText);
}

