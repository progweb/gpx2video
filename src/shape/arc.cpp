#include "log.h"
#include "arc.h"


void ArcShape::arc(cairo_t *cr, double a1, double a2, double offset, double width, double border, const float *fill, const float *outline) {
	struct point p = locate(a2, offset + width);

	cairo_save(cr);
//	cr->translate(center_, center_);
//	cr->scale(center_, center_);
	cairo_arc(cr, center_x_, center_y_, size_ / 2 - offset, DEG2RAD(a1 - correction_), DEG2RAD(a2 - correction_));
	cairo_line_to(cr, p.x, p.y);
	cairo_arc_negative(cr, center_x_, center_y_, size_ / 2 - offset - width, DEG2RAD(a2 - correction_), DEG2RAD(a1 - correction_));
	cairo_close_path(cr);
	cairo_set_source_rgba(cr, fill[0], fill[1], fill[2], fill[3]);
	cairo_fill_preserve(cr);
	if ((border > 0) && (outline != NULL)) {
		cairo_set_line_width(cr, border);
		cairo_set_source_rgba(cr, outline[0], outline[1], outline[2], outline[3]);
	}
	cairo_stroke(cr);
	cairo_restore(cr);
}


void ArcShape::pieslice(cairo_t *cr, double a1, double a2, double border) {
	const float *fill = theme().backgroundColor();
	const float *outline = theme().borderColor();

	cairo_save(cr);
	cairo_arc(cr, center_x_, center_y_, size_ / 2, DEG2RAD(a1 - correction_), DEG2RAD(a2 - correction_));

	if (a2 - a1 < 360) {
		cairo_line_to(cr, 0, 0);
		cairo_close_path(cr);
	}

	// BGRA
	cairo_set_source_rgba(cr, fill[0], fill[1], fill[2], fill[3]);
	cairo_fill_preserve(cr);
	cairo_set_line_width(cr, border);
	cairo_set_source_rgba(cr, outline[0], outline[1], outline[2], outline[3]);
	cairo_stroke(cr);
	cairo_restore(cr);
}


void ArcShape::line(cairo_t *cr, double a, double d1, double d2, const float *fill) {
	struct point p1 = locate(a, d1);
	struct point p2 = locate(a, d2);

	cairo_save(cr);
	cairo_move_to(cr, p1.x, p1.y);
	cairo_line_to(cr, p2.x, p2.y);
	cairo_set_source_rgba(cr, fill[0], fill[1], fill[2], fill[3]);
	cairo_stroke(cr);
	cairo_restore(cr);
}


void ArcShape::text(cairo_t *cr, double a, double d, 
		const float *fill, std::string str) {
	cairo_text_extents_t extents;

	struct ArcShape::point p = locate(a, d);

	cairo_save(cr);
	cairo_set_font_face(cr, fontface_);
	cairo_set_font_size(cr, size_ / 15);
	cairo_text_extents(cr, str.c_str(), &extents);
	cairo_set_source_rgba(cr, fill[0], fill[1], fill[2], fill[3]);
	cairo_move_to(cr, p.x - (extents.width / 2), p.y + (extents.height / 2));
	cairo_show_text(cr, str.c_str());
	cairo_stroke(cr);
	cairo_restore(cr);
}


void ArcShape::xmlwrite(std::ostream &os) {
	log_call();

	ShapeBase::xmlwrite(os);

	os << "<with-unit>" << VideoWidget::bool2string(theme().hasFlag(VideoWidget::Theme::FlagUnit)) << "</with-unit>" << std::endl;

	os << "<with-tick>" << VideoWidget::bool2string(theme().hasFlag(VideoWidget::Theme::FlagTick)) << "</with-tick>" << std::endl;
	os << "<tick-color>" << theme().color2hex(theme().tickColor()) << "</tick-color>" << std::endl;
	os << "<tick-label-color>" << theme().color2hex(theme().tickLabelColor()) << "</tick-label-color>" << std::endl;

	os << "<with-gauge>" << VideoWidget::bool2string(theme().hasFlag(VideoWidget::Theme::FlagGauge)) << "</with-gauge>" << std::endl;
	os << "<gauge-color>" << theme().color2hex(theme().gaugeColor(0)) << "</gauge-color>" << std::endl;
	os << "<gauge-border>" << theme().gaugeBorder() << "</gauge-border>" << std::endl;
	os << "<gauge-border-color>" << theme().color2hex(theme().gaugeBorderColor()) << "</gauge-border-color>" << std::endl;
	os << "<gauge-background-color>" << theme().color2hex(theme().gaugeBackgroundColor()) << "</gauge-background-color>" << std::endl;

	os << "<with-needle>" << VideoWidget::bool2string(theme().hasFlag(VideoWidget::Theme::FlagNeedle)) << "</with-needle>" << std::endl;
	os << "<needle-color>" << theme().color2hex(theme().needleColor()) << "</needle-color>" << std::endl;
}

