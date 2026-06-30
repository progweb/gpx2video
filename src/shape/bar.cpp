#include <librsvg/rsvg.h>
#include <pango/pangocairo.h>

#include "../log_i.h"
#include "bar.h"


void BarShape::bar(cairo_t *cr, double v1, double v2, double width, double border,
		const float *fill, const float *outline) {
	double w, h;
	double a1, a2;

	struct BarShape::point p1, p2; 

	VideoWidget::Theme::GaugeCap cap = theme().gaugeCap();

#define DEG2RAD(a) ((a) * M_PI / 180.0)

	// Scaling
	width = size2pixels(width);
	border = size2pixels(border) / 2.0;

	// Compute position
	p1 = locate(v1, -width / 2);
	p2 = locate(v2, -width / 2);

	// Draw
	switch (cap) {
	case VideoWidget::Theme::GaugeCapRound:
		if (orientation_ == VideoWidget::OrientationVertical) {
			a1 = 180;
			a2 = 0;
			w = width;
			h = 0;
		}
		else {
			a1 = -90;
			a2 = 90;
			w = 0;
			h = width;
		}

		cairo_save(cr);
		cairo_move_to(cr, p1.x, p1.y);
		cairo_line_to(cr, p2.x, p2.y);
		cairo_arc(cr, p2.x + (w / 2), p2.y + (h / 2), width / 2, DEG2RAD(a1), DEG2RAD(a2));
		cairo_line_to(cr, p1.x + w, p1.y + h);
		cairo_arc_negative(cr, p1.x + (w / 2), p1.y + (h / 2), width / 2, DEG2RAD(a1), DEG2RAD(a2));
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
		if (orientation_ == VideoWidget::OrientationVertical) {
			w = width;
			h = p2.y - p1.y;
		}
		else {
			w = p2.x - p1.x;
			h = width;
		}

		cairo_save(cr);
		cairo_rectangle(cr, p1.x, p1.y, w, h);
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


void BarShape::cursor(cairo_t *cr, double v, double width, const float *fill) {
	double size;

	struct BarShape::point p1, p2;

	// Cursor size
	size = theme().gaugeWidth();

	// Scaling
	size = size2pixels(size);
	width = size2pixels(width);

	// Compute position
	p1 = locate(v, -size / 2);
	p2 = locate(v, size / 2);

	// Draw cursor
	cairo_save(cr);
	cairo_set_line_width(cr, width);
	cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
	cairo_move_to(cr, p1.x, p1.y);
	cairo_line_to(cr, p2.x, p2.y);
	cairo_set_source_rgba(cr, fill[0], fill[1], fill[2], fill[3]);
	cairo_stroke(cr);
	cairo_restore(cr);
}


void BarShape::line(cairo_t *cr, double v, double d1, double d2, double width, const float *fill) {
	double offset;

	struct BarShape::point p1, p2;

	// Scaling
	d1 = size2pixels(d1);
	d2 = size2pixels(d2);
   
	// Compute position
	offset = (d2 - d1) / 2.0;

	p1 = locate(v, d1 - offset);
	p2 = locate(v, d2 - offset);

	// Draw
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
	this->extents(cr, font, BarShape::TextAlpha, text, x, y, width, height);

	// Compute position
	p = locate(v, size2pixels(d));

	// Text offset
	x = -x;
	y = -y;

	// Text position
	x += p.x - (width / 2);
	y += p.y - (height / 2);

	ShapeBase::text(cr, x, y, font, fill, outline, text);
}


void BarShape::label(cairo_t *cr, Font &font, const float *fill, const float *outline, const char *text) {
	int x, y;

	enum VideoWidget::Theme::Align halign = theme().labelHorizontalAlign();
	enum VideoWidget::Theme::Align valign = theme().labelVerticalAlign();

	// Text offset
	x = -label_x_; //-x;
	y = -label_y_; //-y;

	// Text horizontal position
	if (halign == VideoWidget::Theme::AlignLeft) {
		x += theme().border() + theme().padding(VideoWidget::Theme::PaddingLeft);
	}
	else if (halign == VideoWidget::Theme::AlignCenter) {
		x += theme().width() / 2;
		x -= label_width_ / 2;
	}
	else if (halign == VideoWidget::Theme::AlignRight) {
		x += theme().width() - theme().border() - theme().padding(VideoWidget::Theme::PaddingRight);
		x -= label_width_;
	}

	// Text vertical position
	if (valign == VideoWidget::Theme::AlignTop) {
		y += theme().border() + theme().padding(VideoWidget::Theme::PaddingTop);
	}
	else if (valign == VideoWidget::Theme::AlignCenter) {
		y += theme().height() / 2 + theme().padding(VideoWidget::Theme::PaddingTop) - theme().padding(VideoWidget::Theme::PaddingBottom);
		y -= label_height_ / 2;
	}
	else if (valign == VideoWidget::Theme::AlignBottom) {
		y += theme().height() - theme().border() - theme().padding(VideoWidget::Theme::PaddingBottom);
		y -= label_height_;
	}

	ShapeBase::text(cr, x, y, font, fill, outline, text);
}


void BarShape::value(cairo_t *cr, double v, BarShape::Font &font,
		const float *fill, const float *outline, const char *text) {
	int x, y;
	int width, height;

	int border;

	double size;
	double needlesize;
	double distance;

	struct BarShape::point p;

	double padding;

	// Scaling
	border = size2pixels(theme().needleBorder()) / 2.0;
	distance = size2pixels(theme().needleDistance());

	// Compute icon size
	size = 48;

	// Padding
	padding = border + shadow2pixels(font);

	// Text dimensions
	this->extents(cr, font, BarShape::TextNumeric, text, x, y, width, height);

	// Needle size
	if (theme().hasFlag(VideoWidget::Theme::FlagNeedle)) {
		if (theme().needleType() == VideoWidget::Theme::NeedleTypeBasic) {
			if (orientation_ == VideoWidget::OrientationVertical)
				needlesize = 2 * (size / 6);
			else
				needlesize = 2 * (size / 6 / 2);
		}
		else if (theme().needleType() == VideoWidget::Theme::NeedleTypeValue)
			needlesize = 2 * (height / 3);
		else
			needlesize = 0;
	}
	else 
		needlesize = 0;

	// Compute position
	p = locate(v, 0);

	if (orientation_ == VideoWidget::OrientationVertical)
		p.x = p.x - (width / 2) - padding - needlesize - distance;
	else
		p.y  = p.y - (height / 2) - padding - needlesize - distance;

	// Text offset
	x = -x;
	y = -y;

	// Text position
	x += p.x - (width / 2);
	y += p.y - (height / 2);

	// Draw text
	ShapeBase::text(cr, x, y, font, fill, outline, text);
}


void BarShape::needle(cairo_t *cr, VideoWidget::Theme::NeedleType type,
	double v, BarShape::Font &font, const char *text,
	double border, const float *fill, const float *outline) {
	int x, y;
	int width, height;

	double size;
	double needlesize;
	double distance;

	double padding;

	struct BarShape::point p;

	// Scaling
	border = size2pixels(border) / 2.0;
	distance = size2pixels(theme().needleDistance());

	// Compute icon size
	size = 48;

	// Padding
	padding = border + shadow2pixels(font);

	// Compute position
	p = locate(v, 0);

	switch (type) {
	case VideoWidget::Theme::NeedleTypeBasic:
	case VideoWidget::Theme::NeedleTypeIcon:
		// Needle size
		needlesize = size / 6;

		// Compute position
		if (orientation_ == VideoWidget::OrientationVertical)
			p.x = p.x - needlesize - distance;
		else
			p.y  = p.y - needlesize - distance;

		cairo_save(cr);

		if (orientation_ == VideoWidget::OrientationVertical) {
			cairo_move_to(cr, p.x, p.y - needlesize);
			cairo_line_to(cr, p.x, p.y + needlesize);
			cairo_line_to(cr, p.x + needlesize, p.y);
		}
		else {
			cairo_move_to(cr, p.x - needlesize, p.y);
			cairo_line_to(cr, p.x + needlesize, p.y);
			cairo_line_to(cr, p.x, p.y + needlesize);
		}

		cairo_close_path(cr);
		cairo_set_source_rgba(cr, fill[0], fill[1], fill[2], fill[3]);
		cairo_fill_preserve(cr);
		cairo_set_line_width(cr, border);
		cairo_set_source_rgba(cr, outline[0], outline[1], outline[2], outline[3]);
		cairo_stroke(cr);
		cairo_restore(cr);
		break;

	case VideoWidget::Theme::NeedleTypeValue:
		// Text dimensions
		this->extents(cr, font, BarShape::TextNumeric, text, x, y, width, height);

		// Needle size
		needlesize = 2 * (height / 3);

		// Compute position
		p = locate(v, 0);

		if (orientation_ == VideoWidget::OrientationVertical)
			p.x = p.x - (width / 2) - padding - needlesize - distance;
		else
			p.y  = p.y - (height / 2) - padding - needlesize - distance;

		// Draw needle
		cairo_save(cr);

		if (orientation_ == VideoWidget::OrientationVertical) {
			cairo_move_to(cr, p.x - (width / 2) - padding, p.y + (height / 2) + padding);
			cairo_line_to(cr, p.x + (width / 2) + padding, p.y + (height / 2) + padding);
			cairo_line_to(cr, p.x + (width / 2) + needlesize + padding, p.y);
			cairo_line_to(cr, p.x + (width / 2) + padding, p.y - (height / 2) - padding);
			cairo_line_to(cr, p.x - (width / 2) - padding, p.y - (height / 2) - padding);
		}
		else {
			cairo_move_to(cr, p.x - (width / 2) - padding, p.y + (height / 2) + padding);
			cairo_line_to(cr, p.x, p.y + (height / 2) + needlesize + padding);
			cairo_line_to(cr, p.x + (width / 2) + padding, p.y + (height / 2) + padding);
			cairo_line_to(cr, p.x + (width / 2) + padding, p.y - (height / 2) - padding);
			cairo_line_to(cr, p.x - (width / 2) - padding, p.y - (height / 2) - padding);
		}

		cairo_close_path(cr);
		cairo_set_source_rgba(cr, fill[0], fill[1], fill[2], fill[3]);
		cairo_fill_preserve(cr);
		cairo_set_line_width(cr, border);
		cairo_set_source_rgba(cr, outline[0], outline[1], outline[2], outline[3]);
		cairo_stroke(cr);
		cairo_restore(cr);

		break;

	default:
		break;
	}
}


void BarShape::icon(cairo_t *cr, double v, const std::string &filename, const float *fill) {
	bool apply_color;

	struct BarShape::point p;

    GError *error = NULL;

	cairo_t *mask = NULL;
	cairo_surface_t *surface = NULL;

	RsvgRectangle viewport;

	RsvgHandle *handle = NULL;

	double size;
	double needlesize;
	double distance;

	if (filename.empty())
		return;

	// Set color
	apply_color = (fill != NULL) && (fill[3] != 0);

	// load svg data
	handle = rsvg_handle_new_from_file(filename.c_str(), &error);
    if (!handle) {
        log_error("Load svn image failure: %s", error->message);
        g_error_free(error);
        return;
    }

	// Scaling
	distance = size2pixels(theme().needleDistance());

	// Compute icon size
	size = size2pixels(theme().iconSize());

	// Compute position
	p = locate(v, 0);

	needlesize = theme().hasFlag(VideoWidget::Theme::FlagNeedle) ? (size / 6) : 0;

	if (orientation_ == VideoWidget::OrientationVertical)
		p.x = p.x - (size / 2) - needlesize - distance;
	else
		p.y  = p.y - (size / 2) - needlesize - distance;

	if (apply_color) {
		// Create alpha only surface
		surface = cairo_image_surface_create(CAIRO_FORMAT_A8, theme().width(), theme().height());

		// Create mask
		mask = cairo_create(surface);
	}

	// Render svg into cairo surface
	viewport = (RsvgRectangle) {
		.x = (double) p.x - (size / 2),
		.y = (double) p.y - (size / 2),
		.width = (double) size,
		.height = (double) size
	};

	rsvg_handle_render_document(handle, apply_color ? mask : cr, &viewport, NULL);

	if (apply_color) {
		// Apply color
		cairo_set_source_rgba(cr, fill[0], fill[1], fill[2], fill[3]);

		// Paint mask
		cairo_mask_surface(cr, surface, 0, 0);
	}

	// Free
	if (surface)
		cairo_surface_destroy(surface);
	if (mask)
		cairo_destroy(mask);

	if (handle != NULL)
		g_object_unref(handle);
}


void BarShape::xmlwrite(std::ostream &os) {
	log_call();

	ShapeBase::xmlwrite(os);

	os << "<shape>" << VideoWidget::shape2string(VideoWidget::ShapeBar) << "</shape>" << std::endl;

	os << "<with-gauge>" << VideoWidget::bool2string(theme().hasFlag(VideoWidget::Theme::FlagGauge)) << "</with-gauge>" << std::endl;
	os << "<gauge-orientation>" << VideoWidget::orientation2string(theme().gaugeOrientation()) << "</gauge-orientation>" << std::endl;
	os << "<gauge-flip>" << VideoWidget::bool2string(theme().gaugeFlip()) << "</gauge-flip>" << std::endl;
	os << "<gauge-width>" << theme().gaugeWidth() << "</gauge-width>" << std::endl;
	os << "<gauge-offset>" << theme().gaugeOffset() << "</gauge-offset>" << std::endl;
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
	os << "<needle-type>" << VideoWidget::needletype2string(theme().needleType()) << "</needle-type>" << std::endl;
	os << "<needle-distance>" << theme().needleDistance() << "</needle-distance>" << std::endl;
	os << "<needle-border>" << theme().needleBorder() << "</needle-border>" << std::endl;
	os << "<needle-border-color>" << VideoWidget::Theme::color2hex(theme().needleBorderColor()) << "</needle-border-color>" << std::endl;
	os << "<needle-background-color>" << VideoWidget::Theme::color2hex(theme().needleBackgroundColor()) << "</needle-background-color>" << std::endl;

	os << "<icon-name>" << VideoWidget::icon2string(theme_.icon()) << theme_.iconFile() << "</icon-name>" << std::endl;
	os << "<icon-size>" << theme_.iconSize() << "</icon-size>" << std::endl;
	os << "<icon-color>" << VideoWidget::Theme::color2hex(theme_.iconColor()) << "</icon-color>" << std::endl;
}

