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

		widget->setUnits(VideoWidget::UnitMPH);

		return widget;
	}

	void prepare(void) {
		const int w = 64;

		double divider = (double) (this->height() - (2 * this->border())) / (double) w;

		this->createBox(&buf_, this->width(), this->height());
		this->drawBorder(buf_);
		this->drawBackground(buf_);
		this->drawImage(buf_, this->border(), this->border(), "./assets/picto/DataOverlay_icn_speed.png", divider);
//		this->drawLabel(buf_, 0, 0, label().c_str());
//		this->drawValue(buf_, 0, 0, "22 km");
	}

	void render(OIIO::ImageBuf *buf, const GPXData &data) {
		char s[128];
		double speed = data.speed();

		if (units() == VideoWidget::UnitKPH) {
		}
		else {
			speed *= 0.6213711922;
		}

		sprintf(s, "%.0f %s", speed, units2string(units()).c_str());

		// Image over
		buf_->specmod().x = this->x();
		buf_->specmod().y = this->y();
		OIIO::ImageBufAlgo::over(*buf, *buf_, *buf, OIIO::ROI());

		// Append dynamic info
		this->drawLabel(buf, this->x(), this->y(), label().c_str());
		this->drawValue(buf, this->x(), this->y(), s);
	}

private:
	OIIO::ImageBuf *buf_;

	SpeedWidget(GPX2Video &app, std::string name)
		: VideoWidget(app, name) 
		, buf_(NULL) {
	}
};

#endif

