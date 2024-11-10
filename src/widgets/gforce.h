#ifndef __GPX2VIDEO__WIDGETS__GFORCE_H__
#define __GPX2VIDEO__WIDGETS__GFORCE_H__

#include "log.h"
#include "videowidget.h"


class GForceWidget : public VideoWidget {
public:
	virtual ~GForceWidget() {
		log_call();

		if (bg_buf_)
			delete bg_buf_;

		if (fg_buf_)
			delete fg_buf_;
	}

	static GForceWidget * create(GPXApplication &app) {
		GForceWidget *widget;

		log_call();

		widget = new GForceWidget(app, "g-force");

		widget->setUnit(VideoWidget::UnitG);

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
			this->drawImage(bg_buf_, this->border(), this->border(), "./assets/picto/DataOverlay_icn_gforce.png", VideoWidget::ZoomFit);
		this->drawLabel(bg_buf_, label().c_str());

		is_update = true;
skip:
		return bg_buf_;
	}

	OIIO::ImageBuf * render(const TelemetryData &data, bool &is_update) {
		char s[128];
		double gforce;

		// Refresh dynamic info
		if ((fg_buf_ != NULL) && (data.type() == TelemetryData::TypeUnchanged)) {
			is_update = false;
			goto skip;
		}

		// Compute data
		// acceleration m/s²
		// gforce g (= 9,80665 m/s²)
		gforce = data.acceleration();
	   
		// Format data
		if (unit() == VideoWidget::UnitG)
			gforce /= 9.81;

		if (data.hasValue(TelemetryData::DataFix))
			sprintf(s, "%.2f %s", gforce, unit2string(unit()).c_str());
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

	GForceWidget(GPXApplication &app, std::string name)
		: VideoWidget(app, name) 
   		, bg_buf_(NULL)
   		, fg_buf_(NULL) {
	}
};

#endif

