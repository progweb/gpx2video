#include "log.h"
#include "bar.h"


void BarShape::bar(cairo_t *cr, double v1, double v2, double width, double border,
		const float *fill, const float *outline) {
	struct BarShape::point p1 = locate(v1, 0);
	struct BarShape::point p2 = locate(v2, 0);

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


void BarShape::cursor(cairo_t *cr, double v, double width) {
	double thick = width / 8.0;

	struct BarShape::point p = locate(v, 0);

	const float *fill = theme().cursorColor();

	cairo_save(cr);
	cairo_rectangle(cr, p.x, p.y - (thick / 2), width, thick);
	cairo_set_source_rgba(cr, fill[0], fill[1], fill[2], fill[3]);
	cairo_fill_preserve(cr);
	cairo_stroke(cr);
	cairo_restore(cr);
}


void BarShape::line(cairo_t *cr, double v, double d1, double d2, const float *fill) {
	struct BarShape::point p1 = locate(v, d1);
	struct BarShape::point p2 = locate(v, d2);

	cairo_save(cr);
	cairo_move_to(cr, p1.x, p1.y);
	cairo_line_to(cr, p2.x, p2.y);
	cairo_set_source_rgba(cr, fill[0], fill[1], fill[2], fill[3]);
	cairo_stroke(cr);
	cairo_restore(cr);
}


void BarShape::text(cairo_t *cr, double v, double d, int size, const float *color, std::string str) {
	cairo_text_extents_t extents;

	struct BarShape::point p = locate(v, d);

	cairo_save(cr);
	cairo_set_font_face(cr, fontface_);
	cairo_set_font_size(cr, size);
	cairo_text_extents(cr, str.c_str(), &extents);

	// TODO
	cairo_set_source_rgba(cr, 0, 0, 0.0, 0.75);
	cairo_move_to(cr, p.x - (extents.width / 2) + 3, p.y + (extents.height / 2) + 3);
	cairo_show_text(cr, str.c_str());

	cairo_set_source_rgba(cr, color[0], color[1], color[2], color[3]);
	cairo_move_to(cr, p.x - (extents.width / 2), p.y + (extents.height / 2));
	cairo_show_text(cr, str.c_str());
//	cairo_stroke(cr);
	cairo_restore(cr);
}


void BarShape::marker(cairo_t *cr, double v, 
		int border, const float *color, const float *fill, const float *outline, std::string str) {
	double padding;

	struct BarShape::point p;

	cairo_text_extents_t extents;

	p = locate(v, 0);

	cairo_save(cr);
	cairo_set_font_face(cr, fontface_);
	cairo_set_font_size(cr, size_ / 14);
	cairo_text_extents(cr, str.c_str(), &extents);

	padding = size_ / 24;
	p.x = p.x - (extents.width / 2) - padding;

	//
	cairo_move_to(cr, p.x - (extents.width / 2) - padding, p.y + (extents.height / 2) + padding);
	cairo_line_to(cr, p.x + (extents.width / 2) + padding, p.y + (extents.height / 2) + padding);
	cairo_line_to(cr, p.x + (extents.width / 2) + 2 * padding, p.y);
	cairo_line_to(cr, p.x + (extents.width / 2) + padding, p.y - (extents.height / 2) - padding);
	cairo_line_to(cr, p.x - (extents.width / 2) - padding, p.y - (extents.height / 2) - padding);
	cairo_close_path(cr);
	cairo_set_source_rgba(cr, fill[0], fill[1], fill[2], fill[3]);
	cairo_fill_preserve(cr);
	cairo_set_line_width(cr, border);
	cairo_set_source_rgba(cr, outline[0], outline[1], outline[2], outline[3]);
	cairo_stroke(cr);

	//
	cairo_set_source_rgba(cr, color[0], color[1], color[2], color[3]);
	cairo_move_to(cr, p.x - (extents.width / 2), p.y + (extents.height / 2));
	cairo_show_text(cr, str.c_str());
	cairo_stroke(cr);
	cairo_restore(cr);
}


void BarShape::label(cairo_t *cr, int x, int y, const float *color, std::string str) {
	struct BarShape::point p;

	cairo_text_extents_t extents;

	cairo_save(cr);
	cairo_set_font_face(cr, fontface_);
	cairo_set_font_size(cr, size_ / 10);
	cairo_text_extents(cr, str.c_str(), &extents);

//	p.x = center_ - (extents.width / 2);
////	p.y = length_ - margin_; // - (extents.height / 2);
//	p.y = length_ + extents.height - padding_bottom_; // - (extents.height / 2);

	p.x = x - (extents.width / 2);
	p.y = y; // + extents.height;

	//
	cairo_set_source_rgba(cr, color[0], color[1], color[2], color[3]);
	cairo_move_to(cr, p.x, p.y);
	cairo_show_text(cr, str.c_str());
	cairo_stroke(cr);
	cairo_restore(cr);
}


void BarShape::xmlwrite(std::ostream &os) {
	log_call();

	ShapeBase::xmlwrite(os);

	os << "<shape>bar</shape>" << std::endl;

	os << "<value-background-color>" << theme_.color2hex(theme_.valueBackgroundColor()) << "</value-background-color>" << std::endl;

	os << "<with-tick>" << VideoWidget::bool2string(theme().hasFlag(VideoWidget::Theme::FlagTick)) << "</with-tick>" << std::endl;
	os << "<with-tick-label>" << VideoWidget::bool2string(theme().hasFlag(VideoWidget::Theme::FlagTickLabel)) << "</with-tick-label>" << std::endl;
	os << "<tick-color>" << VideoWidget::Theme::color2hex(theme().tickColor()) << "</tick-color>" << std::endl;
	os << "<tick-label-color>" << VideoWidget::Theme::color2hex(theme().tickLabelColor()) << "</tick-label-color>" << std::endl;

	os << "<with-gauge>" << VideoWidget::bool2string(theme().hasFlag(VideoWidget::Theme::FlagGauge)) << "</with-gauge>" << std::endl;
	os << "<gauge-color>" << VideoWidget::Theme::color2hex(theme().gaugeColor(0)) << "</gauge-color>" << std::endl;
	os << "<gauge-border>" << theme().gaugeBorder() << "</gauge-border>" << std::endl;
	os << "<gauge-border-color>" << VideoWidget::Theme::color2hex(theme().gaugeBorderColor()) << "</gauge-border-color>" << std::endl;
	os << "<gauge-background-color>" << VideoWidget::Theme::color2hex(theme().gaugeBackgroundColor()) << "</gauge-background-color>" << std::endl;
}

