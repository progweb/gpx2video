#ifndef __GPX2VIDEO__WIDGETS__DURATION_H__
#define __GPX2VIDEO__WIDGETS__DURATION_H__

#include "log.h"
#include "videowidget.h"


class DurationWidget : public VideoWidget {
public:
	virtual ~DurationWidget() {
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

	void prepare(OIIO::ImageBuf *buf) {
		if (buf_ == NULL) {
			this->createBox(&buf_, this->width(), this->height());
			this->drawBorder(buf_);
			this->drawBackground(buf_);
			this->drawImage(buf_, this->border(), this->border(), "./assets/picto/DataOverlay_icn_duration.png", VideoWidget::ZoomFit);
//			this->drawLabel(buf_, 0, 0, label().c_str());
		}

		// Image over
		buf_->specmod().x = this->x();
		buf_->specmod().y = this->y();
		OIIO::ImageBufAlgo::over(*buf, *buf_, *buf, OIIO::ROI());
	}

	void render(OIIO::ImageBuf *buf, const GPXData &data) {
		char s[128];

		int hours = 0;
		int minutes = 0;
		int seconds = 0;

		int duration;

//		struct tm time;

		duration = data.elapsedTime();

		if (duration > 0) {
			seconds = duration % 60;
			duration = duration / 60;
			minutes = duration % 60;
			hours = duration / 60;
		}

		if (data.hasValue(GPXData::TypeFix))
			sprintf(s, "%d:%02d:%02d", hours, minutes, seconds);
		else
			sprintf(s, "--:--:--");

		// Append dynamic info
		this->drawLabel(buf, this->x() + this->height() + this->padding(VideoWidget::PaddingLeft), this->y(), label().c_str());
		this->drawValue(buf, this->x() + this->height() + this->padding(VideoWidget::PaddingLeft), this->y(), s);
	}

private:
	OIIO::ImageBuf *buf_;

	DurationWidget(GPX2Video &app, std::string name)
		: VideoWidget(app, name) 
		, buf_(NULL) {
	}
};

#endif

