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

		widget->setUnits(VideoWidget::UnitMiles);

		return widget;
	}

	void render(OIIO::ImageBuf *buf, const GPXData &data) {
		char s[128];
		double elevation = data.elevation();

		const int w = 64;

		double divider = (double) (this->height() - (2 * this->border())) / (double) w;

		if (units() == VideoWidget::UnitMeter) {
		}
		else {
			elevation *= 0.6213711922;
		}

		sprintf(s, "%.0f %s", elevation, units2string(units()).c_str());

		this->fillBackground(buf);
		this->add(buf, this->x(), this->y(), "./assets/picto/DataOverlay_icn_elevation.png", 
				label().c_str(), s, divider);
	}

private:
	ElevationWidget(GPX2Video &app, std::string name)
		: VideoWidget(app, name) {
	}
};

#endif

