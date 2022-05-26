#ifndef __GPX2VIDEO__WIDGETS__TIME_H__
#define __GPX2VIDEO__WIDGETS__TIME_H__

#include "log.h"
#include "videowidget.h"


class TimeWidget : public VideoWidget {
public:
	~TimeWidget() {
		log_call();

		if (buf_)
			delete buf_;
	}

	static TimeWidget * create(GPX2Video &app) {
		TimeWidget *widget;

		log_call();

		widget = new TimeWidget(app, "time");

		return widget;
	}

	void prepare(OIIO::ImageBuf *buf) {
		const int w = 64;

		double divider = (double) (this->height() - (2 * this->border())) / (double) w;

		this->createBox(&buf_, this->width(), this->height());
		this->drawBorder(buf_);
		this->drawBackground(buf_);
		this->drawImage(buf_, this->border(), this->border(), "./assets/picto/DataOverlay_icn_time.png", divider);
//		this->drawLabel(buf_, 0, 0, label().c_str());
//		this->drawValue(buf_, 0, 0, "22 km");

		// Image over
		buf_->specmod().x = this->x();
		buf_->specmod().y = this->y();
		OIIO::ImageBufAlgo::over(*buf, *buf_, *buf, OIIO::ROI());
	}

	void render(OIIO::ImageBuf *buf, const GPXData &data) {
		char s[128];

		struct tm time;

		(void) data;

		// Don't use gps time, but camera time!
		// Indeed, with garmin devices, gpx time has an offset.
		localtime_r(&app_.time(), &time);

		strftime(s, sizeof(s), "%H:%M:%S", &time);

		// Append dynamic info
		this->drawLabel(buf, this->x(), this->y(), label().c_str());
		this->drawValue(buf, this->x(), this->y(), s);
	}

private:
	OIIO::ImageBuf *buf_;

	TimeWidget(GPX2Video &app, std::string name)
		: VideoWidget(app, name) 
		, buf_(NULL) {
	}
};

#endif

