#ifndef __GPX2VIDEO__WIDGETS__TEXT_H__
#define __GPX2VIDEO__WIDGETS__TEXT_H__

#include "log.h"
#include "videowidget.h"


class TextWidget : public VideoWidget {
public:
	virtual ~TextWidget() {
		log_call();

		if (buf_)
			delete buf_;
	}

	static TextWidget * create(GPX2Video &app) {
		TextWidget *widget;

		log_call();

		widget = new TextWidget(app, "text");

		return widget;
	}

	void prepare(OIIO::ImageBuf *buf) {
		if (buf_ == NULL) {
			this->createBox(&buf_, this->width(), this->height());
			this->drawBorder(buf_);
			this->drawBackground(buf_);
		}

		// Image over
		buf_->specmod().x = this->x();
		buf_->specmod().y = this->y();
		OIIO::ImageBufAlgo::over(*buf, *buf_, *buf, OIIO::ROI());
	}

	void render(OIIO::ImageBuf *buf, const GPXData &data) {
		int h;

		int space_x, space_y;
		int border = this->border();
		int padding_x = this->padding(VideoWidget::PaddingLeft);
		int padding_yt = this->padding(VideoWidget::PaddingTop);
		int padding_yb = this->padding(VideoWidget::PaddingBottom);

		(void) data;

		// width x height
		h = this->height() - 2 * border;

		// Add text (1 pt = 1.333 px)
		// +-------------
		// |  Text    px
		// +-------------
		//        h = px + padding_top + padding_bottom
		int px = h - padding_yt - padding_yb;
		int pt = 3 * px / 4;

		space_x = padding_x + border;
		space_y = padding_yt + border;

		this->drawText(buf, this->x() + space_x, this->y() + space_y, pt, this->text().c_str());
	}

//void drawText(OIIO::ImageBuf *buf, int x, int y, const char *label) {
//	bool result;
//
//	int h;
//
//	int border = this->border();
//	int padding_x = this->padding(VideoWidget::PaddingLeft);
//	int padding_yt = this->padding(VideoWidget::PaddingTop);
//	int padding_yb = this->padding(VideoWidget::PaddingBottom);
//
//	// Apply border
//	x += border;
//	y += border;
//
//	// width x height
//	h = this->height() - 2 * border;
//
//	// Add text (1 pt = 1.333 px)
//	// +-------------
//	// |  Text    px
//	// +-------------
//	//        h = px + padding_top + padding_bottom
//	int px = h - padding_yt - padding_yb;
//	int pt = 3 * px / 4;
//
//	float color[4]; // = { 1.0, 1.0, 1.0, 1.0 };
//
//	memcpy(color, this->textColor(), sizeof(color));
//
//	result = OIIO::ImageBufAlgo::render_text(*buf, 
//		x + padding_x, 
//		y + padding_yt, 
//		label, 
//		pt, this->font(), color, 
//		OIIO::ImageBufAlgo::TextAlignX::Left, 
//		OIIO::ImageBufAlgo::TextAlignY::Top, 
//		this->textShadow());
//
//	if (result == false)
//		fprintf(stderr, "render text error\n");
//}



private:
	OIIO::ImageBuf *buf_;

	TextWidget(GPX2Video &app, std::string name)
		: VideoWidget(app, name) 
		, buf_(NULL) {
	}
};

#endif

