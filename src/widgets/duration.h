#ifndef __GPX2VIDEO__WIDGETS__DURATION_H__
#define __GPX2VIDEO__WIDGETS__DURATION_H__

#include "../shape/text.h"


/**
 * Duration text & icon shape widget
 */

class DurationTextShape : public TextShape {
public:
	virtual ~DurationTextShape() {
		clear();
	}

	static DurationTextShape * create(VideoWidget *widget) {
		DurationTextShape *shape;

		shape = new DurationTextShape(widget);
		
		return shape;
	}

	OIIO::ImageBuf * render(const TelemetryData &data, bool &is_update) {
		cairo_t *cairo;

		int duration;

		// Compute duration
		duration = data.duration();

		// Refresh dynamic info
		if (fg_buf_ != NULL) {
//			if (data.type() == TelemetryData::TypeUnchanged) {
//				is_update = false;
//				goto skip;
//			}

			if (duration == last_duration_) {
				is_update = false;
				goto skip;
			}

			if (no_value_ && !data.hasValue(TelemetryData::DataDuration)) {
				is_update = false;
				goto skip;
			}
		}

		// Format data
		no_value_ = !data.hasValue(TelemetryData::DataDuration);

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
		last_duration_ = duration;
skip:
		return fg_buf_;
	}

	bool updated(const TelemetryData &data) const;
	void draw(cairo_t *cr, const TelemetryData &data);
	void clear(void);

private:
	bool no_value_;

	OIIO::ImageBuf *bg_buf_;
	OIIO::ImageBuf *fg_buf_;

	VideoWidget *widget_;

	int last_duration_;

	std::string icon_filename_;

	DurationTextShape(VideoWidget *widget) 
		: TextShape(widget->theme())
		, bg_buf_(NULL)
		, fg_buf_(NULL) 
		, widget_(widget) {
		no_value_ = false;

		last_duration_ = 0;
	}

	void initialize(cairo_t *cr);
};


/**
 * Widget definition
 */

class DurationWidget : public VideoWidget, ShapeBase {
public:
	virtual ~DurationWidget() {
		delete shape_;
	}

	static DurationWidget * create(GPXApplication &app) {
		DurationWidget *widget;

		widget = new DurationWidget(app);

		return widget;
	}

	ShapeBase * shape(void) {
		return shape_;
	}

	void setShape(VideoWidget::Shape type) {
		if (shape_)
			delete shape_;

		switch (type) {
		case VideoWidget::ShapeText:
			shape_ = DurationTextShape::create(this);
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

	DurationWidget(GPXApplication &app);
};

#endif

