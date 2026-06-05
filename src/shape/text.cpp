#include <librsvg/rsvg.h>
#include <pango/pangocairo.h>

#include "../log_i.h"
#include "text.h"


void TextShape::icon(cairo_t *cr, const std::string &filename, const float *fill) {
	RsvgHandle *handle;

	RsvgRectangle viewport = {
		.x = (double) theme().border(),
		.y = (double) theme().border(),
		.width = (double) (size_ - 2 * theme().border()),
		.height = (double) (size_ - 2 * theme().border())
	};

//	if ((filename == NULL) || (filename[0] == '\0'))
//		return;
	if (filename.empty())
		return;

	// load svg data
	handle = rsvg_handle_new_from_file(filename.c_str(), NULL);

	// Create alpha only surface
	cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_A8, size_, size_);

	// Create mask
	cairo_t *mask = cairo_create(surface);

//	rsvg_handle_render_document(handle, cr, &viewport, NULL);
	rsvg_handle_render_document(handle, mask, &viewport, NULL);

	cairo_destroy(mask);

	// Apply color
	cairo_set_source_rgba(cr, fill[0], fill[1], fill[2], fill[3]);

	// Paint mask
	cairo_mask_surface(cr, surface, 0, 0);

	cairo_surface_destroy(surface);

	g_object_unref(handle);
}


void TextShape::label(cairo_t *cr, TextShape::Font &font, 
		const float *fill, const float *outline, const char *text) {
	int x, y;
//	int width, height;

	int linespace = theme().lineSpace();

	bool with_icon = theme().hasFlag(VideoWidget::Theme::FlagIcon);

	enum VideoWidget::Theme::Align halign = theme().labelHorizontalAlign();
	enum VideoWidget::Theme::Align valign = theme().labelVerticalAlign();

//	this->extents(cr, font, text, x, y, width, height);

	// Text offset
	x = -label_x_; //-x;
	y = -label_y_; //-y;

	// Text horizontal position
	if (halign == VideoWidget::Theme::AlignLeft) {
		x += padding_left_;
		x += (with_icon) ? size_ + padding_left_ : 0;
	}
	else if (halign == VideoWidget::Theme::AlignCenter) {
		x += (with_icon) ? (theme().width() - size_)/2 : theme().width()/2;
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

	ShapeBase::text(cr, x, y, font, fill, outline, text);
}


void TextShape::value(cairo_t *cr, TextShape::Font &font, 
		const float *fill, const float *outline, const char *text) {
	int x, y;
	int width, height;

	int linespace = theme().lineSpace();

	bool with_icon = theme().hasFlag(VideoWidget::Theme::FlagIcon);

	enum VideoWidget::Theme::Align halign = theme().valueHorizontalAlign();
	enum VideoWidget::Theme::Align valign = theme().valueVerticalAlign();

	this->extents(cr, font, text, x, y, width, height);

	// Text offset
	x = -x; //-x;
	y = -value_y_; //-y;

	// Text horizontal position
	if (halign == VideoWidget::Theme::AlignLeft) {
		x += padding_left_;
		x += (with_icon) ? size_ + padding_left_ : 0;
	}
	else if (halign == VideoWidget::Theme::AlignCenter) {
		x += (with_icon) ? (theme().width() - size_)/2 : theme().width()/2;
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

	ShapeBase::text(cr, x, y, font, fill, outline, text);
}


void TextShape::xmlwrite(std::ostream &os) {
	log_call();

	ShapeBase::xmlwrite(os);

	os << "<shape>" << VideoWidget::shape2string(VideoWidget::ShapeText) << "</shape>" << std::endl;
	os << "<icon-color>" << VideoWidget::Theme::color2hex(theme_.iconColor()) << "</icon-color>" << std::endl;
	os << "<line-space>" << theme_.lineSpace() << "</line-space>" << std::endl;

	os << "<with-icon>" << VideoWidget::bool2string(theme().hasFlag(VideoWidget::Theme::FlagIcon)) << "</with-icon>" << std::endl;
}

