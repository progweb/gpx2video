#ifndef __GPX2VIDEO__WIDGETS__SPEED_H__
#define __GPX2VIDEO__WIDGETS__SPEED_H__

#include "log.h"
#include "videowidget.h"


class SpeedWidget : public VideoWidget {
public:
	virtual ~SpeedWidget() {
		log_call();

		if (bg_buf_)
			delete bg_buf_;

		if (fg_buf_)
			delete fg_buf_;
	}

	static SpeedWidget * create(GPX2Video &app) {
		SpeedWidget *widget;

		log_call();

		widget = new SpeedWidget(app, "speed");

		widget->setUnit(VideoWidget::UnitMPH);

		return widget;
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
			this->drawImage(bg_buf_, this->border(), this->border(), "./assets/picto/DataOverlay_icn_speed.png", VideoWidget::ZoomFit);
		this->drawLabel(bg_buf_, label().c_str());

		is_update = true;
skip:
		return bg_buf_;
	}

	OIIO::ImageBuf * render(const GPXData &data, bool &is_update) {
		char s[128];
		double speed = data.speed();

		// Refresh dynamic info
		if ((fg_buf_ != NULL) && (data.type() == GPXData::TypeUnchanged)) {
			is_update = false;
			goto skip;
		}

		// Format data
		if (unit() == VideoWidget::UnitKPH) {
		}
		else {
			speed *= 0.6213711922;
		}

		if (data.hasValue(GPXData::DataFix))
			sprintf(s, "%.0f %s", speed, unit2string(unit()).c_str());
		else
			sprintf(s, "-- %s", unit2string(unit()).c_str());

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

	SpeedWidget(GPX2Video &app, std::string name)
		: VideoWidget(app, name) 
   		, bg_buf_(NULL)
   		, fg_buf_(NULL) {
	}
};

#endif

