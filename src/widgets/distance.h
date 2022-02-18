#ifndef __GPX2VIDEO__WIDGETS__DISTANCE_H__
#define __GPX2VIDEO__WIDGETS__DISTANCE_H__

#include "log.h"
#include "videowidget.h"


class DistanceWidget : public VideoWidget {
public:
	~DistanceWidget() {
		log_call();
	}

	static DistanceWidget * create(GPX2Video &app) {
		DistanceWidget *widget;

		log_call();

		widget = new DistanceWidget(app, "distance");

		widget->setUnits(VideoWidget::UnitMiles);

		return widget;
	}

	void render(OIIO::ImageBuf *buf, const GPXData &data) {
		char s[128];
		double distance = data.distance();

		const int w = 64;

		double divider = (double) this->height() / (double) w;

		distance /= 1000.0;

		if (units() == VideoWidget::UnitKm) {
		}
		else {
			distance *= 0.6213711922;
		}

		sprintf(s, "%.0f %s", distance, units2string(units()).c_str());

		this->add(buf, this->x(), this->y(), "./assets/picto/DataOverlay_icn_distance.png", 
				label().c_str(), s, divider);
	}

private:
	DistanceWidget(GPX2Video &app, std::string name)
		: VideoWidget(app, name) {
	}
};

#endif

