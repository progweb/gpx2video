#ifndef __GPX2VIDEO__WIDGETS__MAXSPEED_H__
#define __GPX2VIDEO__WIDGETS__MAXSPEED_H__

#include "log.h"
#include "videowidget.h"


class MaxSpeedWidget : public VideoWidget {
public:
	~MaxSpeedWidget() {
		log_call();
	}

	static MaxSpeedWidget * create(GPX2Video &app) {
		MaxSpeedWidget *widget;

		log_call();

		widget = new MaxSpeedWidget(app, "maxspeed");

		widget->setUnits(VideoWidget::UnitMPH);

		return widget;
	}

	void render(OIIO::ImageBuf *buf, const GPXData &data) {
		char s[128];
		double speed = data.maxspeed();

		const int w = 64;

		double divider = (double) this->height() / (double) w;

		if (units() == VideoWidget::UnitKPH) {
		}
		else {
			speed *= 0.6213711922;
		}

		sprintf(s, "%.0f %s", speed, units2string(units()).c_str());

		this->add(buf, this->x(), this->y(), "./assets/picto/DataOverlay_icn_maxspeed.png",
				label().c_str(), s, divider);
	}

private:
	MaxSpeedWidget(GPX2Video &app, std::string name)
		: VideoWidget(app, name) {
	}
};

#endif

