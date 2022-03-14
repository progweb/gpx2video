#ifndef __GPX2VIDEO__WIDGETS__POSITION_H__
#define __GPX2VIDEO__WIDGETS__POSITION_H__

#include "log.h"
#include "videowidget.h"


class PositionWidget : public VideoWidget {
public:
	~PositionWidget() {
		log_call();
	}

	static PositionWidget * create(GPX2Video &app) {
		PositionWidget *widget;

		log_call();

		widget = new PositionWidget(app, "position");

		return widget;
	}

	void render(OIIO::ImageBuf *buf, const GPXData &data) {
		char s[128];
		struct GPXData::point pt = data.position();

		const int w = 64;

		double divider = (double) (this->height() - (2 * this->border())) / (double) w;

		sprintf(s, "%.4f, %.4f", pt.lat, pt.lon);

		this->fillBackground(buf);
		this->add(buf, this->x(), this->y(), "./assets/picto/DataOverlay_icn_position.png", 
				label().c_str(), s, divider);
	}

private:
	PositionWidget(GPX2Video &app, std::string name)
		: VideoWidget(app, name) {
	}
};

#endif

