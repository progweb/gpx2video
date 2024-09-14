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

	static ImageWidget * create(GPXApplication &app) {
		ImageWidget *widget;

		log_call();

		widget = new ImageWidget(app, "image");

		return widget;
	}

	OIIO::ImageBuf * prepare(bool &is_update) {
		if (bg_buf_ != NULL) {
			is_update = false;
			goto skip;
		}

		this->createBox(&bg_buf_, this->width(), this->height());
		this->drawBorder(bg_buf_);
		this->drawBackground(bg_buf_);
		this->drawImage(bg_buf_, this->border(), this->border(), this->source().c_str(), this->zoom());

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

	ImageWidget(GPXApplication &app, std::string name)
		: VideoWidget(app, name) 
		, bg_buf_(NULL) {
	}
};

#endif

