#ifndef __GPX2VIDEO__WIDGETS__DISTANCE_H__
#define __GPX2VIDEO__WIDGETS__DISTANCE_H__

#include "log.h"
#include "videowidget.h"


class DistanceWidget : public VideoWidget {
public:
	virtual ~DistanceWidget() {
		log_call();

		if (buf_)
			delete buf_;
	}

	static DistanceWidget * create(GPX2Video &app) {
		DistanceWidget *widget;

		log_call();

		widget = new DistanceWidget(app, "distance");

		widget->setUnit(VideoWidget::UnitMiles);

		return widget;
	}

	void prepare(OIIO::ImageBuf *buf) {
		this->createBox(&buf_, this->width(), this->height());
		this->drawBorder(buf_);
		this->drawBackground(buf_);
		this->drawImage(buf_, this->border(), this->border(), "./assets/picto/DataOverlay_icn_distance.png", VideoWidget::ZoomFit);
//		this->drawLabel(buf_, 0, 0, label().c_str());
//		this->drawValue(buf_, 0, 0, "22 km");

		// Image over
		buf_->specmod().x = this->x();
		buf_->specmod().y = this->y();
		OIIO::ImageBufAlgo::over(*buf, *buf_, *buf, OIIO::ROI());
	}

	void render(OIIO::ImageBuf *buf, const GPXData &data) {
		char s[128];
		double distance = data.distance();

		if (unit() == VideoWidget::UnitKm) {
			distance /= 1000.0;
		}
		else if (unit() == VideoWidget::UnitMeter) {
		}
		else {
			distance /= 1000.0;
			distance *= 0.6213711922;
		}

		sprintf(s, "%.0f %s", distance, unit2string(unit()).c_str());

		// Append dynamic info
		this->drawLabel(buf, this->x() + this->height() + this->padding(), this->y(), label().c_str());
		this->drawValue(buf, this->x() + this->height() + this->padding(), this->y(), s);
	}

private:
	OIIO::ImageBuf *buf_;

	DistanceWidget(GPX2Video &app, std::string name)
		: VideoWidget(app, name)
   		, buf_(NULL) {
	}
};

#endif

