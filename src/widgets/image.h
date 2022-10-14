#ifndef __GPX2VIDEO__WIDGETS__IMAGE_H__
#define __GPX2VIDEO__WIDGETS__IMAGE_H__

#include "log.h"
#include "videowidget.h"


class ImageWidget : public VideoWidget {
public:
	virtual ~ImageWidget() {
		log_call();

		if (buf_)
			delete buf_;
	}

	static ImageWidget * create(GPX2Video &app) {
		ImageWidget *widget;

		log_call();

		widget = new ImageWidget(app, "image");

		return widget;
	}

	void prepare(OIIO::ImageBuf *buf) {
		if (buf_ == NULL) {
			this->createBox(&buf_, this->width(), this->height());
			this->drawBorder(buf_);
			this->drawBackground(buf_);
			this->drawImage(buf_, this->border(), this->border(), this->source().c_str(), this->zoom());
		}

		// Image over
		buf_->specmod().x = this->x();
		buf_->specmod().y = this->y();
		OIIO::ImageBufAlgo::over(*buf, *buf_, *buf, OIIO::ROI());
	}

	void render(OIIO::ImageBuf *buf, const GPXData &data) {
		(void) data;
		(void) buf;
	}

private:
	OIIO::ImageBuf *buf_;

	ImageWidget(GPX2Video &app, std::string name)
		: VideoWidget(app, name) 
		, buf_(NULL) {
	}
};

#endif

