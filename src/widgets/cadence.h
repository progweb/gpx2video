
#ifndef __GPX2VIDEO__WIDGETS__CADENCE_H__
#define __GPX2VIDEO__WIDGETS__CADENCE_H__

#include "videowidget.h"


class CadenceWidget : public VideoWidget {
public:
	~CadenceWidget() {
		VideoWidget::~VideoWidget();
	}

	static CadenceWidget * create(GPX2Video &app) {
		CadenceWidget *widget;

		log_call();

		widget = new CadenceWidget(app, "cadence");

		return widget;
	}

	void draw(OIIO::ImageBuf *buf, const GPXData &data) {
		char s[128];

		const int w = 64;

		double divider = (double) this->height() / (double) w;

		sprintf(s, "%d tr/min", data.cadence());
		this->add(buf, this->x(), this->y(), "./assets/picto/DataOverlay_icn_cadence.png", "CADENCE", s, divider);
	}

private:
	CadenceWidget(GPX2Video &app, std::string name)
		: VideoWidget(app, name) {
	}
};

#endif

