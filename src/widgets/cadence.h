#ifndef __GPX2VIDEO__WIDGETS__CADENCE_H__
#define __GPX2VIDEO__WIDGETS__CADENCE_H__

#include "log.h"
#include "videowidget.h"


class CadenceWidget : public VideoWidget {
public:
	virtual ~CadenceWidget() {
		log_call();

		if (bg_buf_)
			delete bg_buf_;

		if (fg_buf_)
			delete fg_buf_;
	}

	static CadenceWidget * create(GPX2Video &app) {
		CadenceWidget *widget;

		log_call();

		widget = new CadenceWidget(app, "cadence");

		return widget;
	}

	OIIO::ImageBuf * prepare(void) {
		int x, y;

		bool with_picto = this->hasFlag(VideoWidget::FlagPicto);

		if (bg_buf_ == NULL) {
			x = this->padding(VideoWidget::PaddingLeft);
			x += (with_picto) ? this->height() : 0;
			y = 0;

			this->createBox(&bg_buf_, this->width(), this->height());
			this->drawBorder(bg_buf_);
			this->drawBackground(bg_buf_);
			if (with_picto)
				this->drawImage(bg_buf_, this->border(), this->border(), "./assets/picto/DataOverlay_icn_cadence.png", VideoWidget::ZoomFit);
			this->drawLabel(bg_buf_, x, y, label().c_str());
		}

		return bg_buf_;
	}

	OIIO::ImageBuf * render(const GPXData &data) {
		int x, y;

		char s[128];

		bool with_picto = this->hasFlag(VideoWidget::FlagPicto);

		// Refresh dynamic info
		if ((fg_buf_ != NULL) && (data.type() == GPXData::TypeUnchanged))
			goto skip;

		// Format data
		if (data.hasValue(GPXData::DataCadence))
			sprintf(s, "%d tr/min", data.cadence());
		else
			sprintf(s, "-- tr/min");

		// Compute text position
		x = this->padding(VideoWidget::PaddingLeft);
		x += (with_picto) ? this->height() : 0;
		y = 0;

		// Refresh dynamic info
		if (fg_buf_ != NULL)
			delete fg_buf_;

		this->createBox(&fg_buf_, this->width(), this->height());
		this->drawValue(fg_buf_, x, y, s);

skip:
		return fg_buf_;
	}

private:
	OIIO::ImageBuf *bg_buf_;
	OIIO::ImageBuf *fg_buf_;

	CadenceWidget(GPX2Video &app, std::string name)
		: VideoWidget(app, name) 
   		, bg_buf_(NULL)
   		, fg_buf_(NULL) {
	}
};

#endif

