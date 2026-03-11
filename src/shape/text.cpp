#include "log.h"
#include "text.h"


void TextShape::initialize(void) {
	bool ok;

	int h;

	int px;
	int x1, y1, x2, y2;
	int text_width, text_height;

	double ratio = widget_->textRatio();

	int border = widget_->border();
	int padding_yt = widget_->padding(VideoWidget::PaddingTop);
	int padding_yb = widget_->padding(VideoWidget::PaddingBottom);

	bool with_label = widget_->hasFlag(VideoWidget::FlagLabel);
	bool with_value = widget_->hasFlag(VideoWidget::FlagValue);

	const char *text = "0123456789/";

	// Compute font size (1 pt = 1.333 px) for "label" text
	// +-----------------------+  /  no label:
	// |  Label    px          |  /  +-------------------+
	// |  Value    ratio * px  |  /  |  Value     px     |
	// +-----------------------+  /  +-------------------+
	//        h = px + ratio * px + padding_top + padding_bottom
	if (with_label && !widget_->label().empty()) {
		px = widget_->height() - 2 * border - padding_yt - widget_->textLineSpace() - padding_yb;
		px = (int) floor((double) px / (1.0 + ratio));
		// pt = 3 * px / 4;

		for (h=px;; px += 1) {
			this->textSize(widget_->label().c_str(), px,
					x1, y1, x2, y2,
					text_width, text_height);

			if (text_height > h)
				break;

			label_px_ = px;
			label_size_ = text_height;
		}
	}

	// Compute font size (1 pt = 1.333 px) for "value" text
	// +-------------------+  /  no label:
	// |  Label    px      |  /  +-------------------+
	// |  Value    2 * px  |  /  |  Value     px     |
	// +-------------------+  /  +-------------------+
	//        h = px + 2 * px + padding_top + padding_bottom
	if (with_value) {
		px = widget_->height() - 2 * border - padding_yt - padding_yb;
		px = with_label ? (int) floor((double) (px - widget_->textLineSpace()) * ratio / (1.0 + ratio)) : px;
		// pt = 3 * px / 4;

		for (h=px;; px += 1) {
			ok = this->textSize(text, px,
					x1, y1, x2, y2,
					text_width, text_height);

			if (!ok) {
				printf("ERROR: %s %d %d", text, h, px);
				sleep(2);
			}

			if (text_height > h)
				break;

			value_px_ = px;
			value_size_ = text_height;
			value_offset_ = y1;
		}
	}
}


void TextShape::drawLabel(OIIO::ImageBuf *buf, const char *label) {
	bool result;

	int x, y;

	int x1, y1, x2, y2;
	int text_width, text_height;

	int border = widget_->border();
	int padding_xl = widget_->padding(VideoWidget::PaddingLeft);
	int padding_xr = widget_->padding(VideoWidget::PaddingRight);
	int padding_yt = widget_->padding(VideoWidget::PaddingTop);
//	int padding_yb = widget_->padding(VideoWidget::PaddingBottom);

	float color[4]; // = { 1.0, 1.0, 1.0, 1.0 };

	bool with_label = widget_->hasFlag(VideoWidget::FlagLabel);
	bool with_picto = widget_->hasFlag(VideoWidget::FlagPicto);

	enum VideoWidget::TextAlign textAlign = widget_->labelAlign();

	if (!with_label)
		return;

	// Compute text size
	this->textSize(label, label_px_,
			x1, y1, x2, y2,
			text_width, text_height);

	// Text color
	memcpy(color, widget_->textColor(), sizeof(color));

	// Text offset
	x = -x1;
	y = -y1 + widget_->textShadow();

	// Text position
	if (textAlign == VideoWidget::TextAlignLeft) {
		x += padding_xl;
		x += (with_picto) ? widget_->height() + padding_xl : 0;
		x += border + widget_->textShadow();
	}
	else if (textAlign == VideoWidget::TextAlignCenter) {
		x += (with_picto) ? (widget_->width() - widget_->height())/2 : widget_->width()/2;
		x += (with_picto) ? widget_->height() : 0;
		x -= text_width / 2;
	}
	else if (textAlign == VideoWidget::TextAlignRight) {
		x += widget_->width() - padding_xr;
		x -= text_width + border + widget_->textShadow();
	}

	y += border + padding_yt;

	result = OIIO::ImageBufAlgo::render_text_shadow(*buf, 
		x, 
		y, 
		label, 
		label_px_, widget_->font(), color, 
		OIIO::ImageBufAlgo::TextAlignX::Left, 
		OIIO::ImageBufAlgo::TextAlignY::Baseline,
		widget_->textShadow());

	if (result == false)
		fprintf(stderr, "render label text error\n");
}


void TextShape::drawValue(OIIO::ImageBuf *buf, const char *value) {
	bool result;
	
	int x, y;

	int x1, y1, x2, y2;
	int text_width, text_height;

	int border = widget_->border();
	int padding_xl = widget_->padding(VideoWidget::PaddingLeft);
	int padding_xr = widget_->padding(VideoWidget::PaddingRight);
	int padding_yt = widget_->padding(VideoWidget::PaddingTop);
	int padding_yb = widget_->padding(VideoWidget::PaddingBottom);

	float color[4]; // = { 1.0, 1.0, 1.0, 1.0 };

	bool with_picto = widget_->hasFlag(VideoWidget::FlagPicto);
	bool with_label = widget_->hasFlag(VideoWidget::FlagLabel);
	bool with_value = widget_->hasFlag(VideoWidget::FlagValue);

	enum VideoWidget::TextAlign textAlign = widget_->valueAlign();

	if (!with_value)
		return;

	// Compute text size
	this->textSize(value, value_px_,
			x1, y1, x2, y2,
			text_width, text_height);

	// Text color
	memcpy(color, widget_->textColor(), sizeof(color));

	// Text offset
	x = 0;
	y = -value_offset_ + widget_->textShadow();

	// Text position
	if (textAlign == VideoWidget::TextAlignLeft) {
		x += padding_xl;
		x += (with_picto) ? widget_->height() + padding_xl : 0;
		x += border + widget_->textShadow();
	}
	else if (textAlign == VideoWidget::TextAlignCenter) {
		x += (with_picto) ? (widget_->width() - widget_->height())/2 : widget_->width()/2;
		x += (with_picto) ? widget_->height() : 0;
		x -= text_width / 2;
	}
	else if (textAlign == VideoWidget::TextAlignRight) {
		x += widget_->width() - padding_xr;
		x -= text_width + border + widget_->textShadow();
	}

	if (with_label)
		y += widget_->height() - border - widget_->textShadow() - padding_yb - value_size_;
	else
		y += border + padding_yt;

	result = OIIO::ImageBufAlgo::render_text_shadow(*buf, 
		x, 
		y, 
		value, 
		value_px_, widget_->font(), color, 
		OIIO::ImageBufAlgo::TextAlignX::Left, 
		OIIO::ImageBufAlgo::TextAlignY::Baseline, 
		widget_->textShadow());

	if (result == false)
		fprintf(stderr, "render value text error\n");
}

