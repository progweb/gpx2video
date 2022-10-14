#ifndef __GPX2VIDEO__WIDGETS__POSITION_H__
#define __GPX2VIDEO__WIDGETS__POSITION_H__

#include "log.h"
#include "videowidget.h"


class PositionWidget : public VideoWidget {
public:
	virtual ~PositionWidget() {
		log_call();

		if (buf_)
			delete buf_;
	}

	static PositionWidget * create(GPX2Video &app) {
		PositionWidget *widget;

		log_call();

		widget = new PositionWidget(app, "position");

		return widget;
	}

	void prepare(OIIO::ImageBuf *buf) {
		if (buf_ == NULL) {
			this->createBox(&buf_, this->width(), this->height());
			this->drawBorder(buf_);
			this->drawBackground(buf_);
			this->drawImage(buf_, this->border(), this->border(), "./assets/picto/DataOverlay_icn_position.png", VideoWidget::ZoomFit);
//			this->drawLabel(buf_, 0, 0, label().c_str());
		}

		// Image over
		buf_->specmod().x = this->x();
		buf_->specmod().y = this->y();
		OIIO::ImageBufAlgo::over(*buf, *buf_, *buf, OIIO::ROI());
	}

	void render(OIIO::ImageBuf *buf, const GPXData &data) {
		char s[128];
		struct GPXData::point pt = data.position();

		if (data.hasValue(GPXData::TypeFix))
			sprintf(s, "%.4f, %.4f", pt.lat, pt.lon);
		else
			sprintf(s, "--, --");

		// Append dynamic info
		this->drawLabel(buf, this->x() + this->height() + this->padding(VideoWidget::PaddingLeft), this->y(), label().c_str());
		this->drawValue(buf, this->x() + this->height() + this->padding(VideoWidget::PaddingLeft), this->y(), s);
	}

private:
	OIIO::ImageBuf *buf_;

	PositionWidget(GPX2Video &app, std::string name)
		: VideoWidget(app, name)
		, buf_(NULL) {
	}
};

#endif

