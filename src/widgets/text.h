#ifndef __GPX2VIDEO__WIDGETS__TEXT_H__
#define __GPX2VIDEO__WIDGETS__TEXT_H__

#include "../oiio.h"
#include "../shape/base.h"


/**
 * Text shape widget
 */

class TitleShape : public ShapeBase {
public:
	virtual ~TitleShape() {
		clear();
	}

	static TitleShape * create(VideoWidget *widget) {
		TitleShape *shape;

		shape = new TitleShape(widget);
		
		shape->initialize();

		return shape;
	}

	OIIO::ImageBuf * prepare(bool &is_update) {
		int x, y;

		int px;
		int x1, y1, x2, y2;
		int text_width, text_height;

		int border = widget_->border();
//		int padding_x = widget_->padding(VideoWidget::PaddingLeft);
		int padding_yt = widget_->padding(VideoWidget::PaddingTop);
		int padding_yb = widget_->padding(VideoWidget::PaddingBottom);

		if (bg_buf_ != NULL)
			goto skip;

		// Compute font size (1 pt = 1.333 px)
		// +-------------
		// |  Text    px
		// +-------------
		//        h = px + padding_top + padding_bottom
		px = widget_->height() - 2 * border - padding_yt - padding_yb;
		// pt = 3 * px / 4;

		// Create overlay buffer
		this->createBox(&bg_buf_, widget_->width(), widget_->height());
		this->drawBorder(bg_buf_);
		this->drawBackground(bg_buf_);

		this->textSize(widget_->text().c_str(), px,
				x1, y1, x2, y2,
				text_width, text_height);

		// Text offset
		x = -x1;
		y = -y1;

		// Text position
		x += border + (widget_->width() - text_width) / 2;
		y += border + (widget_->height() - text_height) / 2;

		this->drawText(bg_buf_, x, y, px, widget_->text().c_str());

		is_update = true;
skip:
		return bg_buf_;
	}

	OIIO::ImageBuf * render(const TelemetryData &data, bool &is_update) {
		(void) data;

		is_update = false;

		return NULL;
	}

	void clear(void) {
		if (bg_buf_)
			delete bg_buf_;

		bg_buf_ = NULL;
	}

private:
	Theme theme_;

	OIIO::ImageBuf *bg_buf_;

	TitleShape(VideoWidget *widget) 
		: ShapeBase(theme_, widget)
		, bg_buf_(NULL) {
	}
};


/**
 * Widget definition
 */

class TextWidget : public VideoWidget {
public:
	virtual ~TextWidget() {
		delete shape_;
	}

	static TextWidget * create(GPXApplication &app) {
		TextWidget *widget;

		widget = new TextWidget(app, "text");

		return widget;
	}

	void setShape(VideoWidget::Shape type) {
		(void) type;

		if (shape_)
			delete shape_;

		shape_ = TitleShape::create(this);
	}

	bool setBackgroundColor(std::string color) {
		bool result = VideoWidget::setBackgroundColor(color);

		const float *c = backgroundColor();

		shape_->theme().setBackgroundColor(c[0], c[1], c[2], c[3]);

		return result;
	}

	void initialize(void) {
		shape_->initialize();
	}

	OIIO::ImageBuf * prepare(bool &is_update) {
		return shape_->prepare(is_update);
	}

	OIIO::ImageBuf * render(const TelemetryData &data, bool &is_update) {
		return shape_->render(data, is_update);
	}

	void clear(void) {
		shape_->clear();
	}

protected:
	void xmlwrite(std::ostream &os) {
		VideoWidget::xmlwrite(os);

		IndentingOStreambuf indent(os, 4);

		os << "<text-shadow>" << textShadow() << "</text-shadow>" << std::endl;
	}

private:
	TitleShape *shape_;

	TextWidget(GPXApplication &app, std::string name)
		: VideoWidget(app, name) 
   		, shape_(NULL) {
		setShape(VideoWidget::ShapeNone);
	}
};

#endif

