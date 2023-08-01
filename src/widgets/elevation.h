#ifndef __GPX2VIDEO__WIDGETS__ELEVATION_H__
#define __GPX2VIDEO__WIDGETS__ELEVATION_H__

#include "log.h"
#include "videowidget.h"


class ElevationWidget : public VideoWidget {
public:
	virtual ~ElevationWidget() {
		log_call();

		if (bg_buf_)
			delete bg_buf_;

		if (fg_buf_)
			delete fg_buf_;
	}

	static ElevationWidget * create(GPX2Video &app) {
		ElevationWidget *widget;

		log_call();

		widget = new ElevationWidget(app, "elevation");

		widget->setUnit(VideoWidget::UnitMiles);

		return widget;
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
				this->drawImage(bg_buf_, this->border(), this->border(), "./assets/picto/DataOverlay_icn_elevation.png", VideoWidget::ZoomFit);
			this->drawLabel(bg_buf_, x, y, label().c_str());
		}

		return bg_buf_;
	}

	OIIO::ImageBuf * render(const GPXData &data) {
		int x, y;

		char s[128];
		double elevation = data.elevation();

		bool with_picto = this->hasFlag(VideoWidget::FlagPicto);

		// Refresh dynamic info
		if ((fg_buf_ != NULL) && (data.type() == GPXData::TypeUnchanged))
			goto skip;

		// Format data
		if (unit() == VideoWidget::UnitMeter) {
		}
		else {
			elevation *= 3.28084;
		}

		if (data.hasValue(GPXData::DataElevation))
			sprintf(s, "%.0f %s", elevation, unit2string(unit()).c_str());
		else
			sprintf(s, "-- %s", unit2string(unit()).c_str());

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

	ElevationWidget(GPX2Video &app, std::string name)
		: VideoWidget(app, name) 
   		, bg_buf_(NULL)
   		, fg_buf_(NULL) {
	}
};

#endif

