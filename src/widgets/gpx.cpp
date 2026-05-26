#include "gpx.h"


GPXWidget::GPXWidget(GPXApplication &app)
	: VideoWidget(app, VideoWidget::WidgetGPX)
	, ShapeBase(theme(), VideoWidget::ShapeText)
	, bg_buf_(NULL)
	, fg_buf_(NULL) {

#define ADD_SHAPE(shape) \
	shapes_supported_.push_back((VideoWidget::ListItem) { \
		shape, VideoWidget::getFriendlyName(shape), VideoWidget::shape2string(shape) \
	})

	ADD_SHAPE(VideoWidget::ShapeText);
}


void GPXWidget::initialize(void) {
	setPadding(
		theme().border() + theme().padding(VideoWidget::Theme::PaddingLeft),
		theme().border() + theme().padding(VideoWidget::Theme::PaddingRight),
   		theme().border() + theme().padding(VideoWidget::Theme::PaddingTop),
   		theme().border() + theme().padding(VideoWidget::Theme::PaddingBottom));
}


void GPXWidget::draw(cairo_t *cr, const TelemetryData &data) {
	int y = 0;

	ShapeBase::Font font;

	// Add padding
	y += padding_top_;

	// Draw label
	if (theme().hasFlag(VideoWidget::Theme::FlagLabel)) {
		int linewidth = 20 * theme().labelFontSize() / 100;

		std::string s = ((VideoWidget *) this)->label();

		font = (ShapeBase::Font) {
			.size = theme().labelFontSize(),
			.border = theme().labelBorderWidth(),
			.shadow_opacity = theme().labelShadowOpacity(),
			.shadow_distance = theme().labelShadowDistance(),
			.family = theme().labelFontFamily(),
			.style = theme().labelFontStyle(),
			.weight = theme().labelFontWeight(),
		};

		y += label(cr, font, theme().labelColor(), theme().labelBorderColor(), s.c_str());
		y += linewidth;
	}

	// Draw value
	if (theme().hasFlag(VideoWidget::Theme::FlagValue)) {
		int diff;

		time_t t;
		struct tm time;

		char s[128];
		char str[32];

		int linewidth = 20 * theme().valueFontSize() / 100;

		font = (ShapeBase::Font) {
			.size = theme().valueFontSize(),
			.border = theme().valueBorderWidth(),
			.shadow_opacity = theme().valueShadowOpacity(),
			.shadow_distance = theme().valueShadowDistance(),
			.family = theme().valueFontFamily(),
			.style = theme().valueFontStyle(),
			.weight = theme().valueFontWeight(),
		};

		// Line
		sprintf(s, "Line: %d", data.line());

		y += value(cr, y, font, theme().valueColor(), theme().valueBorderColor(), s);
		y += linewidth;

		// Type 
		sprintf(s, "Type: %s", data.type2string());

		y += value(cr, y, font, theme().valueColor(), theme().valueBorderColor(), s);
		y += linewidth;

		// Timestamp
		t = data.timestamp() / 1000;
		gmtime_r(&t, &time);
		strftime(str, sizeof(str), "Timestamp: %H:%M:%S", &time);

		snprintf(s, sizeof(s), "%s.%03d", str, (int) (data.timestamp() % 1000));

		y += value(cr, y, font, theme().valueColor(), theme().valueBorderColor(), s);
		y += linewidth;

		// Video timestamp
		t = timestamp() / 1000;
		gmtime_r(&t, &time);
		strftime(str, sizeof(str), "Video timestamp: %H:%M:%S", &time);

		snprintf(s, sizeof(s), "%s.%03d", str, (int) (timestamp() % 1000));

		y += value(cr, y, font, theme().valueColor(), theme().valueBorderColor(), s);
		y += linewidth;

		// Video timestamp delay
		diff = timestamp() - data.timestamp();

		snprintf(s, sizeof(s), "Delay: %s%d ms", (diff > 0) ? "+" : "", diff);

		y += value(cr, y, font, theme().valueColor(), theme().valueBorderColor(), s);
		y += linewidth;

		// Latitude
		sprintf(s, "Lat: %.4f", data.latitude());

		y += value(cr, y, font, theme().valueColor(), theme().valueBorderColor(), s);
		y += linewidth;

		// Longitude
		sprintf(s, "Lon: %.4f", data.longitude());

		y += value(cr, y, font, theme().valueColor(), theme().valueBorderColor(), s);
		y += linewidth;

		// Elevation
		sprintf(s, "Ele: %.4f", data.elevation());

		y += value(cr, y, font, theme().valueColor(), theme().valueBorderColor(), s);
		y += linewidth;
	}
}


int GPXWidget::label(cairo_t *cr, ShapeBase::Font &font, 
		const float *fill, const float *outline, const char *text) {
	int x, y;

	int tx, ty;
	int width, height;

	enum VideoWidget::Theme::Align textAlign = theme().labelAlign();

	this->textSize(cr, font, text, tx, ty, width, height);

	// Text offset
	x = -tx;
	y = -ty + font.shadow_distance;

	// Text position
	if (textAlign == VideoWidget::Theme::AlignLeft) {
		x += padding_left_;
	}
	else if (textAlign == VideoWidget::Theme::AlignCenter) {
		x += theme().width() / 2;
		x -= width / 2;
	}
	else if (textAlign == VideoWidget::Theme::AlignRight) {
		x += theme().width() - padding_right_;
		x -= width + font.shadow_distance;
	}

	y += padding_top_;

	this->drawText(cr, x, y, font, fill, outline, text);

	return height;
}


int GPXWidget::value(cairo_t *cr, int y, ShapeBase::Font &font, 
		const float *fill, const float *outline, const char *text) {
	int x;

	int tx, ty;
	int width, height;

	enum VideoWidget::Theme::Align textAlign = theme().valueAlign();

	this->textSize(cr, font, text, tx, ty, width, height);

	// Text offset
	x = -tx;
	y += -ty + font.shadow_distance;

	// Text position
	if (textAlign == VideoWidget::Theme::AlignLeft) {
		x += padding_left_;
	}
	else if (textAlign == VideoWidget::Theme::AlignCenter) {
		x += theme().width() / 2;
		x -= width / 2;
	}
	else if (textAlign == VideoWidget::Theme::AlignRight) {
		x += theme().width() - padding_right_;
		x -= width + font.shadow_distance;
	}

	y += height;

//	y += theme().height() - font.shadow_distance - padding_bottom_ - height;

	this->drawText(cr, x, y, font, fill, outline, text);

	return height;
}


