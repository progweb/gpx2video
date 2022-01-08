
#ifndef __GPX2VIDEO__WIDGETS__HEARTRATE_H__
#define __GPX2VIDEO__WIDGETS__HEARTRATE_H__

#include "videowidget.h"


class HeartRateWidget : public VideoWidget {
public:
	~HeartRateWidget() {
		VideoWidget::~VideoWidget();
	}

	static HeartRateWidget * create(GPX2Video &app) {
		HeartRateWidget *widget;

		log_call();

		widget = new HeartRateWidget(app, "heartrate");

		return widget;
	}

	void draw(OIIO::ImageBuf *buf, const GPXData &data) {
		char s[128];

		const int w = 64;

		double divider = (double) this->height() / (double) w;

		sprintf(s, "%d bpm", data.heartrate());
		this->add(buf, this->x(), this->y(), "./assets/picto/DataOverlay_icn_heartrate.png", "FREQ. CARDIAQUE", s, divider);
	}

private:
	HeartRateWidget(GPX2Video &app, std::string name)
		: VideoWidget(app, name) {
	}
};

#endif

