#ifndef __GPX2VIDEO__WIDGETS__HEARTRATE_H__
#define __GPX2VIDEO__WIDGETS__HEARTRATE_H__

#include "log.h"
#include "videowidget.h"


class HeartRateWidget : public VideoWidget {
public:
	virtual ~HeartRateWidget() {
		log_call();

		if (bg_buf_)
			delete bg_buf_;

		if (fg_buf_)
			delete fg_buf_;
	}

	static HeartRateWidget * create(GPX2Video &app) {
		HeartRateWidget *widget;

		log_call();

		widget = new HeartRateWidget(app, "heartrate");

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
			this->drawImage(bg_buf_, this->border(), this->border(), "./assets/picto/DataOverlay_icn_heartrate.png", VideoWidget::ZoomFit);
		this->drawLabel(bg_buf_, label().c_str());

skip:
		return bg_buf_;
	}

	OIIO::ImageBuf * render(const GPXData &data, bool &is_update) {
		char s[128];

		// Refresh dynamic info
		if ((fg_buf_ != NULL) && (data.type() == GPXData::TypeUnchanged)) {
			is_update = false;
			goto skip;
		}

		// Format data
		if (data.hasValue(GPXData::DataHeartrate))
			sprintf(s, "%d bpm", data.heartrate());
		else
			sprintf(s, "-- bpm");

		// Refresh dynamic info
		if (fg_buf_ != NULL)
			delete fg_buf_;

		this->createBox(&fg_buf_, this->width(), this->height());
		this->drawValue(fg_buf_, s);

		is_update = true;
skip:
		return fg_buf_;
	}

private:
	OIIO::ImageBuf *bg_buf_;
	OIIO::ImageBuf *fg_buf_;

	HeartRateWidget(GPX2Video &app, std::string name)
		: VideoWidget(app, name) 
   		, bg_buf_(NULL)
   		, fg_buf_(NULL) {
	}
};

#endif

