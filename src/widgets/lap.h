#ifndef __GPX2VIDEO__WIDGETS__LAP_H__
#define __GPX2VIDEO__WIDGETS__LAP_H__

#include "log.h"
#include "videowidget.h"


class LapWidget : public VideoWidget {
public:
	virtual ~LapWidget() {
		log_call();

		if (bg_buf_)
			delete bg_buf_;

		if (fg_buf_)
			delete fg_buf_;
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


	OIIO::ImageBuf * prepare(void) {
		int x, y;

		bool with_picto = this->hasFlag(VideoWidget::FlagPicto);

		if (bg_buf_ == NULL) {
			x = this->padding(VideoWidget::PaddingLeft);
			x += (with_picto) ? this->height() : 0;
			y = 0;

			this->createBox(&bg_buf_, this->width(), this->height());
			this->drawBorder(bg_buf_);
			this->drawBackground(bg_buf_);
			if (with_picto)
				this->drawImage(bg_buf_, this->border(), this->border(), "./assets/picto/DataOverlay_icn_laps.png", VideoWidget::ZoomFit);
			this->drawLabel(bg_buf_, x, y, label().c_str());
		}

		return bg_buf_;
	}

	OIIO::ImageBuf * render(const GPXData &data) {
		int x, y;

		char s[128];
		int lap = data.lap();

		bool with_picto = this->hasFlag(VideoWidget::FlagPicto);

		// Refresh dynamic info
		if ((fg_buf_ != NULL) && (data.type() == GPXData::TypeUnchanged))
			goto skip;

		// Format data
		if (data.hasValue(GPXData::DataFix))
			sprintf(s, "%d/%d", lap, nbr_target_lap_);
		else
			sprintf(s, "--/--");

		// Compute text position
		x = this->padding(VideoWidget::PaddingLeft);
		x += (with_picto) ? this->height() : 0;
		y = 0;

		// Refresh dynamic info
		if (fg_buf_ != NULL)
			delete fg_buf_;

		this->createBox(&fg_buf_, this->width(), this->height());
		this->drawValue(fg_buf_, x, y, s);

skip:
		return fg_buf_;
	}

private:
	OIIO::ImageBuf *bg_buf_;
	OIIO::ImageBuf *fg_buf_;

	int nbr_target_lap_;

	LapWidget(GPX2Video &app, std::string name)
		: VideoWidget(app, name)
   		, bg_buf_(NULL)
   		, fg_buf_(NULL)
   		, nbr_target_lap_(1) {
	}
};

#endif

