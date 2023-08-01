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

	static DistanceWidget * create(GPX2Video &app) {
		DistanceWidget *widget;

		log_call();

		widget = new DistanceWidget(app, "distance");

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
				this->drawImage(bg_buf_, this->border(), this->border(), "./assets/picto/DataOverlay_icn_distance.png", VideoWidget::ZoomFit);
			this->drawLabel(bg_buf_, x, y, label().c_str());
		}

		return bg_buf_;
	}

	OIIO::ImageBuf * render(const GPXData &data) {
		int x, y;

		char s[128];
		double distance = data.distance();

		bool with_picto = this->hasFlag(VideoWidget::FlagPicto);

		// Refresh dynamic info
		if ((fg_buf_ != NULL) && (data.type() == GPXData::TypeUnchanged))
			goto skip;

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

		if (data.hasValue(GPXData::DataFix)) {
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

	DistanceWidget(GPX2Video &app, std::string name)
		: VideoWidget(app, name)
   		, bg_buf_(NULL)
   		, fg_buf_(NULL) {
	}
};

#endif

