#ifndef __GPX2VIDEO__WIDGETS__TIME_H__
#define __GPX2VIDEO__WIDGETS__TIME_H__

#include "log.h"
#include "videowidget.h"


class TimeWidget : public VideoWidget {
public:
	virtual ~TimeWidget() {
		log_call();

		if (bg_buf_)
			delete bg_buf_;

		if (fg_buf_)
			delete fg_buf_;
	}

	static TimeWidget * create(GPX2Video &app) {
		TimeWidget *widget;

		log_call();

		widget = new TimeWidget(app, "time");

		return widget;
	}

	OIIO::ImageBuf * prepare(void) {
		bool with_picto = this->hasFlag(VideoWidget::FlagPicto);

		if (bg_buf_ != NULL)
			goto skip;

		this->createBox(&bg_buf_, this->width(), this->height());
		this->drawBorder(bg_buf_);
		this->drawBackground(bg_buf_);
		if (with_picto)
			this->drawImage(bg_buf_, this->border(), this->border(), "./assets/picto/DataOverlay_icn_time.png", VideoWidget::ZoomFit);
		this->drawLabel(bg_buf_, label().c_str());

skip:
		return bg_buf_;
	}

	OIIO::ImageBuf * render(const GPXData &data) {
		char s[128];

		struct tm time;

		(void) data;

		// Refresh dynamic info
		if ((fg_buf_ != NULL) && (data.type() == GPXData::TypeUnchanged))
			goto skip;

		// Format data
		// Don't use gps time, but camera time!
		// Indeed, with garmin devices, gpx time has an offset.
		localtime_r(&app_.time(), &time);

		strftime(s, sizeof(s), "%H:%M:%S", &time);

		// Refresh dynamic info
		if (fg_buf_ != NULL)
			delete fg_buf_;

		this->createBox(&fg_buf_, this->width(), this->height());
		this->drawValue(fg_buf_, s);

skip:
		return fg_buf_;
	}

private:
	OIIO::ImageBuf *bg_buf_;
	OIIO::ImageBuf *fg_buf_;

	TimeWidget(GPX2Video &app, std::string name)
		: VideoWidget(app, name) 
   		, bg_buf_(NULL)
   		, fg_buf_(NULL) {
	}
};

#endif

