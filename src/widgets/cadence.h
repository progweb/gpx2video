#ifndef __GPX2VIDEO__WIDGETS__CADENCE_H__
#define __GPX2VIDEO__WIDGETS__CADENCE_H__

#include "log.h"
#include "videowidget.h"


class CadenceWidget : public VideoWidget {
public:
	virtual ~CadenceWidget() {
		log_call();

		if (buf_)
			delete buf_;
	}

	static CadenceWidget * create(GPX2Video &app) {
		CadenceWidget *widget;

		log_call();

		widget = new CadenceWidget(app, "cadence");

		return widget;
	}

	void prepare(OIIO::ImageBuf *buf) {
		bool with_picto = this->hasFlag(VideoWidget::FlagPicto);

		if (buf_ == NULL) {
			this->createBox(&buf_, this->width(), this->height());
			this->drawBorder(buf_);
			this->drawBackground(buf_);
			if (with_picto)
				this->drawImage(buf_, this->border(), this->border(), "./assets/picto/DataOverlay_icn_cadence.png", VideoWidget::ZoomFit);
//			this->drawLabel(buf_, 0, 0, label().c_str());
		}

		// Image over
		buf_->specmod().x = this->x();
		buf_->specmod().y = this->y();
		OIIO::ImageBufAlgo::over(*buf, *buf_, *buf, OIIO::ROI());
	}

	void render(OIIO::ImageBuf *buf, const GPXData &data) {
		int x, y;

		char s[128];

		bool with_picto = this->hasFlag(VideoWidget::FlagPicto);

		if (data.hasValue(GPXData::DataCadence))
			sprintf(s, "%d tr/min", data.cadence());
		else
			sprintf(s, "-- tr/min");

		// Append dynamic info
		x = this->x() + this->padding(VideoWidget::PaddingLeft);
		x += (with_picto) ? this->height() : 0;
		y = this->y();

		this->drawLabel(buf, x, y, label().c_str());
		this->drawValue(buf, x, y, s);
	}

private:
	OIIO::ImageBuf *buf_;

	CadenceWidget(GPX2Video &app, std::string name)
		: VideoWidget(app, name) 
		, buf_(NULL) {
	}
};

#endif

