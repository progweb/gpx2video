#ifndef __GPX2VIDEO__WIDGETS__TEXT_H__
#define __GPX2VIDEO__WIDGETS__TEXT_H__

#include "log.h"
#include "oiio.h"
#include "videowidget.h"


class TextWidget : public VideoWidget {
public:
	virtual ~TextWidget() {
		log_call();

		if (bg_buf_)
			delete bg_buf_;
	}

	static TextWidget * create(GPX2Video &app) {
		TextWidget *widget;

		log_call();

		widget = new TextWidget(app, "text");

		return widget;
	}

	OIIO::ImageBuf * prepare(void) {
		int h;

		int space_x, space_y;
		int border = this->border();
		int padding_x = this->padding(VideoWidget::PaddingLeft);
		int padding_yt = this->padding(VideoWidget::PaddingTop);
		int padding_yb = this->padding(VideoWidget::PaddingBottom);

		// width x height
		h = this->height() - 2 * border;

		// Add text (1 pt = 1.333 px)
		// +-------------
		// |  Text    px
		// +-------------
		//        h = px + padding_top + padding_bottom
		int px = h - padding_yt - padding_yb;
		int pt = 3 * px / 4;

		space_x = padding_x + border;
		space_y = padding_yt + border;

		if (bg_buf_ == NULL) {
			this->createBox(&bg_buf_, this->width(), this->height());
			this->drawBorder(bg_buf_);
			this->drawBackground(bg_buf_);

			this->drawText(bg_buf_, space_x, space_y, pt, this->text().c_str());
		}

		return bg_buf_;
	}

	OIIO::ImageBuf * render(const GPXData &data) {
		(void) data;

		return NULL;
	}


private:
	OIIO::ImageBuf *bg_buf_;

	TextWidget(GPX2Video &app, std::string name)
		: VideoWidget(app, name) 
   		, bg_buf_(NULL) {
	}
};

#endif

