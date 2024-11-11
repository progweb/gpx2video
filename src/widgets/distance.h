#ifndef __GPX2VIDEO__WIDGETS__DISTANCE_H__
#define __GPX2VIDEO__WIDGETS__DISTANCE_H__

#include "log.h"
#include "videowidget.h"


class DistanceWidget : public VideoWidget {
public:
	virtual ~DistanceWidget() {
		log_call();

		if (bg_buf_)
			delete bg_buf_;

		if (fg_buf_)
			delete fg_buf_;
	}

	static DistanceWidget * create(GPXApplication &app) {
		DistanceWidget *widget;

		log_call();

		widget = new DistanceWidget(app, "distance");

		widget->setUnit(VideoWidget::UnitMiles);

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
			this->drawImage(bg_buf_, this->border(), this->border(), "./assets/picto/DataOverlay_icn_distance.png", VideoWidget::ZoomFit);
		this->drawLabel(bg_buf_, label().c_str());

		is_update = true;
skip:
		return bg_buf_;
	}

	OIIO::ImageBuf * render(const TelemetryData &data, bool &is_update) {
		char s[128];
		double distance = data.distance();

		// Refresh dynamic info
		if ((fg_buf_ != NULL) && (data.type() == TelemetryData::TypeUnchanged)) {
			is_update = false;
			goto skip;
		}

		// Format data
		if (unit() == VideoWidget::UnitKm) {
			distance /= 1000.0;
		}
		else if (unit() == VideoWidget::UnitMeter) {
		}
		else if (unit() == VideoWidget::UnitFoot) {
			distance *= 3.28084;
		}
		else {
			distance /= 1000.0;
			distance *= 0.6213711922;
		}

		if (data.hasValue(TelemetryData::DataDistance)) {
			const char *format;

			if (distance < 10)
				format = "%.2f %s";
			else if (distance < 100)
				format = "%.1f %s";
			else
				format = "%.0f %s";

			sprintf(s, format, distance, unit2string(unit()).c_str());
		}
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

	DistanceWidget(GPXApplication &app, std::string name)
		: VideoWidget(app, name)
   		, bg_buf_(NULL)
   		, fg_buf_(NULL) {
	}
};

#endif

