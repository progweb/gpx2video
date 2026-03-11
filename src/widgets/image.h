#ifndef __GPX2VIDEO__WIDGETS__IMAGE_H__
#define __GPX2VIDEO__WIDGETS__IMAGE_H__

#include "../shape/base.h"


/**
 * Image shape widget
 */

class ImageShape : public ShapeBase {
public:
	virtual ~ImageShape() {
		clear();
	}

	static ImageShape * create(VideoWidget *widget) {
		ImageShape *shape;

		shape = new ImageShape(widget);
		
		shape->initialize();

		return shape;
	}

	OIIO::ImageBuf * prepare(bool &is_update) {
		if (bg_buf_ != NULL) {
			is_update = false;
			goto skip;
		}

		this->createBox(&bg_buf_, widget_->width(), widget_->height());
		this->drawBorder(bg_buf_);
		this->drawBackground(bg_buf_);
		this->drawImage(bg_buf_, widget_->border(), widget_->border(), widget_->source().c_str(), widget_->zoom());

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

	ImageShape(VideoWidget *widget) 
		: ShapeBase(theme_, widget)
		, bg_buf_(NULL) {
	}
};


/**
 * Widget definition
 */

class ImageWidget : public VideoWidget {
public:
	virtual ~ImageWidget() {
		delete shape_;
	}

	static ImageWidget * create(GPXApplication &app) {
		ImageWidget *widget;

		widget = new ImageWidget(app, "image");

		return widget;
	}

	void setShape(VideoWidget::Shape type) {
		(void) type;

		if (shape_)
			delete shape_;

		shape_ = ImageShape::create(this);
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

private:
	ImageShape *shape_;

	ImageWidget(GPXApplication &app, std::string name)
		: VideoWidget(app, name) 
   		, shape_(NULL) {
		setShape(VideoWidget::ShapeNone);
	}
};

#endif

