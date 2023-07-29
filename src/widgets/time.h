#ifndef __GPX2VIDEO__WIDGETS__TIME_H__
#define __GPX2VIDEO__WIDGETS__TIME_H__

#include "log.h"
#include "videowidget.h"


class TimeWidget : public VideoWidget {
public:
	virtual ~TimeWidget() {
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
				this->drawImage(buf_, this->border(), this->border(), "./assets/picto/DataOverlay_icn_time.png", VideoWidget::ZoomFit);
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

		struct tm time;

		bool with_picto = this->hasFlag(VideoWidget::FlagPicto);

		(void) data;

		// Don't use gps time, but camera time!
		// Indeed, with garmin devices, gpx time has an offset.
		localtime_r(&app_.time(), &time);

		strftime(s, sizeof(s), "%H:%M:%S", &time);

		// Append dynamic info
		x = this->x() + this->padding(VideoWidget::PaddingLeft);
		x += (with_picto) ? this->height() : 0;
		y = this->y();

		//this->drawLabel(buf, x, y, label().c_str());
		this->drawValue(buf, x, y, s);
	}

private:
	OIIO::ImageBuf *buf_;

	TimeWidget(GPX2Video &app, std::string name)
		: VideoWidget(app, name) 
		, buf_(NULL) {
	}
};

#endif

