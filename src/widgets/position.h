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
		int x, y;

		bool with_picto = this->hasFlag(VideoWidget::FlagPicto);

		if (buf_ == NULL) {
			x = this->padding(VideoWidget::PaddingLeft);
			x += (with_picto) ? this->height() : 0;
			y = 0;

			this->createBox(&buf_, this->width(), this->height());
			this->drawBorder(buf_);
			this->drawBackground(buf_);
			if (with_picto)
				this->drawImage(buf_, this->border(), this->border(), "./assets/picto/DataOverlay_icn_position.png", VideoWidget::ZoomFit);
			this->drawLabel(buf_, x, y, label().c_str());
		}

		// Image over
		buf_->specmod().x = this->x();
		buf_->specmod().y = this->y();
		OIIO::ImageBufAlgo::over(*buf, *buf_, *buf, OIIO::ROI());
	}

	void render(OIIO::ImageBuf *buf, const GPXData &data) {
		int x, y;

		char s[128];
		struct GPXData::point pt = data.position();

		bool with_picto = this->hasFlag(VideoWidget::FlagPicto);

		if (data.hasValue(GPXData::DataFix))
			sprintf(s, "%.4f, %.4f", pt.lat, pt.lon);
		else
			sprintf(s, "--, --");

		// Append dynamic info
		x = this->x() + this->padding(VideoWidget::PaddingLeft);
		x += (with_picto) ? this->height() : 0;
		y = this->y();

		//this->drawLabel(buf, x, y, label().c_str());
		this->drawValue(buf, x, y, s);
	}

private:
	OIIO::ImageBuf *buf_;

	PositionWidget(GPX2Video &app, std::string name)
		: VideoWidget(app, name)
		, buf_(NULL) {
	}
};

#endif

