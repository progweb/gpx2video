#ifndef __GPX2VIDEO__WIDGETS__DURATION_H__
#define __GPX2VIDEO__WIDGETS__DURATION_H__

#include "log.h"
#include "videowidget.h"


class DurationWidget : public VideoWidget {
public:
	~DurationWidget() {
		log_call();

		if (buf_)
			delete buf_;
	}

	static DurationWidget * create(GPX2Video &app) {
		DurationWidget *widget;

		log_call();

		widget = new DurationWidget(app, "duration");

		return widget;
	}

	void prepare(void) {
		const int w = 64;

		double divider = (double) (this->height() - (2 * this->border())) / (double) w;

		this->createBox(&buf_, this->width(), this->height());
		this->drawBorder(buf_);
		this->drawBackground(buf_);
		this->drawImage(buf_, this->border(), this->border(), "./assets/picto/DataOverlay_icn_duration.png", divider);
//		this->drawLabel(buf_, 0, 0, label().c_str());
//		this->drawValue(buf_, 0, 0, "22 km");
	}

	void render(OIIO::ImageBuf *buf, const GPXData &data) {
		char s[128];

		int hours;
		int minutes;
		int seconds;

		int duration;

//		struct tm time;

		duration = data.elapsedTime();
		seconds = duration % 60;
		duration = duration / 60;
		minutes = duration % 60;
		hours = duration / 60;

		sprintf(s, "%d:%02d:%02d", hours, minutes, seconds);

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

	DurationWidget(GPX2Video &app, std::string name)
		: VideoWidget(app, name) 
		, buf_(NULL) {
	}
};

#endif

