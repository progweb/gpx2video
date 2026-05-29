#include "../log_i.h"
#include "arc.h"


void ArcShape::background(cairo_t *cr, double border, const float *fill, const float *outline) {
	cairo_save(cr);
	cairo_arc(cr, center_x_, center_y_, size_ / 2, 0, 2 * M_PI);
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


void ArcShape::arc_i(cairo_t *cr, double a1, double a2, double offset, double width, double border, const float *fill, const float *outline) {
	struct point p1e = locate(a1, offset);
	struct point p1i = locate(a1, offset + width);

	struct point p2e = locate(a2, offset);
	struct point p2i = locate(a2, offset + width);

	VideoWidget::Theme::GaugeCap cap = theme().gaugeCap();

	switch (cap) {
	case VideoWidget::Theme::GaugeCapRound:
		cairo_save(cr);
		cairo_arc(cr, center_x_, center_y_, size_ / 2 - offset, DEG2RAD(a1 - correction_), DEG2RAD(a2 - correction_));
		cairo_arc(cr, (p2i.x + p2e.x) / 2.0, (p2i.y + p2e.y) / 2.0, width / 2.0, DEG2RAD(a2 - correction_), M_PI + DEG2RAD(a2 - correction_));
		cairo_arc_negative(cr, center_x_, center_y_, size_ / 2 - offset - width, DEG2RAD(a2 - correction_), DEG2RAD(a1 - correction_));
		cairo_arc(cr, (p1i.x + p1e.x) / 2.0, (p1i.y + p1e.y) / 2.0, width / 2.0, M_PI + DEG2RAD(a1 - correction_), DEG2RAD(a1 - correction_));
		cairo_close_path(cr);
		cairo_set_source_rgba(cr, fill[0], fill[1], fill[2], fill[3]);
		cairo_fill_preserve(cr);
		if ((border > 0) && (outline != NULL)) {
			cairo_set_line_width(cr, border);
			cairo_set_source_rgba(cr, outline[0], outline[1], outline[2], outline[3]);
		}
		cairo_stroke(cr);
		cairo_restore(cr);
		break;

	case VideoWidget::Theme::GaugeCapSquare:
	default:
		cairo_save(cr);
		cairo_arc(cr, center_x_, center_y_, size_ / 2 - offset, DEG2RAD(a1 - correction_), DEG2RAD(a2 - correction_));
		cairo_line_to(cr, p2i.x, p2i.y);
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
		break;
	}
}


void ArcShape::arc(cairo_t *cr, double a1, double a2, double offset, double width, double border, const float *fill, const float *outline) {
	arc_i(cr, std::min(a1, a2), std::max(a1, a2), offset, width, border, fill, outline);
}


void ArcShape::pieslice(cairo_t *cr, double a1, double a2, double border, const float *fill, const float *outline) {
	cairo_save(cr);
	cairo_arc(cr, center_x_, center_y_, size_ / 2, DEG2RAD(a1 - correction_), DEG2RAD(a2 - correction_));
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


void ArcShape::line(cairo_t *cr, double a, double d1, double d2, double width, const float *fill) {
	struct point p1 = locate(a, d1);
	struct point p2 = locate(a, d2);

	cairo_save(cr);
	cairo_set_line_width(cr, width);
	cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
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


void ArcShape::value(cairo_t *cr, ArcShape::Font &font, 
		const float *fill, const float *outline, const char *text) {
	int x, y;
	int width, height;

	enum VideoWidget::Theme::Align textAlign = theme().valueAlign();

	this->extents(cr, font, text, x, y, width, height);

	// Text offset
	x = -x;
	y = -y + font.shadow_distance;

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

	y += theme().height() - font.shadow_distance - padding_bottom_ - height;

	ShapeBase::text(cr, x, y, font, fill, outline, text);
}


void ArcShape::unit(cairo_t *cr, ArcShape::Font &font, 
		const float *fill, const float *outline, const char *text) {
	int x, y;
	int width, height;

	enum VideoWidget::Theme::Align textAlign = theme().valueAlign();

	this->extents(cr, font, text, x, y, width, height);

	// Text offset
	x = -x;
	y = -y + font.shadow_distance;

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

	y += theme().height() - font.shadow_distance - padding_bottom_;
	y += height / 3.0;

	ShapeBase::text(cr, x, y, font, fill, outline, text);
}


void ArcShape::needle(cairo_t *cr, VideoWidget::Theme::NeedleType type,
		double xa, double len, bool design, const float *color1, const float *color2) {
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
		if (design) {
			cairo_set_line_width(cr, size_ / 36);
			cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);

			p = locate(xa - 0, (size_ / 2.1));
			cairo_move_to(cr, p.x, p.y);

			p = locate(xa - 0, (size_ / 1.65));
			cairo_line_to(cr, p.x, p.y);

			cairo_stroke(cr);
		}

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

		//
		if (design) {
			cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);

			cairo_arc(cr, size_ / 2, size_ / 2, size_ / 512, 0, 2 * M_PI);
			cairo_fill_preserve(cr);
			cairo_stroke(cr);
		}

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
		//
		fill = color2;
		cairo_set_source_rgba(cr, fill[0], fill[1], fill[2], fill[3]);

		if (design) {
			cairo_arc(cr, size_ / 2, size_ / 2, size_ / 15, 0, 2 * M_PI);
			cairo_close_path(cr);
			cairo_fill_preserve(cr);
			cairo_stroke(cr);
		}

		//
		fill = color1;
		cairo_set_source_rgba(cr, fill[0], fill[1], fill[2], fill[3]);

		cairo_set_line_width(cr, size_ / 64);
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

			p = locate(xa - 0, (size_ / 1.65));
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


void ArcShape::xmlwrite(std::ostream &os) {
	log_call();

	ShapeBase::xmlwrite(os);

	os << "<shape>" << VideoWidget::shape2string(VideoWidget::ShapeArc) << "</shape>" << std::endl;

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
	os << "<tick-label-color>" << VideoWidget::Theme::color2hex(theme().tickLabelColor()) << "</tick-label-color>" << std::endl;

	os << "<with-needle>" << VideoWidget::bool2string(theme().hasFlag(VideoWidget::Theme::FlagNeedle)) << "</with-needle>" << std::endl;
	os << "<needle-type>" << VideoWidget::needletype2string(theme().needleType()) << "</needle-type>" << std::endl;
	os << "<needle-primary-color>" << VideoWidget::Theme::color2hex(theme().needlePrimaryColor()) << "</needle-primary-color>" << std::endl;
	os << "<needle-secondary-color>" << VideoWidget::Theme::color2hex(theme().needleSecondaryColor()) << "</needle-secondary-color>" << std::endl;
}

