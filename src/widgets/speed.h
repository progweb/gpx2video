
#ifndef __GPX2VIDEO__WIDGETS__SPEED_H__
#define __GPX2VIDEO__WIDGETS__SPEED_H__

#include "videowidget.h"


class SpeedWidget : public VideoWidget {
public:
	~SpeedWidget() {
		VideoWidget::~VideoWidget();
	}

	static SpeedWidget * create(GPX2Video &app) {
		SpeedWidget *widget;

		log_call();

		widget = new SpeedWidget(app, "speed");

		return widget;
	}

	void draw(OIIO::ImageBuf *buf, const GPXData &data) {
		char s[128];

		const int w = 64;

		double divider = (double) this->height() / (double) w;

		sprintf(s, "%.0f km/h", data.speed());
		this->add(buf, this->x(), this->y(), "./assets/picto/DataOverlay_icn_speed.png", "VITESSE", s, divider);
	}

private:
	SpeedWidget(GPX2Video &app, std::string name)
		: VideoWidget(app, name) {
	}
};

#endif

