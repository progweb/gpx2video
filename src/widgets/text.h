#ifndef __GPX2VIDEO__WIDGETS__TEXT_H__
#define __GPX2VIDEO__WIDGETS__TEXT_H__

#include <pango/pangocairo.h>

#include "../oiio.h"
#include "../shape/base.h"


/**
 * Widget definition
 */

class TextWidget : public VideoWidget, public ShapeBase {
public:
	virtual ~TextWidget() {
	}

	static TextWidget * create(GPXApplication &app) {
		TextWidget *widget;

		widget = new TextWidget(app, "text");

		return widget;
	}

	void setPadding(int left, int right, int top, int bottom) {
		padding_left_ = left;
		padding_right_ = right;
		padding_top_ = top;
		padding_bottom_ = bottom;
	}

	OIIO::ImageBuf * prepare(bool &is_update) {
		if (bg_buf_ != NULL) {
			is_update = false;
			goto skip;
		}

		this->initialize();
		this->createBox(&bg_buf_, theme().width(), theme().height());
		this->drawBorder(bg_buf_);
		this->drawBackground(bg_buf_);

		is_update = true;
skip:
		return bg_buf_;
//		int x, y;
//
//		int px;
//		int x1, y1, x2, y2;
//		int text_width, text_height;
//
//		int border = theme().border();
////		int padding_x = theme().padding(VideoWidget::Theme::PaddingLeft);
//		int padding_yt = theme().padding(VideoWidget::Theme::PaddingTop);
//		int padding_yb = theme().padding(VideoWidget::Theme::PaddingBottom);
//
//		if (bg_buf_ != NULL)
//			goto skip;
//
//		// Compute font size (1 pt = 1.333 px)
//		// +-------------
//		// |  Text    px
//		// +-------------
//		//        h = px + padding_top + padding_bottom
//		px = theme().height() - 2 * border - padding_yt - padding_yb;
//		// pt = 3 * px / 4;
//
//		// Create overlay buffer
//		this->createBox(&bg_buf_, theme().width(), theme().height());
//		this->drawBorder(bg_buf_);
//		this->drawBackground(bg_buf_);
//
//		this->textSize(this->text().c_str(), px,
//				x1, y1, x2, y2,
//				text_width, text_height);
//
//		// Text offset
//		x = -x1;
//		y = 0; //-y1;
//
//		// Text position
//		x += border + (theme().width() - text_width) / 2;
//		y += border + (theme().height() - text_height) / 2;
//
//		this->drawText(bg_buf_, x, y, px, this->text().c_str());
//
//		is_update = true;
//skip:
//		return bg_buf_;
//
//		is_update = false;
//
//		return NULL;
	}

	OIIO::ImageBuf * render(const TelemetryData &data, bool &is_update) {
//		(void) data;
//
//		is_update = false;
//
//		return NULL;
		cairo_t *cairo;

		if (fg_buf_ != NULL) {
			is_update = false;
			goto skip;
		}

		// Image buffer
		this->createBox(&fg_buf_, theme().width(), theme().height());

		// Cairo context
		cairo = this->createCairoContext(fg_buf_);

		// Draw
		draw(cairo, data);

		// Data bytes
		this->renderCairoContext(fg_buf_, cairo);

		// Release
		this->destroyCairoContext(cairo);

		is_update = true;
skip:
		return fg_buf_;
	}

	void clear(void) {
		if (bg_buf_)
			delete bg_buf_;

		if (fg_buf_)
			delete fg_buf_;

		bg_buf_ = NULL;
		fg_buf_ = NULL;
	}

	bool isStatic(void) {
		return true;
	}

protected:
	void xmlwrite(std::ostream &os) {
		VideoWidget::xmlwrite(os);

		IndentingOStreambuf indent(os, 4);

		ShapeBase::xmlwrite(os);

		os << "<text>" << text() << "</text>" << std::endl;
	}

private:
	OIIO::ImageBuf *bg_buf_;
	OIIO::ImageBuf *fg_buf_;

	int padding_left_;
	int padding_right_;
	int padding_top_;
	int padding_bottom_;

	TextWidget(GPXApplication &app, std::string name)
		: VideoWidget(app, name) 
		, ShapeBase(theme())
		, bg_buf_(NULL)
		, fg_buf_(NULL) {
	}


	void initialize(void);
	void draw(cairo_t *cr, const TelemetryData &data);
	void label(cairo_t *cr, ShapeBase::Font &font, 
			const float *fill, const float *outline, const char *text);
	void value(cairo_t *cr, ShapeBase::Font &font, 
			const float *fill, const float *outline, const char *text);
};

#endif

