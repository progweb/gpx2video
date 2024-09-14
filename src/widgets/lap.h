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

	static LapWidget * create(GPXApplication &app) {
		LapWidget *widget;

		log_call();

		widget = new LapWidget(app, "lap");

		return widget;
	}

	void setTargetLap(int target) {
		nbr_target_lap_ = target;
	}


	OIIO::ImageBuf * prepare(bool &is_update) {
		bool with_picto = this->hasFlag(VideoWidget::FlagPicto);

		if (bg_buf_ != NULL) {
			is_update = false;
			goto skip;
		}

		this->createBox(&bg_buf_, this->width(), this->height());
		this->drawBorder(bg_buf_);
		this->drawBackground(bg_buf_);
		if (with_picto)
			this->drawImage(bg_buf_, this->border(), this->border(), "./assets/picto/DataOverlay_icn_laps.png", VideoWidget::ZoomFit);
		this->drawLabel(bg_buf_, label().c_str());

		is_update = true;
skip:
		return bg_buf_;
	}

	OIIO::ImageBuf * render(const TelemetryData &data, bool &is_update) {
		char s[128];
		int lap = data.lap();

		// Refresh dynamic info
		if ((fg_buf_ != NULL) && (data.type() == TelemetryData::TypeUnchanged)) {
			is_update = false;
			goto skip;
		}

		// Format data
		if (data.hasValue(TelemetryData::DataFix))
			sprintf(s, "%d/%d", lap, nbr_target_lap_);
		else
			sprintf(s, "--/--");

		// Refresh dynamic info
		if (fg_buf_ != NULL)
			delete fg_buf_;

		this->createBox(&fg_buf_, this->width(), this->height());
		this->drawValue(fg_buf_, s);

		is_update = true;
skip:
		return fg_buf_;
	}

private:
	OIIO::ImageBuf *bg_buf_;
	OIIO::ImageBuf *fg_buf_;

	int nbr_target_lap_;

	LapWidget(GPXApplication &app, std::string name)
		: VideoWidget(app, name)
   		, bg_buf_(NULL)
   		, fg_buf_(NULL)
   		, nbr_target_lap_(1) {
	}
};

#endif

