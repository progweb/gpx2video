#ifndef __GPX2VIDEO__WIDGETS__ELEVATION_H__
#define __GPX2VIDEO__WIDGETS__ELEVATION_H__

#include "log.h"
#include "videowidget.h"


class ElevationWidget : public VideoWidget {
public:
	virtual ~ElevationWidget() {
		log_call();

		if (buf_)
			delete buf_;
	}

	static ElevationWidget * create(GPX2Video &app) {
		ElevationWidget *widget;

		log_call();

		widget = new ElevationWidget(app, "elevation");

		widget->setUnit(VideoWidget::UnitMiles);

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
				this->drawImage(buf_, this->border(), this->border(), "./assets/picto/DataOverlay_icn_elevation.png", VideoWidget::ZoomFit);
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
		double elevation = data.elevation();

		bool with_picto = this->hasFlag(VideoWidget::FlagPicto);

		if (unit() == VideoWidget::UnitMeter) {
		}
		else {
			elevation *= 3.28084;
		}

		if (data.hasValue(GPXData::DataElevation))
			sprintf(s, "%.0f %s", elevation, unit2string(unit()).c_str());
		else
			sprintf(s, "-- %s", unit2string(unit()).c_str());

		// Append dynamic info
		x = this->x() + this->padding(VideoWidget::PaddingLeft);
		x += (with_picto) ? this->height() : 0;
		y = this->y();

		//this->drawLabel(buf, x, y, label().c_str());
		this->drawValue(buf, x, y, s);
	}

private:
	OIIO::ImageBuf *buf_;

	ElevationWidget(GPX2Video &app, std::string name)
		: VideoWidget(app, name) 
		, buf_(NULL) {
	}
};

#endif

