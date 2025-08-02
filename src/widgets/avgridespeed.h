#ifndef __GPX2VIDEO__WIDGETS__AVGRIDESPEED_H__
#define __GPX2VIDEO__WIDGETS__AVGRIDESPEED_H__

#include "utils.h"
#include "videowidget.h"


class AvgRideSpeedWidget : public VideoWidget {
public:
	virtual ~AvgRideSpeedWidget() {
		clear();
	}

	static AvgRideSpeedWidget * create(GPXApplication &app) {
		AvgRideSpeedWidget *widget;

		widget = new AvgRideSpeedWidget(app, "avgridespeed");

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
			this->drawImage(bg_buf_, this->border(), this->border(), "./assets/picto/DataOverlay_icn_avgspeed.png", VideoWidget::ZoomFit);
		this->drawLabel(bg_buf_, label().c_str());

		is_update = true;
skip:
		return bg_buf_;
	}

	OIIO::ImageBuf * render(const TelemetryData &data, bool &is_update) {
		char s[128];
		double speed = data.avgridespeed();
		
		// Refresh dynamic info
		if (fg_buf_ != NULL) {
			if (data.type() == TelemetryData::TypeUnchanged) {
				is_update = false;
				goto skip;
			}

			if (no_value_ && !data.hasValue(TelemetryData::DataAverageRideSpeed)) {
				is_update = false;
				goto skip;
			}
		}

		// Format data
		no_value_ = !data.hasValue(TelemetryData::DataAverageRideSpeed);

		if (unit() == VideoWidget::UnitKPH) {
		}
		else {
			speed *= 0.6213711922;
		}

		if (!no_value_)
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

	void clear(void) {
		if (bg_buf_)
			delete bg_buf_;

		if (fg_buf_)
			delete fg_buf_;

		bg_buf_ = NULL;
		fg_buf_ = NULL;
	}

protected:
	void xmlwrite(std::ostream &os) {
		VideoWidget::xmlwrite(os);

		IndentingOStreambuf indent(os, 4);

		os << "<unit>" << unit2string(unit(), false) << "</unit>" << std::endl;
		os << "<text-shadow>" << textShadow() << "</text-shadow>" << std::endl;
	}

private:
	bool no_value_;

	OIIO::ImageBuf *bg_buf_;
	OIIO::ImageBuf *fg_buf_;

	AvgRideSpeedWidget(GPXApplication &app, std::string name)
		: VideoWidget(app, name) 
   		, bg_buf_(NULL)
   		, fg_buf_(NULL) {
		no_value_ = false;
	}
};

#endif

