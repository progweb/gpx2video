#ifndef __GPX2VIDEO__WIDGETS__CADENCE_H__
#define __GPX2VIDEO__WIDGETS__CADENCE_H__

#include "log.h"
#include "videowidget.h"


class CadenceWidget : public VideoWidget {
public:
	~CadenceWidget() {
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

	void prepare(void) {
		const int w = 64;

		double divider = (double) (this->height() - (2 * this->border())) / (double) w;

		this->createBox(&buf_, this->width(), this->height());
		this->drawBorder(buf_);
		this->drawBackground(buf_);
		this->drawImage(buf_, this->border(), this->border(), "./assets/picto/DataOverlay_icn_cadence.png", divider);
//		this->drawLabel(buf_, 0, 0, label().c_str());
//		this->drawValue(buf_, 0, 0, "22 km");
	}

	void render(OIIO::ImageBuf *buf, const GPXData &data) {
		char s[128];

		sprintf(s, "%d tr/min", data.cadence());

		// Image over
		buf_->specmod().x = this->x();
		buf_->specmod().y = this->y();
		OIIO::ImageBufAlgo::over(*buf, *buf_, *buf, OIIO::ROI());

		// Append dynamic info
		this->drawLabel(buf, this->x(), this->y(), label().c_str());
		this->drawValue(buf, this->x(), this->y(), s);
	}

private:
	OIIO::ImageBuf *buf_;

	CadenceWidget(GPX2Video &app, std::string name)
		: VideoWidget(app, name) 
		, buf_(NULL) {
	}
};

#endif

