#ifndef __GPX2VIDEO__WIDGETS__LAP_H__
#define __GPX2VIDEO__WIDGETS__LAP_H__

#include "log.h"
#include "videowidget.h"


class LapWidget : public VideoWidget {
public:
	virtual ~LapWidget() {
		log_call();

		if (buf_)
			delete buf_;
	}

	static LapWidget * create(GPX2Video &app) {
		LapWidget *widget;

		log_call();

		widget = new LapWidget(app, "lap");

		return widget;
	}

	void setTargetLap(int target) {
		nbr_target_lap_ = target;
	}


	void prepare(OIIO::ImageBuf *buf) {
		bool with_picto = this->hasFlag(VideoWidget::FlagPicto);

		if (buf_ == NULL) {
			this->createBox(&buf_, this->width(), this->height());
			this->drawBorder(buf_);
			this->drawBackground(buf_);
			if (with_picto)
				this->drawImage(buf_, this->border(), this->border(), "./assets/picto/DataOverlay_icn_laps.png", VideoWidget::ZoomFit);
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
		int lap = data.lap();

		bool with_picto = this->hasFlag(VideoWidget::FlagPicto);

		if (data.hasValue(GPXData::DataFix))
			sprintf(s, "%d/%d", lap, nbr_target_lap_);
		else
			sprintf(s, "--/--");

		// Append dynamic info
		x = this->x() + this->padding(VideoWidget::PaddingLeft);
		x += (with_picto) ? this->height() : 0;
		y = this->y();

		this->drawLabel(buf, x, y, label().c_str());
		this->drawValue(buf, x, y, s);
	}

private:
	OIIO::ImageBuf *buf_;

	int nbr_target_lap_;

	LapWidget(GPX2Video &app, std::string name)
		: VideoWidget(app, name)
   		, buf_(NULL)
   		, nbr_target_lap_(1) {
	}
};

#endif

