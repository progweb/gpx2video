#ifndef __GPX2VIDEO__WIDGETS__TIME_H__
#define __GPX2VIDEO__WIDGETS__TIME_H__

#include "../shape/arc.h"
#include "../shape/text.h"


/**
 * Time text & icon shape widget
 */

class TimeTextShape : public TextShape {
public:
	virtual ~TimeTextShape() {
		clear();
	}

	static TimeTextShape * create(VideoWidget *widget) {
		TimeTextShape *shape;

		shape = new TimeTextShape(widget);
		
		return shape;
	}

	OIIO::ImageBuf * render(const TelemetryData &data, bool &is_update) {
		cairo_t *cairo;

		time_t t;

		// Compute time
		t = data.datetime() / 1000;

		// Check changes
		if (fg_buf_ != NULL) {
			if (t == last_time_) {
				is_update = false;
				goto skip;
			}
		}

		// Refresh dynamic info
		if (fg_buf_ != NULL)
			delete fg_buf_;

		// Image buffer
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

	bool updated(const TelemetryData &data) const; 
	void draw(cairo_t *cr, const TelemetryData &data);
	void clear(void);

private:
	OIIO::ImageBuf *bg_buf_;
	OIIO::ImageBuf *fg_buf_;

	VideoWidget *widget_;

	time_t last_time_;

	std::string icon_filename_;

	TimeTextShape(VideoWidget *widget)
		: TextShape(widget->theme())
		, bg_buf_(NULL)
		, fg_buf_(NULL)
		, widget_(widget) {
		last_time_ = 0;

		icon_filename_ = widget->getIconFilename(widget->type());
	}

	void initialize(cairo_t *cr);
};


/**
 * Speed arc shape widget
 */

class TimeArcShape : public ArcShape {
public:
	virtual ~TimeArcShape() {
		clear();
	}

	static TimeArcShape * create(VideoWidget *widget) {
		TimeArcShape *shape;

		shape = new TimeArcShape(widget->theme(), widget);

		return shape;
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

		// Image buffer
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

	bool updated(const TelemetryData &data) const;
	void draw(cairo_t *cr, const TelemetryData &data);

	void clear(void);

private:
	int size_;
	int width_;
	int height_;

	int tick_step_;
	int tick_mstep_;

	OIIO::ImageBuf *bg_buf_;
	OIIO::ImageBuf *fg_buf_;

	VideoWidget *widget_;

	time_t last_time_;

	TimeArcShape(VideoWidget::Theme &theme, VideoWidget *widget) 
		: ArcShape(theme)
		, bg_buf_(NULL)
		, fg_buf_(NULL)
		, widget_(widget) {
		last_time_ = 0;
	}

	void initialize(cairo_t *cr);
	void tickinit(int min, int max);
	void ticklenwidth(int value, int *len, int *width);
};


/**
 * Widget definition
 */

class TimeWidget : public VideoWidget, ShapeBase {
public:
	virtual ~TimeWidget() {
		delete shape_;
	}

	static TimeWidget * create(GPXApplication &app) {
		TimeWidget *widget;

		widget = new TimeWidget(app);

		return widget;
	}

	ShapeBase * shape(void) {
		return shape_;
	}

	void setShape(VideoWidget::Shape type) {
		VideoWidget::setShape(type);

		if (shape_)
			delete shape_;

		switch (type) {
		case VideoWidget::ShapeArc:
			shape_ = TimeArcShape::create(this);
			break;

		case VideoWidget::ShapeText:
			shape_ = TimeTextShape::create(this);
			break;

		default:
			// TODO raise exception
			break;
		}
	}

	OIIO::ImageBuf * render(const TelemetryData &data, bool &is_update) {
		return shape_->render(data, is_update);
	}

	bool updated(const TelemetryData &data) const {
		return shape_->updated(data);
	}

	void draw(cairo_t *cairo, const TelemetryData &data) {
		shape_->draw(cairo, data);
	}

	void clear(void) {
		shape_->clear();
	}

protected:
	void xmlwrite(std::ostream &os) {
		VideoWidget::xmlwrite(os);

		shape_->xmlwrite(os);
	}

private:
	ShapeBase *shape_;

	TimeWidget(GPXApplication &app);
};

#endif

