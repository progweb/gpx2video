#ifndef __GPX2VIDEO__WIDGETS__DATE_H__
#define __GPX2VIDEO__WIDGETS__DATE_H__

#include "log.h"
#include "videowidget.h"


class DateWidget : public VideoWidget {
public:
	~DateWidget() {
		log_call();
	}

	static DateWidget * create(GPX2Video &app) {
		DateWidget *widget;

		log_call();

		widget = new DateWidget(app, "date");

		widget->setFormat("%Y-%m-%d");

		return widget;
	}

	void render(OIIO::ImageBuf *buf, const GPXData &data) {
		char s[128];

		struct tm time;

		const int w = 64;

		(void) data;

		double divider = (double) this->height() / (double) w;

		// Don't use gps time, but camera time!
		// Indeed, with garmin devices, gpx time has an offset.
		localtime_r(&app_.time(), &time);

		strftime(s, sizeof(s), format().c_str(), &time);

		this->add(buf, this->x(), this->y(), "./assets/picto/DataOverlay_icn_date.png", 
				label().c_str(), s, divider);
	}

private:
	DateWidget(GPX2Video &app, std::string name)
		: VideoWidget(app, name) {
	}
};

#endif

