#include <librsvg/rsvg.h>
#include <pango/pangocairo.h>

#include "log.h"
#include "text.h"


//void TextShape::initialize(void) {
//	bool ok;
//
//	int h;
//
//	int px;
//	int x1, y1, x2, y2;
//	int text_width, text_height;
//
//	double ratio = theme().textRatio();
//
//	int border = theme().border();
//	int padding_yt = theme().padding(VideoWidget::Theme::PaddingTop);
//	int padding_yb = theme().padding(VideoWidget::Theme::PaddingBottom);
//
//	bool with_label = theme().hasFlag(VideoWidget::Theme::FlagLabel);
//	bool with_value = theme().hasFlag(VideoWidget::Theme::FlagValue);
//
//	const char *text = "0123456789/";
//
//	// Compute font size (1 pt = 1.333 px) for "label" text
//	// +-----------------------+  /  no label:
//	// |  Label    px          |  /  +-------------------+
//	// |  Value    ratio * px  |  /  |  Value     px     |
//	// +-----------------------+  /  +-------------------+
//	//        h = px + ratio * px + padding_top + padding_bottom
//	if (with_label && !widget_->label().empty()) {
//		px = theme().height() - 2 * border - padding_yt - theme().textLineSpace() - padding_yb;
//		px = (int) floor((double) px / (1.0 + ratio));
//		// pt = 3 * px / 4;
//
//		for (h=px;; px += 1) {
//			this->textSize(widget_->label().c_str(), px,
//					x1, y1, x2, y2,
//					text_width, text_height);
//
//			if (text_height > h)
//				break;
//
//			label_px_ = px;
//			label_size_ = text_height;
//		}
//	}
//
//	// Compute font size (1 pt = 1.333 px) for "value" text
//	// +-------------------+  /  no label:
//	// |  Label    px      |  /  +-------------------+
//	// |  Value    2 * px  |  /  |  Value     px     |
//	// +-------------------+  /  +-------------------+
//	//        h = px + 2 * px + padding_top + padding_bottom
//	if (with_value) {
//		px = theme().height() - 2 * border - padding_yt - padding_yb;
//		px = with_label ? (int) floor((double) (px - theme().textLineSpace()) * ratio / (1.0 + ratio)) : px;
//		// pt = 3 * px / 4;
//
//		for (h=px;; px += 1) {
//			ok = this->textSize(text, px,
//					x1, y1, x2, y2,
//					text_width, text_height);
//
//			if (!ok) {
//				printf("ERROR: %s %d %d", text, h, px);
//				sleep(2);
//			}
//
//			if (text_height > h)
//				break;
//
//			value_px_ = px;
//			value_size_ = text_height;
//			value_offset_ = y1;
//		}
//	}
//}
//
//
//void TextShape::drawLabel(OIIO::ImageBuf *buf, const char *label) {
//	bool result;
//
//	int x, y;
//
//	int x1, y1, x2, y2;
//	int text_width, text_height;
//
//	int border = theme().border();
//	int padding_xl = theme().padding(VideoWidget::Theme::PaddingLeft);
//	int padding_xr = theme().padding(VideoWidget::Theme::PaddingRight);
//	int padding_yt = theme().padding(VideoWidget::Theme::PaddingTop);
////	int padding_yb = theme().padding(VideoWidget::Theme::PaddingBottom);
//
//	float color[4]; // = { 1.0, 1.0, 1.0, 1.0 };
//
//	bool with_label = theme().hasFlag(VideoWidget::Theme::FlagLabel);
//	bool with_icon = theme().hasFlag(VideoWidget::Theme::FlagIcon);
//
//	enum VideoWidget::Theme::Align textAlign = theme().labelAlign();
//
//	if (!with_label)
//		return;
//
//	// Compute text size
//	this->textSize(label, label_px_,
//			x1, y1, x2, y2,
//			text_width, text_height);
//
//	// Text color
//	memcpy(color, theme().labelColor(), sizeof(color));
//
//	// Text offset
//	x = -x1;
//	y = -y1 + theme().textShadow();
//
//	// Text position
//	if (textAlign == VideoWidget::Theme::AlignLeft) {
//		x += padding_xl;
//		x += (with_icon) ? theme().height() + padding_xl : 0;
//		x += border + theme().textShadow();
//	}
//	else if (textAlign == VideoWidget::Theme::AlignCenter) {
//		x += (with_icon) ? (theme().width() - theme().height())/2 : theme().width()/2;
//		x += (with_icon) ? theme().height() : 0;
//		x -= text_width / 2;
//	}
//	else if (textAlign == VideoWidget::Theme::AlignRight) {
//		x += theme().width() - padding_xr;
//		x -= text_width + border + theme().textShadow();
//	}
//
//	y += border + padding_yt;
//
//	result = OIIO::ImageBufAlgo::render_text_shadow(*buf, 
//		x, 
//		y, 
//		label, 
//		label_px_, theme().font(), color, 
//		OIIO::ImageBufAlgo::TextAlignX::Left, 
//		OIIO::ImageBufAlgo::TextAlignY::Baseline,
//		theme().textShadow());
//
//	if (result == false)
//		fprintf(stderr, "render label text error\n");
//}
//
//
//void TextShape::drawValue(OIIO::ImageBuf *buf, const char *value) {
//	bool result;
//	
//	int x, y;
//
//	int x1, y1, x2, y2;
//	int text_width, text_height;
//
//	int border = theme().border();
//	int padding_xl = theme().padding(VideoWidget::Theme::PaddingLeft);
//	int padding_xr = theme().padding(VideoWidget::Theme::PaddingRight);
//	int padding_yt = theme().padding(VideoWidget::Theme::PaddingTop);
//	int padding_yb = theme().padding(VideoWidget::Theme::PaddingBottom);
//
//	float color[4]; // = { 1.0, 1.0, 1.0, 1.0 };
//
//	bool with_icon = theme().hasFlag(VideoWidget::Theme::FlagIcon);
//	bool with_label = theme().hasFlag(VideoWidget::Theme::FlagLabel);
//	bool with_value = theme().hasFlag(VideoWidget::Theme::FlagValue);
//
//	enum VideoWidget::Theme::Align textAlign = theme().valueAlign();
//
//	if (!with_value)
//		return;
//
//	// Compute text size
//	this->textSize(value, value_px_,
//			x1, y1, x2, y2,
//			text_width, text_height);
//
//	// Text color
//	memcpy(color, theme().valueColor(), sizeof(color));
//
//	// Text offset
//	x = 0;
//	y = -value_offset_ + theme().textShadow();
//
//	// Text position
//	if (textAlign == VideoWidget::Theme::AlignLeft) {
//		x += padding_xl;
//		x += (with_icon) ? theme().height() + padding_xl : 0;
//		x += border + theme().textShadow();
//	}
//	else if (textAlign == VideoWidget::Theme::AlignCenter) {
//		x += (with_icon) ? (theme().width() - theme().height())/2 : theme().width()/2;
//		x += (with_icon) ? theme().height() : 0;
//		x -= text_width / 2;
//	}
//	else if (textAlign == VideoWidget::Theme::AlignRight) {
//		x += theme().width() - padding_xr;
//		x -= text_width + border + theme().textShadow();
//	}
//
//	if (with_label)
//		y += theme().height() - border - theme().textShadow() - padding_yb - value_size_;
//	else
//		y += border + padding_yt;
//
//	result = OIIO::ImageBufAlgo::render_text_shadow(*buf, 
//		x, 
//		y, 
//		value, 
//		value_px_, theme().font(), color, 
//		OIIO::ImageBufAlgo::TextAlignX::Left, 
//		OIIO::ImageBufAlgo::TextAlignY::Baseline, 
//		theme().textShadow());
//
//	if (result == false)
//		fprintf(stderr, "render value text error\n");
//}




void TextShape::icon(cairo_t *cr, const char *filename) {
	RsvgHandle *handle;

	RsvgRectangle viewport = {
		.x = (double) theme().border(),
		.y = (double) theme().border(),
		.width = (double) (size_ - 2 * theme().border()),
		.height = (double) (size_ - 2 * theme().border())
	};

	if ((filename == NULL) || (filename[0] == '\0'))
		return;

	// load svg data
	handle = rsvg_handle_new_from_file(filename, NULL);

	rsvg_handle_render_document(handle, cr, &viewport, NULL);
}


void TextShape::label(cairo_t *cr, TextShape::Font &font, 
		const float *fill, const float *outline, const char *text) {
	int x, y;
	int width, height;

	bool with_icon = theme().hasFlag(VideoWidget::Theme::FlagIcon);

	enum VideoWidget::Theme::Align textAlign = theme().labelAlign();

	this->textSize(cr, font, text, x, y, width, height);

	// Text offset
	x = -x;
	y = -y + font.shadow_distance;

	// Text position
	if (textAlign == VideoWidget::Theme::AlignLeft) {
		x += padding_left_;
		x += (with_icon) ? size_ + padding_left_ : 0;
	}
	else if (textAlign == VideoWidget::Theme::AlignCenter) {
		x += (with_icon) ? (theme().width() - size_)/2 : theme().width()/2;
		x += (with_icon) ? size_ : 0;
		x -= width / 2;
	}
	else if (textAlign == VideoWidget::Theme::AlignRight) {
		x += theme().width() - padding_right_;
		x -= width + font.shadow_distance;
	}

	y += padding_top_;

	this->drawText(cr, x, y, font, fill, outline, text);
}


void TextShape::value(cairo_t *cr, TextShape::Font &font, 
		const float *fill, const float *outline, const char *text) {
	int x, y;
	int width, height;

	bool with_icon = theme().hasFlag(VideoWidget::Theme::FlagIcon);

	enum VideoWidget::Theme::Align textAlign = theme().valueAlign();

	this->textSize(cr, font, text, x, y, width, height);

	// Text offset
	x = -x;
	y = -y + font.shadow_distance;

	// Text position
	if (textAlign == VideoWidget::Theme::AlignLeft) {
		x += padding_left_;
		x += (with_icon) ? size_ + padding_left_ : 0;
	}
	else if (textAlign == VideoWidget::Theme::AlignCenter) {
		x += (with_icon) ? (theme().width() - size_)/2 : theme().width()/2;
		x += (with_icon) ? size_ : 0;
		x -= width / 2;
	}
	else if (textAlign == VideoWidget::Theme::AlignRight) {
		x += theme().width() - padding_right_;
		x -= width + font.shadow_distance;
	}

	y += theme().height() - font.shadow_distance - padding_bottom_ - height;

	this->drawText(cr, x, y, font, fill, outline, text);
}


void TextShape::xmlwrite(std::ostream &os) {
	log_call();

	ShapeBase::xmlwrite(os);

	os << "<with-unit>" << VideoWidget::bool2string(theme().hasFlag(VideoWidget::Theme::FlagUnit)) << "</with-unit>" << std::endl;

//	os << "<text-ratio>" << theme().textRatio() << "</text-ratio>" << std::endl;
//	os << "<text-shadow>" << theme().textShadow() << "</text-shadow>" << std::endl;
//	os << "<text-linespace>" << theme().textLineSpace() << "</text-linespace>" << std::endl;

	os << "<with-icon>" << VideoWidget::bool2string(theme().hasFlag(VideoWidget::Theme::FlagIcon)) << "</with-icon>" << std::endl;
}

