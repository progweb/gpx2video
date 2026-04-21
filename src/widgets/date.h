#ifndef __GPX2VIDEO__WIDGETS__DATE_H__
#define __GPX2VIDEO__WIDGETS__DATE_H__

#include "../shape/text.h"


/**
 * Date text & icon shape widget
 */

class DateTextShape : public TextShape {
public:
	virtual ~DateTextShape() {
		clear();
	}

	static DateTextShape * create(VideoWidget *widget) {
		DateTextShape *shape;

		shape = new DateTextShape(widget);
		
		return shape;
	}

	OIIO::ImageBuf * prepare(bool &is_update) {
		if (bg_buf_ != NULL) {
			is_update = false;
			goto skip;
		}

		this->initialize();
		this->createBox(&bg_buf_, theme().width(), theme().height());
		this->drawBorder(bg_buf_);
		this->drawBackground(bg_buf_);

		is_update = true;
skip:
		return bg_buf_;
	}

	OIIO::ImageBuf * render(const TelemetryData &data, bool &is_update) {
		cairo_t *cairo;

		time_t t;

		// Compute time
		t = data.datetime() / 1000;

		// Refresh dynamic info
		if (fg_buf_ != NULL) {
			if (t == last_time_) {
				is_update = false;
				goto skip;
			}
		}

		// Refresh dynamic info
		if (fg_buf_ != NULL)
			delete fg_buf_;

		this->createBox(&fg_buf_, theme().width(), theme().height());

		// Cairo context
		cairo = this->createCairoContext(fg_buf_);

		// Draw
		draw(cairo, data);

		// Data bytes
		this->renderCairoContext(fg_buf_, cairo);

		// Release
		this->destroyCairoContext(cairo);

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

	VideoWidget *widget_;

	DateTextShape(VideoWidget *widget) 
		: TextShape(widget->theme())
		, bg_buf_(NULL)
		, fg_buf_(NULL) 
		, widget_(widget) {
		last_time_ = 0;
	}

	void initialize(void);
	void draw(cairo_t *cr, const TelemetryData &data);
};


/**
 * Widget definition
 */

class DateWidget : public VideoWidget {
public:
	virtual ~DateWidget() {
		delete shape_;
	}

	static DateWidget * create(GPXApplication &app) {
		DateWidget *widget;

		widget = new DateWidget(app);

		widget->setFormat("%Y-%m-%d");

		return widget;
	}

	void setShape(VideoWidget::Shape type) {
		if (shape_)
			delete shape_;

		switch (type) {
		case VideoWidget::ShapeText:
			shape_ = DateTextShape::create(this);
			break;

		default:
			// TODO raise exception
			break;
		}
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

		shape_->xmlwrite(os);

		os << "<format>" << format() << "</format>" << std::endl;
	}

private:
	ShapeBase *shape_;

	DateWidget(GPXApplication &app)
		: VideoWidget(app, VideoWidget::WidgetDate) 
   		, shape_(NULL) {
		setShape(VideoWidget::ShapeText);
	}
};

#endif

