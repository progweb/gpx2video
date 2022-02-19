#ifndef __GPX2VIDEO__WIDGETS__SPEED_H__
#define __GPX2VIDEO__WIDGETS__SPEED_H__

#include "log.h"
#include "videowidget.h"


class SpeedWidget : public VideoWidget {
public:
	~SpeedWidget() {
		log_call();
	}

	static SpeedWidget * create(GPX2Video &app) {
		SpeedWidget *widget;

		log_call();

		widget = new SpeedWidget(app, "speed");

		widget->setUnits(VideoWidget::UnitMPH);

		return widget;
	}

	void render(OIIO::ImageBuf *buf, const GPXData &data) {
		char s[128];
		double speed = data.speed();

		const int w = 64;

		double divider = (double) (this->height() - (2 * this->border())) / (double) w;

		if (units() == VideoWidget::UnitKPH) {
		}
		else {
			speed *= 0.6213711922;
		}

		sprintf(s, "%.0f %s", speed, units2string(units()).c_str());

		this->fillBackground(buf);
		this->add(buf, this->x(), this->y(), "./assets/picto/DataOverlay_icn_speed.png",
				label().c_str(), s, divider);
	}

private:
	SpeedWidget(GPX2Video &app, std::string name)
		: VideoWidget(app, name) {
	}
};

#endif

