#include <librsvg/rsvg.h>
#include <pango/pangocairo.h>

#include "../log_i.h"
#include "chart.h"


void ChartShape::axis(cairo_t *cr, double width, double border, const float *fill, const float *outline) {
	double x1, y1;
	double x2, y2;

	struct point p1, p2;

	// Scaling
	width = size2pixels(width);
	border = size2pixels(border);

	// x-axis
	x1 = scale(x_min_, x_max_, x_min_);
	y1 = scale(y_min_, y_max_, y_min_);

	x2 = scale(x_min_, x_max_, x_max_);
	y2 = scale(y_min_, y_max_, y_min_);

	// Compute x-axis position
	p1 = locate(x1, y1);
	p2 = locate(x2, y2);

	// Draw x-axis
	cairo_save(cr);
	cairo_set_line_width(cr, width + border);
	cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
	cairo_move_to(cr, p1.x, p1.y);
	cairo_line_to(cr, p2.x, p2.y);
	cairo_set_source_rgba(cr, outline[0], outline[1], outline[2], outline[3]);
	cairo_stroke(cr);
	cairo_restore(cr);

	// Draw x-axis
	cairo_save(cr);
	cairo_set_line_width(cr, width);
	cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
	cairo_move_to(cr, p1.x, p1.y);
	cairo_line_to(cr, p2.x, p2.y);
	cairo_set_source_rgba(cr, fill[0], fill[1], fill[2], fill[3]);
	cairo_stroke(cr);
	cairo_restore(cr);

	// y-axis
	x1 = scale(x_min_, x_max_, x_min_);
	y1 = scale(y_min_, y_max_, y_min_);

	x2 = scale(x_min_, x_max_, x_min_);
	y2 = scale(y_min_, y_max_, y_max_);

	// Compute x-axis position
	p1 = locate(x1, y1);
	p2 = locate(x2, y2);

	// Draw y-axis
	cairo_save(cr);
	cairo_set_line_width(cr, width + border);
	cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
	cairo_move_to(cr, p1.x, p1.y);
	cairo_line_to(cr, p2.x, p2.y);
	cairo_set_source_rgba(cr, outline[0], outline[1], outline[2], outline[3]);
	cairo_stroke(cr);
	cairo_restore(cr);

	// Draw y-axis
	cairo_save(cr);
	cairo_set_line_width(cr, width);
	cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
	cairo_move_to(cr, p1.x, p1.y);
	cairo_line_to(cr, p2.x, p2.y);
	cairo_set_source_rgba(cr, fill[0], fill[1], fill[2], fill[3]);
	cairo_stroke(cr);
	cairo_restore(cr);
}


void ChartShape::cursor(cairo_t *cr, double x, double y, double width, const float *fill) {
	double x1, y1;
	double x2, y2;

	struct ChartShape::point p1, p2;

	// Scaling
	width = size2pixels(width);

	// cursor
	x1 = x;
	y1 = scale(y_min_, y_max_, std::max(y_min_, std::min(y_max_, 0.0)));

	x2 = x;
	y2 = y;

	// Compute x-axis position
	p1 = locate(x1, y1);
	p2 = locate(x2, y2);

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


void ChartShape::value(cairo_t *cr, double y, ChartShape::Font &font,
		const float *fill, const float *outline, const char *text) {
	double x;

	int txtx, txty;
	int width, height;

	double padding;

	struct ChartShape::point p;

	// Padding
	padding = theme().hasFlag(VideoWidget::Theme::FlagAll) ? 
		theme().axisBorder() + theme().axisThick() : 0.0;

	// Scaling
	padding = size2pixels(padding);

	// x
	x = scale(x_min_, x_max_, x_min_);

	// Compute position
	p = locate(x, y);

	// Text dimensions
	this->extents(cr, font, ChartShape::TextNumeric, text, txtx, txty, width, height);

	// Text offset
	x = -txtx;
	y = -txty;

	// Text position
	x += p.x - width - padding;
	y += p.y - (height / 2);

	// Draw text
	ShapeBase::text(cr, x, y, font, fill, outline, text);
}


void ChartShape::value(cairo_t *cr, double x, double y, ChartShape::Font &font,
		const float *fill, const float *outline, const char *text) {
	int txtx, txty;
	int width, height;

	int border;

	double size;
	double needlesize;
	double distance;

	struct ChartShape::point p;

	double padding;

	// Scaling
	border = size2pixels(theme().needleBorder()) / 2.0;
	distance = size2pixels(theme().needleDistance());

	// Compute icon size
	size = 48;

	// Padding
	padding = border + shadow2pixels(font);

	// Text dimensions
	this->extents(cr, font, ChartShape::TextNumeric, text, txtx, txty, width, height);

	// Needle size
	if (theme().hasFlag(VideoWidget::Theme::FlagNeedle)) {
		if (theme().needleType() == VideoWidget::Theme::NeedleTypeBasic)
			needlesize = 2 * (size / 6);
		else if (theme().needleType() == VideoWidget::Theme::NeedleTypeValue)
			needlesize = 2 * (height / 3);
		else
			needlesize = 0;
	}
	else 
		needlesize = 0;

	// Compute position
	p = locate(x, y);

	p.x = p.x + (width / 2) + padding + needlesize + distance;

	if (theme().hasFlag(VideoWidget::Theme::FlagNeedle)) {
		if (theme().needleType() == VideoWidget::Theme::NeedleTypeValue) {
			p.x = p.x - padding;
			p.y = p.y - height - padding;
		}
	}

	// Text offset
	x = -txtx;
	y = -txty;

	// Text position
	x += p.x - (width / 2);
	y += p.y - (height / 2);

	// Draw text
	ShapeBase::text(cr, x, y, font, fill, outline, text);
}


void ChartShape::unit(cairo_t *cr, ChartShape::Font &font,
		const float *fill, const float *outline, const char *text) {
	double x, y;

	int txtx, txty;
	int width, height;

	double padding;

	struct ChartShape::point p;

	// Padding
	padding = theme().hasFlag(VideoWidget::Theme::FlagAll) ? 
		theme().axisBorder() + theme().axisThick() : 0.0;

	// Scaling
	padding = size2pixels(padding);

	// x
	x = scale(x_min_, x_max_, x_min_);
	y = scale(y_min_, y_max_, y_max_);

	// Compute position
	p = locate(x, y);

	// Text dimensions
	this->extents(cr, font, ChartShape::TextAlpha, text, txtx, txty, width, height);

	// Text offset
	x = -txtx;
	y = -txty;

	// Text position
	x += p.x - width - padding;
	y += p.y - (height / 2);

	// Draw text
	ShapeBase::text(cr, x, y, font, fill, outline, text);
}


void ChartShape::needle(cairo_t *cr, VideoWidget::Theme::NeedleType type,
	double x, double y, ChartShape::Font &font, const char *text,
	double border, const float *fill, const float *outline) {
	int txtx, txty;
	int width, height;

	double size;
	double needlesize;
	double distance;

	struct ChartShape::point p;

	double padding;

	// Scaling
	border = size2pixels(theme().needleBorder()) / 2.0;
	distance = size2pixels(theme().needleDistance());

	// Compute icon size
	size = 48;

	// Padding
	padding = border + shadow2pixels(font);

	// Compute position
	p = locate(x, y);

	switch (type) {
	case VideoWidget::Theme::NeedleTypeBasic:
		// Needle size
		needlesize = size / 6;

		// Compute position
		p.x = p.x + distance;

		cairo_save(cr);

		cairo_move_to(cr, p.x, p.y);
		cairo_line_to(cr, p.x + needlesize, p.y - needlesize);
		cairo_line_to(cr, p.x + needlesize, p.y + needlesize);

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
		this->extents(cr, font, ChartShape::TextNumeric, text, txtx, txty, width, height);

		// Needle size
		needlesize = 2 * (height / 3);

		// Compute position
		p.x = p.x + (width / 2) + needlesize + distance;
		p.y = p.y - (height / 2) - needlesize;

		// Draw needle
		cairo_save(cr);

		cairo_move_to(cr, p.x - (width / 2) - padding, p.y - (height / 2) - padding);
		cairo_line_to(cr, p.x + (width / 2) + padding, p.y - (height / 2) - padding);
		cairo_line_to(cr, p.x + (width / 2) + padding, p.y + (height / 2) + padding);
		cairo_line_to(cr, p.x - (width / 2) + needlesize - padding, p.y + (height / 2) + padding);
		cairo_line_to(cr, p.x - (width / 2) - needlesize, p.y + (height / 2) + needlesize);
		cairo_line_to(cr, p.x - (width / 2) - padding, p.y + (height / 2) - needlesize + padding);

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


void ChartShape::icon(cairo_t *cr, double x, double y, const std::string &filename, const float *fill) {
	bool apply_color;

	struct ChartShape::point p;

    GError *error = NULL;

	cairo_t *mask = NULL;
	cairo_surface_t *surface = NULL;

	RsvgRectangle viewport;

	RsvgHandle *handle = NULL;

	double size;

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

	// Compute icon size
	size = size2pixels(theme().iconSize());

	// Compute position
	p = locate(x, y);

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


void ChartShape::curve_create(struct curve **curvep, cairo_t *cr, double width, double border, 
		const float *color, const float *outline, const float *fill) {
	struct curve *curve;

	curve = (struct curve *) malloc(sizeof(struct curve));

	curve->cr = cr;
	curve->width = width;
	curve->border = border;
	curve->color = color;
	curve->outline = outline;
	curve->fill = fill;

	*curvep = curve;
}


void ChartShape::curve_point(struct curve *curve, double x, double y) {
	struct point p;

	p = locate(x, y);

	cairo_line_to(curve->cr, p.x, p.y);
}


void ChartShape::curve_stroke(struct curve *curve) {
	double x, y;

	struct point p1, p2;

	cairo_t *cr = curve->cr;

	const float *color = curve->color;
	const float *outline = curve->outline;
	const float *fill = curve->fill;

	cairo_save(cr);

	// Copy curve path
	cairo_path_t *path = cairo_copy_path(cr);

	// Draw backgroud path
	cairo_set_source_rgba(cr, outline[0], outline[1], outline[2], outline[3]);
	cairo_set_line_width(cr, curve->width + curve->border);
	cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
	cairo_stroke(cr);

	// Fill curve path
	cairo_append_path(curve->cr, path);

	// p1: xmin x 0
	x = scale(x_min_, x_max_, x_min_);
	y = scale(y_min_, y_max_, std::max(y_min_, 0.0));

	p1 = locate(x, y);

	// p2: xmax x 0
	x = scale(x_min_, x_max_, x_max_);
	y = scale(y_min_, y_max_, std::max(y_min_, 0.0));

	p2 = locate(x, y);

	cairo_line_to(cr, p2.x, p2.y);
	cairo_line_to(cr, p1.x, p1.y);
	cairo_close_path(cr);
	cairo_set_source_rgba(cr, fill[0], fill[1], fill[2], fill[3]);
	cairo_fill(cr);
	cairo_stroke(cr);

	// Draw curve path
	cairo_append_path(curve->cr, path);
	cairo_set_source_rgba(cr, color[0], color[1], color[2], color[3]);
	cairo_set_line_width(cr, curve->width);
	cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
	cairo_stroke(cr);
	
	cairo_restore(cr);

	cairo_path_destroy(path);
}


void ChartShape::curve_free(struct curve *curve) {
	free(curve);
}


void ChartShape::xmlwrite(std::ostream &os) {
	log_call();

	ShapeBase::xmlwrite(os);

	os << "<shape>" << VideoWidget::shape2string(VideoWidget::ShapeChart) << "</shape>" << std::endl;

	os << "<with-axis>" << VideoWidget::bool2string(theme().hasFlag(VideoWidget::Theme::FlagAxis)) << "</with-axis>" << std::endl;
	os << "<axis-thick>" << theme().axisThick() << "</axis-thick>" << std::endl;
	os << "<axis-border>" << theme().axisBorder() << "</axis-border>" << std::endl;
	os << "<axis-border-color>" << VideoWidget::Theme::color2hex(theme().axisBorderColor()) << "</axis-border-color>" << std::endl;
	os << "<axis-color>" << VideoWidget::Theme::color2hex(theme().axisColor()) << "</axis-color>" << std::endl;

	os << "<curve-thick>" << theme().curveThick() << "</curve-thick>" << std::endl;
	os << "<curve-border>" << theme().curveBorder() << "</curve-border>" << std::endl;
	os << "<curve-border-color>" << VideoWidget::Theme::color2hex(theme().curveBorderColor()) << "</curve-border-color>" << std::endl;
	os << "<curve-color>" << VideoWidget::Theme::color2hex(theme().curveColor()) << "</curve-color>" << std::endl;
	os << "<curve-fill-color>" << VideoWidget::Theme::color2hex(theme().curveFillColor()) << "</curve-fill-color>" << std::endl;

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

