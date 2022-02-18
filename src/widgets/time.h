#ifndef __GPX2VIDEO__WIDGETS__TIME_H__
#define __GPX2VIDEO__WIDGETS__TIME_H__

#include "log.h"
#include "videowidget.h"


class TimeWidget : public VideoWidget {
public:
	~TimeWidget() {
		log_call();
	}

	static TimeWidget * create(GPX2Video &app) {
		TimeWidget *widget;

		log_call();

		widget = new TimeWidget(app, "time");

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

		strftime(s, sizeof(s), "%H:%M:%S", &time);

		this->add(buf, this->x(), this->y(), "./assets/picto/DataOverlay_icn_time.png", "HEURE", s, divider);
	}

private:
	TimeWidget(GPX2Video &app, std::string name)
		: VideoWidget(app, name) {
	}
};

#endif

