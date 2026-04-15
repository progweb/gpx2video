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

	void clear(void) {
		if (bg_buf_)
			delete bg_buf_;

		if (fg_buf_)
			delete fg_buf_;

		bg_buf_ = NULL;
		fg_buf_ = NULL;
	}

private:
	OIIO::ImageBuf *bg_buf_;
	OIIO::ImageBuf *fg_buf_;

	VideoWidget *widget_;

	time_t last_time_;

	TimeTextShape(VideoWidget *widget)
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
 * Speed arc shape widget
 */

class TimeArcShape : public ArcShape {
public:
	virtual ~TimeArcShape() {
		clear();
	}

	static TimeArcShape * create(VideoWidget *widget) {
		TimeArcShape *shape;

		cairo_font_face_t *fontface;

		fontface = cairo_toy_font_face_create(
			"Roboto", 
			CAIRO_FONT_SLANT_NORMAL, 
			CAIRO_FONT_WEIGHT_NORMAL
		);

		shape = new TimeArcShape(widget->theme(), fontface, widget);

		return shape;
	}

	OIIO::ImageBuf * prepare(bool &is_update) {
		is_update = false;

		this->initialize();

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

	void clear(void) {
		if (bg_buf_)
			delete bg_buf_;

		if (fg_buf_)
			delete fg_buf_;

		bg_buf_ = NULL;
		fg_buf_ = NULL;
	}

private:
	cairo_font_face_t *fontface_;

	int size_;
	int width_;
	int height_;

	int tick_step_;
	int tick_mstep_;

	OIIO::ImageBuf *bg_buf_;
	OIIO::ImageBuf *fg_buf_;

	VideoWidget *widget_;

	time_t last_time_;

	TimeArcShape(VideoWidget::Theme &theme, cairo_font_face_t *fontface, VideoWidget *widget) 
		: ArcShape(theme, fontface)
		, fontface_(fontface)
		, bg_buf_(NULL)
		, fg_buf_(NULL)
		, widget_(widget) {
		last_time_ = 0;

		cairo_font_face_reference(fontface);
	}

	void initialize(void);
	void tickinit(int min, int max);
	void ticklenwidth(int value, int *len, int *width);
	void draw(cairo_t *cr, const TelemetryData &data);
	void drawNeedle(cairo_t *cr, VideoWidget::Theme::NeedleType type, double xa, double len, bool design,
			const float *color1, const float *color2);
};


/**
 * Widget definition
 */

class TimeWidget : public VideoWidget {
public:
	virtual ~TimeWidget() {
		delete shape_;
	}

	static TimeWidget * create(GPXApplication &app) {
		TimeWidget *widget;

		widget = new TimeWidget(app, "time");

		return widget;
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

	bool isShapeSupported(VideoWidget::Shape type) {
		switch (type) {
		case VideoWidget::ShapeArc:
		case VideoWidget::ShapeText:
			return true;
		default:
			return false;
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
	}

private:
	ShapeBase *shape_;

	TimeWidget(GPXApplication &app, std::string name)
		: VideoWidget(app, name) 
   		, shape_(NULL) {
		setShape(VideoWidget::ShapeText);
	}
};

#endif

