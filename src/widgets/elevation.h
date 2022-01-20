#ifndef __GPX2VIDEO__WIDGETS__ELEVATION_H__
#define __GPX2VIDEO__WIDGETS__ELEVATION_H__

#include "log.h"
#include "videowidget.h"


class ElevationWidget : public VideoWidget {
public:
	~ElevationWidget() {
		log_call();
	}

	static ElevationWidget * create(GPX2Video &app) {
		ElevationWidget *widget;

		log_call();

		widget = new ElevationWidget(app, "elevation");

		return widget;
	}

	void render(OIIO::ImageBuf *buf, const GPXData &data) {
		char s[128];

		const int w = 64;

		double divider = (double) this->height() / (double) w;

		sprintf(s, "%.0f m", data.elevation());
		this->add(buf, this->x(), this->y(), "./assets/picto/DataOverlay_icn_elevation.png", 
				label().c_str(), s, divider);
	}

private:
	ElevationWidget(GPX2Video &app, std::string name)
		: VideoWidget(app, name) {
	}
};

#endif

