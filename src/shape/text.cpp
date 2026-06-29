#include <librsvg/rsvg.h>
#include <pango/pangocairo.h>

#include "../log_i.h"
#include "text.h"


void TextShape::icon(cairo_t *cr, const std::string &filename, const float *fill) {
	bool apply_color;

    GError *error = NULL;

	cairo_t *mask = NULL;
	cairo_surface_t *surface = NULL;

	double size;
	double padding_left, padding_top;

	RsvgRectangle viewport;

	RsvgHandle *handle = NULL;

	if (filename.empty())
		return;

	// Set color
	apply_color = (fill != NULL) && (fill[3] != 0);

	// Compute icon size
	size = ((double) (size_ - 2 * theme().border()) * theme().iconSize()) / 100.0;

	// Padding
	padding_left = (size_ - size) / 2.0;
	padding_top = (theme().textOrientation() == VideoWidget::OrientationHorizontal) ? padding_left : 0.0;

	// load svg data
	handle = rsvg_handle_new_from_file(filename.c_str(), &error);
    if (!handle) {
        log_error("Load svn image failure: %s", error->message);
        g_error_free(error);
        return;
    }

	if (apply_color) {
		// Create alpha only surface
		surface = cairo_image_surface_create(CAIRO_FORMAT_A8, size_, size_);

		// Create mask
		mask = cairo_create(surface);
	}

	// Render svg into cairo surface
	viewport = (RsvgRectangle) {
		.x = (double) theme().border() + padding_left,
		.y = (double) theme().border() + padding_top,
		.width = size,
		.height = size
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


void TextShape::label(cairo_t *cr, TextShape::Font &font, 
		const float *fill, const float *outline, const char *text) {
	int x, y;

	double linespace = theme().lineSpace();

	bool with_icon = theme().hasFlag(VideoWidget::Theme::FlagIcon);

	enum VideoWidget::Theme::Align halign = theme().labelHorizontalAlign();
	enum VideoWidget::Theme::Align valign = theme().labelVerticalAlign();

	// Scaling
	linespace = size2pixels(linespace);

	// Text offset
	x = -label_x_;
	y = -label_y_;

	if (theme().textOrientation() == VideoWidget::OrientationHorizontal) {
		// Text horizontal position
		if (halign == VideoWidget::Theme::AlignLeft) {
			x += padding_left_;
			x += (with_icon) ? size_ + padding_left_ : 0;
		}
		else if (halign == VideoWidget::Theme::AlignCenter) {
			x += (with_icon) ? (theme().width() - size_) / 2 : theme().width() / 2;
			x += (with_icon) ? size_ : 0;
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

		// Value offset 
		if (theme().hasFlag(VideoWidget::Theme::FlagValue)) {
			if (valign == theme().valueVerticalAlign()) {
				if (valign == VideoWidget::Theme::AlignTop) {
				}
				else if (valign == VideoWidget::Theme::AlignCenter)
					y -= (linespace + value_height_) / 2;
				else if (valign == VideoWidget::Theme::AlignBottom)
					y -= linespace + value_height_;
			}
		}
	}
	else {
		// Compute icon size
		double size = with_icon ? ((double) (size_ - 2 * theme().border()) * theme().iconSize()) / 100.0 : 0.0;

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
			y += padding_top_ + size;
		}
		else if (valign == VideoWidget::Theme::AlignCenter) {
			y += theme().height() / 2 + padding_top_ - padding_bottom_;
			y -= label_height_ / 2;
		}
		else if (valign == VideoWidget::Theme::AlignBottom) {
			y += theme().height() - padding_bottom_;
			y -= label_height_;
		}

		// Value offset 
		if (theme().hasFlag(VideoWidget::Theme::FlagValue)) {
			if (valign == theme().valueVerticalAlign()) {
				if (valign == VideoWidget::Theme::AlignTop) {
				}
				else if (valign == VideoWidget::Theme::AlignCenter)
					y -= (linespace + value_height_) / 2;
				else if (valign == VideoWidget::Theme::AlignBottom)
					y -= linespace + value_height_;
			}
		}

		// Unit offset 
		if (theme().hasFlag(VideoWidget::Theme::FlagUnit)) {
			if (valign == theme().valueVerticalAlign()) {
				if (valign == VideoWidget::Theme::AlignTop) {
				}
				else if (valign == VideoWidget::Theme::AlignCenter)
					y -= (linespace + unit_height_) / 2;
				else if (valign == VideoWidget::Theme::AlignBottom)
					y -= linespace + unit_height_;
			}
		}
	}

	ShapeBase::text(cr, x, y, font, fill, outline, text);
}


void TextShape::value(cairo_t *cr, TextShape::Font &font, 
		const float *fill, const float *outline, const char *text) {
	int x, y;
	int width, height;

	double distance = theme().unitDistance();
	double linespace = theme().lineSpace();

	bool with_icon = theme().hasFlag(VideoWidget::Theme::FlagIcon);

	enum VideoWidget::Theme::Align halign = theme().valueHorizontalAlign();
	enum VideoWidget::Theme::Align valign = theme().valueVerticalAlign();

	this->extents(cr, font, text, x, y, width, height);

	setValueExtents(x, value_y_, width, value_height_);

	// Scaling
	distance = size2pixels(distance);
	linespace = size2pixels(linespace);

	// Text offset
	x = -x;
	y = -value_y_;

	if (theme().textOrientation() == VideoWidget::OrientationHorizontal) {
		// Text horizontal position
		if (halign == VideoWidget::Theme::AlignLeft) {
			x += padding_left_;
			x += (with_icon) ? size_ + padding_left_ : 0;
		}
		else if (halign == VideoWidget::Theme::AlignCenter) {
			x += (with_icon) ? (theme().width() - size_) / 2 : theme().width() / 2;
			x += (with_icon) ? size_ : 0;
			x -= width / 2;
		}
		else if (halign == VideoWidget::Theme::AlignRight) {
			x += theme().width() - padding_right_;
			x -= width;
		}

		// Text vertical position
		if (valign == VideoWidget::Theme::AlignTop) {
			y += padding_top_;
		}
		else if (valign == VideoWidget::Theme::AlignCenter) {
			y += theme().height() / 2 + padding_top_ - padding_bottom_;
			y -= value_height_ / 2;
		}
		else if (valign == VideoWidget::Theme::AlignBottom) {
			y += theme().height() - padding_bottom_;
			y -= value_height_;
		}

		// Label offset 
		if (theme().hasFlag(VideoWidget::Theme::FlagLabel)) {
			if (valign == theme().labelVerticalAlign()) {
				if (valign == VideoWidget::Theme::AlignTop)
					y += linespace + label_height_;
				else if (valign == VideoWidget::Theme::AlignCenter)
					y += (linespace + label_height_) / 2;
				else if (valign == VideoWidget::Theme::AlignBottom) {
				}
			}
		}

		// Unit offset 
		if (theme().hasFlag(VideoWidget::Theme::FlagUnit)) {
			if (halign == VideoWidget::Theme::AlignLeft) {
			}
			else if (halign == VideoWidget::Theme::AlignLeft)
				x -= (distance + unit_width_) / 2;
			else if (halign == VideoWidget::Theme::AlignRight)
				x -= distance + unit_width_;
		}
	}
	else {
		// Compute icon size
		double size = with_icon ? ((double) (size_ - 2 * theme().border()) * theme().iconSize()) / 100.0 : 0.0;

		// Text horizontal position
		if (halign == VideoWidget::Theme::AlignLeft) {
			x += padding_left_;
		}
		else if (halign == VideoWidget::Theme::AlignCenter) {
			x += theme().width() / 2;
			x -= width / 2;
		}
		else if (halign == VideoWidget::Theme::AlignRight) {
			x += theme().width() - padding_right_;
			x -= width;
		}

		// Text vertical position
		if (valign == VideoWidget::Theme::AlignTop) {
			y += padding_top_ + size;
		}
		else if (valign == VideoWidget::Theme::AlignCenter) {
			y += theme().height() / 2 + padding_top_ - padding_bottom_;
			y -= value_height_ / 2;
		}
		else if (valign == VideoWidget::Theme::AlignBottom) {
			y += theme().height() - padding_bottom_;
			y -= value_height_;
		}

		// Label offset 
		if (theme().hasFlag(VideoWidget::Theme::FlagLabel)) {
			if (valign == theme().labelVerticalAlign()) {
				if (valign == VideoWidget::Theme::AlignTop)
					y += linespace + label_height_;
				else if (valign == VideoWidget::Theme::AlignCenter)
					y += (linespace + label_height_) / 2;
				else if (valign == VideoWidget::Theme::AlignBottom) {
				}
			}
		}

		// Unit offset 
		if (theme().hasFlag(VideoWidget::Theme::FlagUnit)) {
			if (valign == VideoWidget::Theme::AlignTop) {
			}
			else if (valign == VideoWidget::Theme::AlignCenter)
				y -= (linespace + unit_height_) / 2;
			else if (valign == VideoWidget::Theme::AlignBottom)
				y -= linespace + unit_height_;
		}
	}

	ShapeBase::text(cr, x, y, font, fill, outline, text);
}


void TextShape::unit(cairo_t *cr, TextShape::Font &font, 
		const float *fill, const float *outline, const char *text) {
	int x, y;

	double distance = theme().unitDistance();
	double linespace = theme().lineSpace();

	bool with_icon = theme().hasFlag(VideoWidget::Theme::FlagIcon);

	enum VideoWidget::Theme::Align halign = theme().valueHorizontalAlign();
	enum VideoWidget::Theme::Align valign = theme().valueVerticalAlign();

	// Scaling
	distance = size2pixels(distance);
	linespace = size2pixels(linespace);

	// Text offset
	x = -unit_x_;
	y = -unit_y_;

	if (theme().textOrientation() == VideoWidget::OrientationHorizontal) {
		// Text horizontal position
		if (halign == VideoWidget::Theme::AlignLeft) {
			x += padding_left_;
			x += (with_icon) ? size_ + padding_left_ : 0;
		}
		else if (halign == VideoWidget::Theme::AlignCenter) {
			x += (with_icon) ? (theme().width() - size_) / 2 : theme().width() / 2;
			x += (with_icon) ? size_ : 0;
			x -= unit_width_ / 2;
		}
		else if (halign == VideoWidget::Theme::AlignRight) {
			x += theme().width() - padding_right_;
			x -= unit_width_;
		}

		// Text vertical position
		if (valign == VideoWidget::Theme::AlignTop) {
			y += padding_top_;
			y += value_height_;
			y -= unit_height_;
		}
		else if (valign == VideoWidget::Theme::AlignCenter) {
			y += theme().height() / 2 + padding_top_ - padding_bottom_;
			y += value_height_ / 2;
			y -= unit_height_;
		}
		else if (valign == VideoWidget::Theme::AlignBottom) {
			y += theme().height() - padding_bottom_;
			y -= unit_height_;
		}

		// Label offset 
		if (theme().hasFlag(VideoWidget::Theme::FlagLabel)) {
			if (valign == theme().labelVerticalAlign()) {
				if (valign == VideoWidget::Theme::AlignTop)
					y += linespace + label_height_;
				else if (valign == VideoWidget::Theme::AlignCenter)
					y += (linespace + label_height_) / 2;
				else if (valign == VideoWidget::Theme::AlignBottom) {
				}
			}
		}

		// Value offset 
		if (theme().hasFlag(VideoWidget::Theme::FlagValue)) {
			if (halign == VideoWidget::Theme::AlignLeft)
				x += value_width_ + distance;
			else if (halign == VideoWidget::Theme::AlignCenter)
				x += (value_width_ + distance) / 2;
			else if (halign == VideoWidget::Theme::AlignRight) {
			}
		}
	}
	else {
		// Compute icon size
		double size = with_icon ? ((double) (size_ - 2 * theme().border()) * theme().iconSize()) / 100.0 : 0.0;

		// Text horizontal position
		if (halign == VideoWidget::Theme::AlignLeft) {
			x += padding_left_;
		}
		else if (halign == VideoWidget::Theme::AlignCenter) {
			x += theme().width() / 2;
			x -= unit_width_ / 2;
		}
		else if (halign == VideoWidget::Theme::AlignRight) {
			x += theme().width() - padding_right_;
			x -= unit_width_;
		}

		// Text vertical position
		if (valign == VideoWidget::Theme::AlignTop) {
			y += padding_top_ + size;
		}
		else if (valign == VideoWidget::Theme::AlignCenter) {
			y += theme().height() / 2 + padding_top_ - padding_bottom_;
			y -= value_height_ / 2;
		}
		else if (valign == VideoWidget::Theme::AlignBottom) {
			y += theme().height() - padding_bottom_;
			y -= value_height_;
		}

		// Label offset 
		if (theme().hasFlag(VideoWidget::Theme::FlagLabel)) {
			if (valign == theme().labelVerticalAlign()) {
				if (valign == VideoWidget::Theme::AlignTop)
					y += linespace + label_height_;
				else if (valign == VideoWidget::Theme::AlignCenter)
					y += (linespace + label_height_) / 2;
				else if (valign == VideoWidget::Theme::AlignBottom) {
				}
			}
		}

		// Value offset 
		if (theme().hasFlag(VideoWidget::Theme::FlagValue)) {
			if (valign == VideoWidget::Theme::AlignTop)
				y += linespace + value_height_;
			else if (valign == VideoWidget::Theme::AlignCenter)
				y += (linespace + value_height_) / 2;
			else if (valign == VideoWidget::Theme::AlignBottom) {
			}
		}
	}

	ShapeBase::text(cr, x, y, font, fill, outline, text);
}


void TextShape::xmlwrite(std::ostream &os) {
	log_call();

	ShapeBase::xmlwrite(os);

	os << "<shape>" << VideoWidget::shape2string(VideoWidget::ShapeText) << "</shape>" << std::endl;

	os << "<line-space>" << theme_.lineSpace() << "</line-space>" << std::endl;
	os << "<text-orientation>" << VideoWidget::orientation2string(theme().textOrientation()) << "</text-orientation>" << std::endl;

	os << "<with-icon>" << VideoWidget::bool2string(theme().hasFlag(VideoWidget::Theme::FlagIcon)) << "</with-icon>" << std::endl;
	os << "<icon-name>" << VideoWidget::icon2string(theme_.icon()) << theme_.iconFile() << "</icon-name>" << std::endl;
	os << "<icon-color>" << VideoWidget::Theme::color2hex(theme_.iconColor()) << "</icon-color>" << std::endl;
}

