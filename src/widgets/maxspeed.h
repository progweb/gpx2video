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

		return widget;
	}

	void render(OIIO::ImageBuf *buf, const GPXData &data) {
		char s[128];

		const int w = 64;

		double divider = (double) this->height() / (double) w;

		sprintf(s, "%.0f km/h", data.maxspeed());
		this->add(buf, this->x(), this->y(), "./assets/picto/DataOverlay_icn_speed.png",
				label().c_str(), s, divider);
	}

private:
	MaxSpeedWidget(GPX2Video &app, std::string name)
		: VideoWidget(app, name) {
	}
};

#endif

