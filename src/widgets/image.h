#ifndef __GPX2VIDEO__WIDGETS__IMAGE_H__
#define __GPX2VIDEO__WIDGETS__IMAGE_H__

#include "log.h"
#include "videowidget.h"


class ImageWidget : public VideoWidget {
public:
	virtual ~ImageWidget() {
		log_call();

		if (bg_buf_)
			delete bg_buf_;
	}

	static ImageWidget * create(GPX2Video &app) {
		ImageWidget *widget;

		log_call();

		widget = new ImageWidget(app, "image");

		return widget;
	}

	OIIO::ImageBuf * prepare(void) {
		if (bg_buf_ != NULL)
			goto skip;

		this->createBox(&bg_buf_, this->width(), this->height());
		this->drawBorder(bg_buf_);
		this->drawBackground(bg_buf_);
		this->drawImage(bg_buf_, this->border(), this->border(), this->source().c_str(), this->zoom());

skip:
		return bg_buf_;
	}

	OIIO::ImageBuf * render(const GPXData &data, bool &is_update) {
		(void) data;

		is_update = false;

		return NULL;
	}

private:
	OIIO::ImageBuf *bg_buf_;

	ImageWidget(GPX2Video &app, std::string name)
		: VideoWidget(app, name) 
		, bg_buf_(NULL) {
	}
};

#endif

