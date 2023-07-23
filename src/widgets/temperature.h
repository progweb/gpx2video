#ifndef __GPX2VIDEO__WIDGETS__TEMPERATURE_H__
#define __GPX2VIDEO__WIDGETS__TEMPERATURE_H__

#include "log.h"
#include "videowidget.h"


class TemperatureWidget : public VideoWidget {
public:
	virtual ~TemperatureWidget() {
		log_call();

		if (buf_)
			delete buf_;
	}

	static TemperatureWidget * create(GPX2Video &app) {
		TemperatureWidget *widget;

		log_call();

		widget = new TemperatureWidget(app, "temperature");

		widget->setUnit(VideoWidget::UnitCelsius);

		return widget;
	}

	void prepare(OIIO::ImageBuf *buf) {
		bool with_picto = this->hasFlag(VideoWidget::FlagPicto);

		if (buf_ == NULL) {
			this->createBox(&buf_, this->width(), this->height());
			this->drawBorder(buf_);
			this->drawBackground(buf_);
			if (with_picto)
				this->drawImage(buf_, this->border(), this->border(), "./assets/picto/DataOverlay_icn_temperature.png", VideoWidget::ZoomFit);
		}

		// Image over
		buf_->specmod().x = this->x();
		buf_->specmod().y = this->y();
		OIIO::ImageBufAlgo::over(*buf, *buf_, *buf, OIIO::ROI());
	}

	void render(OIIO::ImageBuf *buf, const GPXData &data) {
		int x, y;

		char s[128];

		double temperature = data.temperature();

		bool with_picto = this->hasFlag(VideoWidget::FlagPicto);

		if (unit() == VideoWidget::UnitCelsius) {
		}
		else {
			temperature = (temperature * 9/5) + 32;
		}

		if (data.hasValue(GPXData::DataTemperature))
			sprintf(s, "%.0f %s", temperature, unit2string(unit()).c_str());
		else
			sprintf(s, "-- %s", unit2string(unit()).c_str());

		// Append dynamic info
		x = this->x() + this->padding(VideoWidget::PaddingLeft);
		x += (with_picto) ? this->height() : 0;
		y = this->y();

		this->drawLabel(buf, x, y, label().c_str());
		this->drawValue(buf, x, y, s);
	}

private:
	OIIO::ImageBuf *buf_;

	TemperatureWidget(GPX2Video &app, std::string name)
		: VideoWidget(app, name) 
		, buf_(NULL) {
	}
};

#endif

