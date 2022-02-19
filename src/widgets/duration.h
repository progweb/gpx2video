#ifndef __GPX2VIDEO__WIDGETS__DURATION_H__
#define __GPX2VIDEO__WIDGETS__DURATION_H__

#include "log.h"
#include "videowidget.h"


class DurationWidget : public VideoWidget {
public:
	~DurationWidget() {
		log_call();
	}

	static DurationWidget * create(GPX2Video &app) {
		DurationWidget *widget;

		log_call();

		widget = new DurationWidget(app, "duration");

		return widget;
	}

	void render(OIIO::ImageBuf *buf, const GPXData &data) {
		char s[128];

		int hours;
		int minutes;
		int seconds;

		int duration;

//		struct tm time;

		const int w = 64;

		double divider = (double) (this->height() - (2 * this->border())) / (double) w;

		duration = data.duration();
		seconds = duration % 60;
		duration = duration / 60;
		minutes = duration % 60;
		hours = duration / 60;

		sprintf(s, "%d:%02d:%02d", hours, minutes, seconds);

		this->fillBackground(buf);
		this->add(buf, this->x(), this->y(), "./assets/picto/DataOverlay_icn_duration.png", 
				label().c_str(), s, divider);
	}

private:
	DurationWidget(GPX2Video &app, std::string name)
		: VideoWidget(app, name) {
	}
};

#endif

