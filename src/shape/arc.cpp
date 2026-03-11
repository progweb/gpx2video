#include "log.h"
#include "arc.h"

void ArcShape::arc(cairo_t *cr, int index, double a1, double a2, double offset, double width, double border) {
	struct point p = locate(a2, offset + width);

	const float *fill = theme().arcColor(index);
	const float outline[4] = { 0.0, 0.0, 0.0, 0.0 };

	cairo_save(cr);
//	cr->translate(center_, center_);
//	cr->scale(center_, center_);
	cairo_arc(cr, center_, center_, center_ - offset, DEG2RAD(a1 - correction_), DEG2RAD(a2 - correction_));
	cairo_line_to(cr, p.x, p.y);
	cairo_arc_negative(cr, center_, center_, center_ - offset - width, DEG2RAD(a2 - correction_), DEG2RAD(a1 - correction_));
	cairo_close_path(cr);
	cairo_set_source_rgba(cr, fill[0], fill[1], fill[2], fill[3]);
	cairo_fill_preserve(cr);
	cairo_set_line_width(cr, border);
	cairo_set_source_rgba(cr, outline[0], outline[1], outline[2], outline[3]);
	cairo_stroke(cr);
	cairo_restore(cr);
}

void ArcShape::pieslice(cairo_t *cr, double a1, double a2, double border) {
	const float *fill = theme().backgroundColor();
	const float *outline = theme().borderColor();

	cairo_save(cr);
	cairo_arc(cr, center_, center_, center_, DEG2RAD(a1 - correction_), DEG2RAD(a2 - correction_));

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

void ArcShape::line(cairo_t *cr, double a, double d1, double d2) {
	struct point p1 = locate(a, d1);
	struct point p2 = locate(a, d2);

	const float *fill = theme().tickColor();

	cairo_save(cr);
	cairo_move_to(cr, p1.x, p1.y);
	cairo_line_to(cr, p2.x, p2.y);
	cairo_set_source_rgba(cr, fill[0], fill[1], fill[2], fill[3]);
	cairo_stroke(cr);
	cairo_restore(cr);
}

void ArcShape::text(cairo_t *cr, double a, double d, std::string str) {
	cairo_text_extents_t extents;

	struct ArcShape::point p = locate(a, d);

	cairo_save(cr);
	cairo_set_font_face(cr, fontface_);
	cairo_set_font_size(cr, size_ / 15);
	cairo_text_extents(cr, str.c_str(), &extents);
	cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
	cairo_move_to(cr, p.x - (extents.width / 2), p.y + (extents.height / 2));
	cairo_show_text(cr, str.c_str());
	cairo_stroke(cr);
	cairo_restore(cr);
}

