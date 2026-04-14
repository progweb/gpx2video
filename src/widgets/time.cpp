#include "time.h"


/**
 * Text shape
 */

void TimeTextShape::initialize(void) {
	setSize(theme().height());

	setPadding(
		theme().border() + theme().padding(VideoWidget::Theme::PaddingLeft),
		theme().border() + theme().padding(VideoWidget::Theme::PaddingRight),
   		theme().border() + theme().padding(VideoWidget::Theme::PaddingTop),
   		theme().border() + theme().padding(VideoWidget::Theme::PaddingBottom));
}


void TimeTextShape::draw(cairo_t *cr, const TelemetryData &data) {
	char s[128];

	TextShape::Font font;

	time_t t;
	struct tm time;

	// Format data
	if (data.datetime() != 0) {
		// Compute time
		t = data.datetime() / 1000;

		// Format data
		// Don't use gps time, but camera time!
		// Indeed, with garmin devices, gpx time has an offset.
		localtime_r(&t, &time);

		strftime(s, sizeof(s), "%H:%M:%S", &time);
	}
	else
		strncpy(s, "--:--:--", sizeof(s));

	// Draw icon
	if (theme().hasFlag(VideoWidget::Theme::FlagIcon)) {
		icon(cr, "./assets/picto/DataOverlay_icn_time.svg");
	}

	// Draw label
	if (theme().hasFlag(VideoWidget::Theme::FlagLabel)) {
		font = (TextShape::Font) {
			.size = theme().labelFontSize(),
			.border = theme().labelBorderWidth(),
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
			.border = theme().valueBorderWidth(),
			.shadow_opacity = theme().valueShadowOpacity(),
			.shadow_distance = theme().valueShadowDistance(),
			.style = theme().valueFontStyle(),
			.weight = theme().valueFontWeight(),
		};

		value(cr, font, theme().valueColor(), theme().valueBorderColor(), s);
	}
}


/**
 * Text shape
 */

void TimeArcShape::initialize(void) {
	width_ = theme().width();
	height_ = theme().height();

	size_ = std::min(width_, height_);

	init(fontface_, width_, height_, size_);
}


void TimeArcShape::tickinit(int min, int max) {
	(void) min;
	(void) max;

	tick_step_ = 1;
	tick_mstep_ = 0;
}


void TimeArcShape::ticklenwidth(int value, int *len, int *width) {
	if (value % 5 == 0) {
		*len = size_ / 15 + size_ / 51;
		*width = size_ / 96;
	}
	else {
		*len = size_ / 15 - size_ / 51;
		*width = size_ / 128;
	}
}


void TimeArcShape::draw(cairo_t *cr, const TelemetryData &data) {
	bool no_value = false;

	int rotate = 0;

	VideoWidget::Theme::NeedleType type;

	double xa;

	time_t t;
	struct tm time;

	int tmin, tmax;

	const float *primary_color, *secondary_color;

	// Limit
	tmin = 0;
	tmax = 60;

	// Arc size
	int arc_size = 360;
	int arc_rotate = 0;

	rotate += arc_rotate;

	setArcRange(180 - (arc_size / 2), 180 + (arc_size / 2));

	// Format data
	if (data.datetime() != 0) {
		// Compute time
		t = data.datetime() / 1000;

		// Format data
		// Don't use gps time, but camera time!
		// Indeed, with garmin devices, gpx time has an offset.
		localtime_r(&t, &time);
	}
	else
		no_value = true;

	// Tick init
	tickinit(tmin, tmax);

	// Draw background
	pieslice(cr, 0, 360, 0);

	// Draw tick lines around arc line
	if (theme().hasFlag(VideoWidget::Theme::FlagTick)) {
		for (int value = tmin; value < tmax + tick_step_; value = value + tick_step_) {
			int ticklen;
			int tickwidth;

			xa = scale(tmin, tmax, value, rotate);

			if (xa > (end() + rotate))
				break;

			ticklenwidth(value, &ticklen, &tickwidth);

			line(cr, xa, 0, ticklen, tickwidth, theme().tickColor());
		}
	}

	// Draw arc label
	// 1 2 3 4 5 6...
	// 1 3 6...
	// 1 6...
	if (theme().hasFlag(VideoWidget::Theme::FlagTickLabel)) {
		double distance;
	   
		if (theme().hasFlag(VideoWidget::Theme::FlagTick))
			distance = size_ / 6.5;
		else
			distance = size_ / 16;

		int mstep = (tick_mstep_ > 0) ? 3 * tick_mstep_ : 1;

		for (int value = 0; value < 12; value = value + mstep) {
			xa = scale(tmin / 5, tmax / 5, value + 1, rotate);

			if (xa > (end() + rotate))
				break;

			std::string str = std::to_string(value + 1);

			text(cr, xa, distance, theme().tickLabelColor(), str);
		}
	}

	// Needle type
	if (!no_value && theme().hasFlag(VideoWidget::Theme::FlagNeedle)) {
		// thin, light, basic, design
		type = theme().needleType();

		// Colors
		primary_color = theme().needlePrimaryColor();
		secondary_color = theme().needleSecondaryColor();

		// Draw minute needle
		xa = scale(tmin, tmax, time.tm_min, rotate);
		drawNeedle(cr, type, xa, (size_ / 72), false, primary_color, secondary_color);

		// Draw hour needle
		xa = scale(tmin, tmax, time.tm_hour * 5, rotate);
		drawNeedle(cr, type, xa, (size_ / 4), false, primary_color, secondary_color);

		// Draw second needle
		xa = scale(tmin, tmax, time.tm_sec, rotate);
		drawNeedle(cr, VideoWidget::Theme::NeedleTypeThin, xa, 0.0,
				(type == VideoWidget::Theme::NeedleTypeDesign),
				secondary_color, secondary_color);
	}
}


void TimeArcShape::drawNeedle(cairo_t *cr, VideoWidget::Theme::NeedleType  type, double xa, double len, bool design,
		const float *color1, const float *color2) {
	struct point p;
   
	const float *fill;

	cairo_save(cr);

	switch (type) {
	case VideoWidget::Theme::NeedleTypeDesign:
		//
		fill = color1;
		cairo_set_source_rgba(cr, fill[0], fill[1], fill[2], fill[3]);

		cairo_set_line_width(cr, size_ / 36);
		cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);

		p = locate(xa - 0, len);
		cairo_move_to(cr, p.x, p.y);

		p = locate(xa - 0, (size_ / 2));
		cairo_line_to(cr, p.x, p.y);

		cairo_stroke(cr);

		//
		fill = color2;
		cairo_set_source_rgba(cr, fill[0], fill[1], fill[2], fill[3]);

		cairo_set_line_width(cr, size_ / 72);
		cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);

		p = locate(xa - 0, len);
		cairo_move_to(cr, p.x, p.y);

		p = locate(xa - 0, (size_ / 3));
		cairo_line_to(cr, p.x, p.y);

		cairo_stroke(cr);
		break;

	case VideoWidget::Theme::NeedleTypeBasic:
		fill = color1;
		cairo_set_source_rgba(cr, fill[0], fill[1], fill[2], fill[3]);

		p = locate(xa - 0, len);
		cairo_move_to(cr, p.x, p.y);

		p = locate(xa - 90, (size_ / 2) - 6);
		cairo_line_to(cr, p.x, p.y);

		p = locate(xa - 180, (size_ / 2) - 6);
		cairo_line_to(cr, p.x, p.y);

		p = locate(xa + 90, (size_ / 2) - 6);
		cairo_line_to(cr, p.x, p.y);

		cairo_close_path(cr);
		cairo_fill_preserve(cr);
		cairo_stroke(cr);
		break;

	case VideoWidget::Theme::NeedleTypeLight:
		fill = color1;
		cairo_set_source_rgba(cr, fill[0], fill[1], fill[2], fill[3]);

		//
		cairo_set_line_width(cr, size_ / 64);
		cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);

		p = locate(xa - 0, len);
		cairo_move_to(cr, p.x, p.y);

		p = locate(xa - 0, (size_ / 2));
		cairo_line_to(cr, p.x, p.y);

		cairo_stroke(cr);

		break;

	case VideoWidget::Theme::NeedleTypeThin:
	default:
		fill = color1;
		cairo_set_source_rgba(cr, fill[0], fill[1], fill[2], fill[3]);

		if (design) {
			cairo_arc(cr, size_ / 2, size_ / 2, size_ / 64, 0, 2 * M_PI);
			cairo_fill_preserve(cr);
			cairo_stroke(cr);
		}

		//
		cairo_set_line_width(cr, size_ / 96);
		cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);

		p = locate(xa - 0, len);
		cairo_move_to(cr, p.x, p.y);

		p = locate(xa - 0, (size_ / 2));
		cairo_line_to(cr, p.x, p.y);

		cairo_stroke(cr);

		//
		if (design) {
			cairo_set_line_width(cr, size_ / 64);
			cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);

			p = locate(xa - 0, (size_ / 2.1));
			cairo_move_to(cr, p.x, p.y);

			p = locate(xa - 0, (size_ / 1.8));
			cairo_line_to(cr, p.x, p.y);

			cairo_stroke(cr);
		}

		//
		if (design) {
			cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);

			cairo_arc(cr, size_ / 2, size_ / 2, size_ / 512, 0, 2 * M_PI);
			cairo_fill_preserve(cr);
			cairo_stroke(cr);
		}

		break;
	}

	cairo_restore(cr);
}


//void ClockIndicator::drawSecondNeedle(Cairo::RefPtr<Cairo::Context> const &cr, int type, int rotate) {
//	double value = _second;
//
//	double xa = arc_.scale(min_, max_, value * 5, rotate);
//
//	struct Arc::point p;
//   
//	glm::vec4 fill = theme().needleSecondaryColor();
//
//	cairo_save();
//
//	switch (type) {
//	case 0:
//	case 1:
//	default:
//		//
//		cairo_set_source_rgba(fill[0], fill[1], fill[2], fill[3]);
//
//		if (type == 1) {
//			cairo_arc(_size / 2, _size / 2, _size / 64, 0, 2 * M_PI);
//			cairo_fill_preserve();
//			cairo_stroke();
//		}
//
//		//
//		cairo_set_line_width(_size / 96);
//		cairo_set_line_cap(Cairo::Context::LineCap::ROUND);
//
//		p = arc_.locate(xa - 0, (_size / 64));
//		cairo_move_to(p.x, p.y);
//
//		p = arc_.locate(xa - 0, (_size / 2));
//		cairo_line_to(p.x, p.y);
//
//		cairo_stroke();
//
//		//
//		if (type == 1) {
//			cairo_set_line_width(_size / 64);
//			cairo_set_line_cap(Cairo::Context::LineCap::ROUND);
//
//			p = arc_.locate(xa - 0, (_size / 2.1));
//			cairo_move_to(p.x, p.y);
//
//			p = arc_.locate(xa - 0, (_size / 1.8));
//			cairo_line_to(p.x, p.y);
//
//			cairo_stroke();
//		}
//
//		//
//		if (type == 1) {
//			cairo_set_source_rgba(0.0, 0.0, 0.0, 1.0);
//
//			cairo_arc(_size / 2, _size / 2, _size / 512, 0, 2 * M_PI);
//			cairo_fill_preserve();
//			cairo_stroke();
//		}
//
//		break;
//	}
//
//	cairo_restore();
//}
//
//
