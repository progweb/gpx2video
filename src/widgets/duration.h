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
		bool with_picto = this->hasFlag(VideoWidget::FlagPicto);

		if (buf_ == NULL) {
			this->createBox(&buf_, this->width(), this->height());
			this->drawBorder(buf_);
			this->drawBackground(buf_);
			if (with_picto)
				this->drawImage(buf_, this->border(), this->border(), "./assets/picto/DataOverlay_icn_duration.png", VideoWidget::ZoomFit);
//			this->drawLabel(buf_, 0, 0, label().c_str());
		}

		// Image over
		buf_->specmod().x = this->x();
		buf_->specmod().y = this->y();
		OIIO::ImageBufAlgo::over(*buf, *buf_, *buf, OIIO::ROI());
	}

	void render(OIIO::ImageBuf *buf, const GPXData &data) {
		int x, y;

		char s[128];

		int hours = 0;
		int minutes = 0;
		int seconds = 0;

		int duration;

		bool with_picto = this->hasFlag(VideoWidget::FlagPicto);

		duration = data.elapsedTime();

		if (duration > 0) {
			seconds = duration % 60;
			duration = duration / 60;
			minutes = duration % 60;
			hours = duration / 60;
		}

		if (data.hasValue(GPXData::DataFix))
			sprintf(s, "%d:%02d:%02d", hours, minutes, seconds);
		else
			sprintf(s, "--:--:--");

		// Append dynamic info
		x = this->x() + this->padding(VideoWidget::PaddingLeft);
		x += (with_picto) ? this->height() : 0;
		y = this->y();

		this->drawLabel(buf, x, y, label().c_str());
		this->drawValue(buf, x, y, s);
	}

private:
	OIIO::ImageBuf *buf_;

	DurationWidget(GPX2Video &app, std::string name)
		: VideoWidget(app, name) 
		, buf_(NULL) {
	}
};

#endif

