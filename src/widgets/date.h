#ifndef __GPX2VIDEO__WIDGETS__DATE_H__
#define __GPX2VIDEO__WIDGETS__DATE_H__

#include "log.h"
#include "videowidget.h"


class DateWidget : public VideoWidget {
public:
	virtual ~DateWidget() {
		log_call();

		clear();
	}

	static DateWidget * create(GPXApplication &app) {
		DateWidget *widget;

		log_call();

		widget = new DateWidget(app, "date");

		widget->setFormat("%Y-%m-%d");

		return widget;
	}

	OIIO::ImageBuf * prepare(bool &is_update) {
		bool with_picto = this->hasFlag(VideoWidget::FlagPicto);

		if (bg_buf_ != NULL) {
			is_update = false;
			goto skip;
		}

		// Create overlay buffer
		this->createBox(&bg_buf_, this->width(), this->height());
		this->drawBorder(bg_buf_);
		this->drawBackground(bg_buf_);
		if (with_picto)
			this->drawImage(bg_buf_, this->border(), this->border(), "./assets/picto/DataOverlay_icn_date.png", VideoWidget::ZoomFit);
		this->drawLabel(bg_buf_, label().c_str());

		is_update = true;
skip:
		return bg_buf_;
	}

	OIIO::ImageBuf * render(const TelemetryData &data, bool &is_update) {
		char s[128];

		time_t t;
		struct tm time;

		(void) data;

		// Compute time
		t = app_.time() / 1000;

		// Refresh dynamic info
		if (fg_buf_ != NULL) {
			if (t == last_time_) {
				is_update = false;
				goto skip;
			}
		}

		// Format data
		// Don't use gps time, but camera time!
		// Indeed, with garmin devices, gpx time has an offset.
		localtime_r(&t, &time);

		strftime(s, sizeof(s), format().c_str(), &time);

		// Refresh dynamic info
		if (fg_buf_ != NULL)
			delete fg_buf_;

		this->createBox(&fg_buf_, this->width(), this->height());
		this->drawValue(fg_buf_, s);

		is_update = true;
		last_time_ = t;

skip:
		return fg_buf_;
	}

	void clear(void) {
		if (bg_buf_)
			delete bg_buf_;

		if (fg_buf_)
			delete fg_buf_;

		bg_buf_ = NULL;
		fg_buf_ = NULL;
	}

private:
	time_t last_time_;

	OIIO::ImageBuf *bg_buf_;
	OIIO::ImageBuf *fg_buf_;

	DateWidget(GPXApplication &app, std::string name)
		: VideoWidget(app, name) 
   		, bg_buf_(NULL)
   		, fg_buf_(NULL) {
		last_time_ = 0;
	}
};

#endif

