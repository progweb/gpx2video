#ifndef __GPX2VIDEO__WIDGETS__SPEED_H__
#define __GPX2VIDEO__WIDGETS__SPEED_H__

#include "log.h"
#include "videowidget.h"


class SpeedWidget : public VideoWidget {
public:
	~SpeedWidget() {
		log_call();

		if (buf_)
			delete buf_;
	}

	static SpeedWidget * create(GPX2Video &app) {
		SpeedWidget *widget;

		log_call();

		widget = new SpeedWidget(app, "speed");

		widget->setUnit(VideoWidget::UnitMPH);

		return widget;
	}

	void prepare(OIIO::ImageBuf *buf) {
		this->createBox(&buf_, this->width(), this->height());
		this->drawBorder(buf_);
		this->drawBackground(buf_);
		this->drawImage(buf_, this->border(), this->border(), "./assets/picto/DataOverlay_icn_speed.png", VideoWidget::ZoomFit);
//		this->drawLabel(buf_, 0, 0, label().c_str());
//		this->drawValue(buf_, 0, 0, "22 km");

		// Image over
		buf_->specmod().x = this->x();
		buf_->specmod().y = this->y();
		OIIO::ImageBufAlgo::over(*buf, *buf_, *buf, OIIO::ROI());
	}

	void render(OIIO::ImageBuf *buf, const GPXData &data) {
		char s[128];
		double speed = data.speed();

		if (unit() == VideoWidget::UnitKPH) {
		}
		else {
			speed *= 0.6213711922;
		}

		sprintf(s, "%.0f %s", speed, unit2string(unit()).c_str());

		// Append dynamic info
		this->drawLabel(buf, this->x() + this->height() + this->padding(), this->y(), label().c_str());
		this->drawValue(buf, this->x() + this->height() + this->padding(), this->y(), s);
	}

private:
	OIIO::ImageBuf *buf_;

	SpeedWidget(GPX2Video &app, std::string name)
		: VideoWidget(app, name) 
		, buf_(NULL) {
	}
};

#endif

