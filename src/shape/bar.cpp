#include "../log_i.h"
#include "bar.h"


void BarShape::bar(cairo_t *cr, double v1, double v2, double width, double border,
		const float *fill, const float *outline) {
	struct BarShape::point p1 = locate(v1, -width / 2);
	struct BarShape::point p2 = locate(v2, -width / 2);

	cairo_save(cr);
	cairo_rectangle(cr, p1.x, p1.y, width, p2.y - p1.y);
	cairo_set_source_rgba(cr, fill[0], fill[1], fill[2], fill[3]);
	cairo_fill_preserve(cr);
	if ((border > 0) && (outline != NULL)) {
		cairo_set_line_width(cr, border);
		cairo_set_source_rgba(cr, outline[0], outline[1], outline[2], outline[3]);
	}
	cairo_stroke(cr);
	cairo_restore(cr);
}


void BarShape::cursor(cairo_t *cr, double v, double width, const float *fill) {
	double thick = width / 8.0;

	struct BarShape::point p = locate(v, -width / 2);

//	const float *fill = theme().cursorColor();

	cairo_save(cr);
	cairo_rectangle(cr, p.x, p.y - (thick / 2), width, thick);
	cairo_set_source_rgba(cr, fill[0], fill[1], fill[2], fill[3]);
	cairo_fill_preserve(cr);
	cairo_stroke(cr);
	cairo_restore(cr);
}


void BarShape::line(cairo_t *cr, double v, double d1, double d2, double width, const float *fill) {
	double offset = (d2 - d1) / 2.0;

	struct BarShape::point p1 = locate(v, d1 - offset);
	struct BarShape::point p2 = locate(v, d2 - offset);

	cairo_save(cr);
	cairo_set_line_width(cr, width);
	cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
	cairo_move_to(cr, p1.x, p1.y);
	cairo_line_to(cr, p2.x, p2.y);
	cairo_set_source_rgba(cr, fill[0], fill[1], fill[2], fill[3]);
	cairo_stroke(cr);
	cairo_restore(cr);
}


void BarShape::ticklabel(cairo_t *cr, double v, double d, Font &font, const float *fill, const float *outline, const char *text) {
	int x, y;
	int width, height;

	struct BarShape::point p; // = locate(v, d);

	// Text dimensions
	this->extents(cr, font, text, x, y, width, height);

	// Compute position
	p = locate(v, d);

	// Text offset
	x = -x;
	y = -y;

	// Text position
	x += p.x - (width / 2);
	y += p.y - (height / 2);

	ShapeBase::text(cr, x, y, font, fill, outline, text);
}


//void BarShape::text(cairo_t *cr, double v, double d, int size, const float *color, std::string str) {
//	cairo_text_extents_t extents;
//
//	struct BarShape::point p = locate(v, d);
//
//	cairo_save(cr);
//	cairo_set_font_face(cr, fontface_);
//	cairo_set_font_size(cr, size);
//	cairo_text_extents(cr, str.c_str(), &extents);
//
//	// TODO
//	cairo_set_source_rgba(cr, 0, 0, 0.0, 0.75);
//	cairo_move_to(cr, p.x - (extents.width / 2) + 3, p.y + (extents.height / 2) + 3);
//	cairo_show_text(cr, str.c_str());
//
//	cairo_set_source_rgba(cr, color[0], color[1], color[2], color[3]);
//	cairo_move_to(cr, p.x - (extents.width / 2), p.y + (extents.height / 2));
//	cairo_show_text(cr, str.c_str());
////	cairo_stroke(cr);
//	cairo_restore(cr);
//}


void BarShape::value(cairo_t *cr, double v, BarShape::Font &font,
		const float *fill, const float *outline, const char *text) {
	int x, y;
	int width, height;

	struct BarShape::point p;

	int border = theme().valueBorderWidth();

	const float *bordercolor = theme().needleBorderColor();
	const float *backgroundcolor = theme().needleBackgroundColor();

	double padding = border + font.shadow_distance;

	double distance = theme().needleDistance();

	// Text dimensions
	this->extents(cr, font, text, x, y, width, height);

	// Compute position
	p = locate(v, 0);

	p.x = p.x - (width / 2) - (2 * padding) - (height / 2) - distance;

	// Draw needle
	if (theme().hasFlag(VideoWidget::Theme::FlagNeedle)) {
		cairo_save(cr);
		cairo_move_to(cr, p.x - (width / 2) - padding, p.y + (height / 2) + padding);
		cairo_line_to(cr, p.x + (width / 2) + padding, p.y + (height / 2) + padding);
		cairo_line_to(cr, p.x + (width / 2) + 2 * (height / 3), p.y);
		cairo_line_to(cr, p.x + (width / 2) + padding, p.y - (height / 2) - padding);
		cairo_line_to(cr, p.x - (width / 2) - padding, p.y - (height / 2) - padding);
		cairo_close_path(cr);
		cairo_set_source_rgba(cr, backgroundcolor[0], backgroundcolor[1], backgroundcolor[2], backgroundcolor[3]);
		cairo_fill_preserve(cr);
		cairo_set_line_width(cr, border);
		cairo_set_source_rgba(cr, bordercolor[0], bordercolor[1], bordercolor[2], bordercolor[3]);
		cairo_stroke(cr);
		cairo_restore(cr);
	}

	if (theme().hasFlag(VideoWidget::Theme::FlagValue)) {
		// Text offset
		x = -x;
		y = -y;

		// Text position
		x += p.x - (width / 2);
		y += p.y - (height / 2);

		// Draw text
		ShapeBase::text(cr, x, y, font, fill, outline, text);
	}
}


void BarShape::label(cairo_t *cr, Font &font, const float *fill, const float *outline, const char *text) {
	int x, y;
//	int width, height;

	enum VideoWidget::Theme::Align halign = theme().labelHorizontalAlign();
	enum VideoWidget::Theme::Align valign = theme().labelVerticalAlign();

//	// Text dimensions
//	this->extents(cr, font, text, x, y, width, height);

	// Text offset
	x = -label_x_; //-x;
	y = -label_y_; //-y;

	// Text horizontal position
	if (halign == VideoWidget::Theme::AlignLeft) {
		x += padding_left_;
	}
	else if (halign == VideoWidget::Theme::AlignCenter) {
		x += theme().width() / 2;
		x -= label_width_ / 2;
	}
	else if (halign == VideoWidget::Theme::AlignRight) {
		x += theme().width() - padding_right_;
		x -= label_width_;
	}

	// Text vertical position
	if (valign == VideoWidget::Theme::AlignTop) {
		y += padding_top_;
	}
	else if (valign == VideoWidget::Theme::AlignCenter) {
		y += theme().height() / 2 + padding_top_ - padding_bottom_;
		y -= label_height_ / 2;
	}
	else if (valign == VideoWidget::Theme::AlignBottom) {
		y += theme().height() - padding_bottom_;
		y -= label_height_;
	}

	ShapeBase::text(cr, x, y, font, fill, outline, text);
}


void BarShape::xmlwrite(std::ostream &os) {
	log_call();

	ShapeBase::xmlwrite(os);

	os << "<shape>" << VideoWidget::shape2string(VideoWidget::ShapeBar) << "</shape>" << std::endl;

	os << "<with-gauge>" << VideoWidget::bool2string(theme().hasFlag(VideoWidget::Theme::FlagGauge)) << "</with-gauge>" << std::endl;
	os << "<gauge-rotation>" << theme().gaugeWidth() << "</gauge-rotation>" << std::endl;
	os << "<gauge-flip>" << VideoWidget::bool2string(theme().gaugeFlip()) << "</gauge-flip>" << std::endl;
	os << "<gauge-width>" << theme().gaugeWidth() << "</gauge-width>" << std::endl;
	os << "<gauge-cap>" << VideoWidget::gaugecap2string(theme().gaugeCap()) << "</gauge-cap>" << std::endl;
	os << "<gauge-primary-color>" << VideoWidget::Theme::color2hex(theme().gaugePrimaryColor()) << "</gauge-primary-color>" << std::endl;
	os << "<gauge-secondary-color>" << VideoWidget::Theme::color2hex(theme().gaugeSecondaryColor()) << "</gauge-secondary-color>" << std::endl;
	os << "<gauge-border>" << theme().gaugeBorder() << "</gauge-border>" << std::endl;
	os << "<gauge-border-color>" << VideoWidget::Theme::color2hex(theme().gaugeBorderColor()) << "</gauge-border-color>" << std::endl;
	os << "<gauge-background-color>" << VideoWidget::Theme::color2hex(theme().gaugeBackgroundColor()) << "</gauge-background-color>" << std::endl;

	os << "<with-tick>" << VideoWidget::bool2string(theme().hasFlag(VideoWidget::Theme::FlagTick)) << "</with-tick>" << std::endl;
	os << "<with-tick-label>" << VideoWidget::bool2string(theme().hasFlag(VideoWidget::Theme::FlagTickLabel)) << "</with-tick-label>" << std::endl;
	os << "<tick-size>" << theme().tickSize() << "</tick-size>" << std::endl;
	os << "<tick-color>" << VideoWidget::Theme::color2hex(theme().tickColor()) << "</tick-color>" << std::endl;
	os << "<tick-label-distance>" << theme().tickLabelDistance() << "</tick-label-distance>" << std::endl;
	os << "<tick-label-font-size>" << theme_.tickLabelFontSize() << "</tick-label-font-size>" << std::endl;
	os << "<tick-label-color>" << VideoWidget::Theme::color2hex(theme().tickLabelColor()) << "</tick-label-color>" << std::endl;
	os << "<tick-label-border-color>" << VideoWidget::Theme::color2hex(theme().tickLabelBorderColor()) << "</tick-label-border-color>" << std::endl;

	os << "<with-needle>" << VideoWidget::bool2string(theme().hasFlag(VideoWidget::Theme::FlagNeedle)) << "</with-needle>" << std::endl;
	os << "<needle-distance>" << theme().needleDistance() << "</needle-distance>" << std::endl;
	os << "<needle-border-color>" << VideoWidget::Theme::color2hex(theme().needleBorderColor()) << "</needle-border-color>" << std::endl;
	os << "<needle-background-color>" << VideoWidget::Theme::color2hex(theme().needleBackgroundColor()) << "</needle-background-color>" << std::endl;
}

