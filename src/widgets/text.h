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

	static TextWidget * create(GPXApplication &app) {
		TextWidget *widget;

		log_call();

		widget = new TextWidget(app, "text");

		return widget;
	}

	OIIO::ImageBuf * prepare(bool &is_update) {
		int x, y;

		int px;
		int x1, y1, x2, y2;
		int text_width, text_height;

		int border = this->border();
//		int padding_x = this->padding(VideoWidget::PaddingLeft);
		int padding_yt = this->padding(VideoWidget::PaddingTop);
		int padding_yb = this->padding(VideoWidget::PaddingBottom);

		if (bg_buf_ != NULL)
			goto skip;

		// Compute font size (1 pt = 1.333 px)
		// +-------------
		// |  Text    px
		// +-------------
		//        h = px + padding_top + padding_bottom
		px = this->height() - 2 * border - padding_yt - padding_yb;
		// pt = 3 * px / 4;

		// Create overlay buffer
		this->createBox(&bg_buf_, this->width(), this->height());
		this->drawBorder(bg_buf_);
		this->drawBackground(bg_buf_);

		this->textSize(this->text().c_str(), px,
				x1, y1, x2, y2,
				text_width, text_height);

		// Text offset
		x = -x1;
		y = -y1;

		// Text position
		x += border + (this->width() - text_width) / 2;
		y += border + (this->height() - text_height) / 2;

		this->drawText(bg_buf_, x, y, px, this->text().c_str());

		is_update = true;
skip:
		return bg_buf_;
	}

	OIIO::ImageBuf * render(const TelemetryData &data, bool &is_update) {
		(void) data;

		is_update = false;

		return NULL;
	}


private:
	OIIO::ImageBuf *bg_buf_;

	TextWidget(GPXApplication &app, std::string name)
		: VideoWidget(app, name) 
   		, bg_buf_(NULL) {
	}
};

#endif

