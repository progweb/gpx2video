#ifndef __GPX2VIDEO__WIDGETS__GRADE_H__
#define __GPX2VIDEO__WIDGETS__GRADE_H__

#include "log.h"
#include "videowidget.h"


class GradeWidget : public VideoWidget {
public:
	virtual ~GradeWidget() {
		log_call();

		if (buf_)
			delete buf_;
	}

	static GradeWidget * create(GPX2Video &app) {
		GradeWidget *widget;

		log_call();

		widget = new GradeWidget(app, "grade");

		return widget;
	}

	void prepare(OIIO::ImageBuf *buf) {
		this->createBox(&buf_, this->width(), this->height());
		this->drawBorder(buf_);
		this->drawBackground(buf_);
		this->drawImage(buf_, this->border(), this->border(), "./assets/picto/DataOverlay_icn_grade.png", VideoWidget::ZoomFit);
//		this->drawLabel(buf_, 0, 0, label().c_str());
//		this->drawValue(buf_, 0, 0, "22 km");

		// Image over
		buf_->specmod().x = this->x();
		buf_->specmod().y = this->y();
		OIIO::ImageBufAlgo::over(*buf, *buf_, *buf, OIIO::ROI());
	}

	void render(OIIO::ImageBuf *buf, const GPXData &data) {
		char s[128];

		sprintf(s, "%.0f%%", data.grade());

		// Append dynamic info
		this->drawLabel(buf, this->x() + this->height() + this->padding(), this->y(), label().c_str());
		this->drawValue(buf, this->x() + this->height() + this->padding(), this->y(), s);
	}

private:
	OIIO::ImageBuf *buf_;

	GradeWidget(GPX2Video &app, std::string name)
		: VideoWidget(app, name) 
		, buf_(NULL) {
	}
};

#endif

