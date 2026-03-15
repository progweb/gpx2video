#include "verticalspeed.h"


/**
 * Text shape
 */

void VerticalSpeedTextShape::initialize(void) {
	setSize(theme().height());

	setPadding(
		theme().border() + theme().padding(VideoWidget::Theme::PaddingLeft),
		theme().border() + theme().padding(VideoWidget::Theme::PaddingRight),
   		theme().border() + theme().padding(VideoWidget::Theme::PaddingTop),
   		theme().border() + theme().padding(VideoWidget::Theme::PaddingBottom));
}


void VerticalSpeedTextShape::draw(cairo_t *cr, const TelemetryData &data) {
	char s[128];

	TextShape::Font font;

	double verticalspeed = data.verticalspeed();

	// Format data
	no_value_ = !data.hasValue(TelemetryData::DataVerticalSpeed);

	if (widget_->unit() == VideoWidget::UnitMPS) {
	}
	else {
	}

	if (!no_value_)
		sprintf(s, "%.1f %s", verticalspeed, widget_->unit2string(widget_->unit()).c_str());
	else
		sprintf(s, "-- %s", widget_->unit2string(widget_->unit()).c_str());

	// Draw icon
	if (theme().hasFlag(VideoWidget::Theme::FlagIcon)) {
		icon(cr, "./assets/picto/DataOverlay_icn_aviationAltitude.svg");
	}

	// Draw label
	if (theme().hasFlag(VideoWidget::Theme::FlagLabel)) {
		font = (TextShape::Font) {
			.size = theme().labelFontSize(),
			.border = theme().labelBorder(),
			.shadow_opacity = theme().labelShadowOpacity(),
			.shadow_distance = theme().labelShadowDistance(),
			.style = theme().labelFontStyle(),
			.weight = theme().labelFontWeight(),
		};

		label(cr, font, theme().labelColor(), theme().labelBorderColor(), widget_->label().c_str());
	}

	// Draw value
	if (theme().hasFlag(VideoWidget::Theme::FlagValue)) {
		font = (TextShape::Font) {
			.size = theme().valueFontSize(),
			.border = theme().valueBorder(),
			.shadow_opacity = theme().valueShadowOpacity(),
			.shadow_distance = theme().valueShadowDistance(),
			.style = theme().valueFontStyle(),
			.weight = theme().valueFontWeight(),
		};

		value(cr, font, theme().valueColor(), theme().valueBorderColor(), s);
	}
}


/**
 * Bar shape
 */

void VerticalSpeedBarShape::initialize(void) {
//	int offset;
   
	width_ = theme().width();
	height_ = theme().height();

	size_ = std::min(width_, height_);

//	offset = ((size_ / 8) + (size_ / 25)) / 2;

//	init(fontface_, width_, height_, size_, -offset); //, size_);
	init(fontface_, width_, height_, size_); //, size_);

	tick_step_ = 10;
	tick_mstep_ = 10;
}


void VerticalSpeedBarShape::ticklenwidth(int value, int *offset, int *len, int *width) {
	VideoWidget::Theme::Align align = theme().tickAlign();

	if (value % 10 == 0) {
		*len = (size_ / 8) + (size_ / 25);
		*width = size_ / 64;
		*offset = 0;
	}
	else if (value % 5 == 0) {
		*len = size_ / 8;
		*width = size_ / 64;
		if (align == VideoWidget::Theme::AlignCenter)
			*offset = (size_ / 25) / 2;
		else if (align == VideoWidget::Theme::AlignRight)
			*offset = (size_ / 25);
		else
			*offset = 0;
	}
	else {
		*len = (size_ / 8) - (size_ / 25);
		*width = size_ / 128;
		if (align == VideoWidget::Theme::AlignCenter)
			*offset = (size_ / 25);
		else if (align == VideoWidget::Theme::AlignRight)
			*offset = 2 * (size_ / 25);
		else
			*offset = 0;
	}
}


void VerticalSpeedBarShape::draw(cairo_t *cr, const TelemetryData &data) {
	char s[128];

	int offset = 0;
	int rotate = 0;

	int top = 0, bottom = 0;

	int amin, amax;

	double xb1, xb2;

	double verticalspeed = data.verticalspeed();

	// Limit
	amin = theme().valueMin();
	amax = theme().valueMax();

	// Format data
	no_value_ = !data.hasValue(TelemetryData::DataVerticalSpeed);

	if (widget_->unit() == VideoWidget::UnitMPS) {
	}
	else {
	}

	// Apply padding
	if (theme().hasFlag(VideoWidget::Theme::FlagValue))
		offset = (size_ / 14);
	else if (theme().hasFlag(VideoWidget::Theme::FlagTickLabel))
		offset = (size_ / 14) / 2;
	else
		offset = 0;

	if (theme().hasFlag(VideoWidget::Theme::FlagLabel))
		bottom = size_ / 10;

	setPadding(
		theme().padding(VideoWidget::Theme::PaddingLeft),
		theme().padding(VideoWidget::Theme::PaddingRight),
		theme().padding(VideoWidget::Theme::PaddingTop) + offset + top,
		theme().padding(VideoWidget::Theme::PaddingBottom) + offset + bottom
	);

	// Compute gauge position
	offset = ((size_ / 8) + (size_ / 25)); // gauge width
	offset /= 2;
	setOffset(-offset);

	// Draw gauge background
	bar(cr, 0, 1, (size_ / 8) + (size_ / 25), theme().gaugeBorder(),
			theme().gaugeBackgroundColor(), theme().gaugeBorderColor());

	// Draw gauge
	if (theme().hasFlag(VideoWidget::Theme::FlagGauge) && (verticalspeed >= amin)) {
		double to = std::min(verticalspeed, (double) amax);
		double from = (to > 0) ? std::max(0, amin) : std::min(0, amax);

		xb1 = scale(amin, amax, from, rotate);
		xb2 = scale(amin, amax, to, rotate);

		bar(cr, xb1, xb2, size_ / 8 + size_ / 25, 0,
				theme().gaugeColor(0));
	}

	// Draw cursor
	if (theme().hasFlag(VideoWidget::Theme::FlagCursor) && ((verticalspeed >= amin) && (verticalspeed <= amax))) {
		double xb = scale(amin, amax, verticalspeed, rotate);

		cursor(cr, xb, size_ / 8 + size_ / 25);
	}

	// Draw tick lines on bar
	if (theme().hasFlag(VideoWidget::Theme::FlagTick)) {
		for (int value = amin; value < amax + tick_step_; value = value + tick_step_) {
			int ticklen;
			int tickwidth;
			int tickoffset;

			double xb = scale(amin, amax, value, rotate);

			ticklenwidth(value / tick_mstep_, &tickoffset, &ticklen, &tickwidth);

			line(cr, xb, tickoffset, tickoffset + ticklen, theme().tickColor());
		}
	}

	// Draw bar label
	if (theme().hasFlag(VideoWidget::Theme::FlagTickLabel)) {
		int min = amin; // + (mstep_ * step_);
		int max = amax;

		bool first = true;

		int step = tick_mstep_ * tick_step_;

		min /= step;
		min *= step;

		max /= step;
		max *= step;

		for (int value = min; value < max + step; value = value + step) {
			double xb = scale(amin, amax, value, rotate);

			if (first || (value >= max))
				sprintf(s, "%d %s", value, widget_->unit2string(widget_->unit()).c_str());
			else
				sprintf(s, "%d", value);

//				std::string str = std::to_string(value);

			if (value < amin)
				continue;

			text(cr, xb, size_ / 2.75, size_ / 14, theme().tickLabelColor(), std::string(s)); //str);

			first = false;
		}
	}

	// Draw marker value
	if (theme().hasFlag(VideoWidget::Theme::FlagValue) && ((verticalspeed >= amin) && (verticalspeed <= amax))) {
		double xb = scale(amin, amax, verticalspeed, rotate);

		std::string value = std::to_string((int) std::round(verticalspeed));

		marker(cr, xb, theme().valueBorder(), theme().valueColor(), theme().valueBackgroundColor(), theme().valueBorderColor(), value);
	}

	// Draw label
	if (theme().hasFlag(VideoWidget::Theme::FlagLabel)) {
		int x, y;

		x = width_ / 2;
		y = height_ - theme().padding(VideoWidget::Theme::PaddingBottom);

		label(cr, x, y, theme().labelColor(), widget_->label());
	}
}
