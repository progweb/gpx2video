#ifndef __GPX2VIDEO__WIDGETS__DISTANCE_H__
#define __GPX2VIDEO__WIDGETS__DISTANCE_H__

#include "../shape/bar.h"
#include "../shape/text.h"


/**
 * Distance text & icon shape widget
 */

class DistanceTextShape : public TextShape {
public:
	virtual ~DistanceTextShape() {
		clear();
	}

	static DistanceTextShape * create(VideoWidget *widget) {
		DistanceTextShape *shape;

		shape = new DistanceTextShape(widget);
		
		return shape;
	}

	OIIO::ImageBuf * render(const TelemetryData &data, bool &is_update) {
		cairo_t *cairo;

		// Refresh dynamic info
		if (fg_buf_ != NULL) {
			if ((data.type() == TelemetryData::TypeUnchanged)) {
				is_update = false;
				goto skip;
			}

			if (no_value_ && !data.hasValue(TelemetryData::DataDistance)) {
				is_update = false;
				goto skip;
			}
		}

		// Format data
		no_value_ = !data.hasValue(TelemetryData::DataDistance);

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

	std::string icon_filename_;

	DistanceTextShape(VideoWidget *widget) 
		: TextShape(widget->theme())
		, bg_buf_(NULL)
		, fg_buf_(NULL) 
		, widget_(widget) {
		no_value_ = false;
	}

	void initialize(cairo_t *cr);
};


/**
 * Distance bar shape widget
 */

class DistanceBarShape : public BarShape {
public:
	virtual ~DistanceBarShape() {
		clear();
	}

	static DistanceBarShape * create(VideoWidget *widget) {
		DistanceBarShape *shape;

		shape = new DistanceBarShape(widget->theme(), widget);

		return shape;
	}

	OIIO::ImageBuf * render(const TelemetryData &data, bool &is_update) {
		cairo_t *cairo;

		// Refresh dynamic info
		if (fg_buf_ != NULL) {
			if ((data.type() == TelemetryData::TypeUnchanged)) {
				is_update = false;
				goto skip;
			}

			if (no_value_ && !data.hasValue(TelemetryData::DataDistance)) {
				is_update = false;
				goto skip;
			}
		}

		// Format data
		no_value_ = !data.hasValue(TelemetryData::DataDistance);

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

skip:
		return fg_buf_;
	}

	bool updated(const TelemetryData &data) const;
	void draw(cairo_t *cr, const TelemetryData &data);

	void clear(void);

private:
	bool no_value_;

	int tick_step_;
	int tick_mstep_;

	int tick_label_width_;
	int tick_label_height_;

	std::string icon_filename_;

	OIIO::ImageBuf *bg_buf_;
	OIIO::ImageBuf *fg_buf_;

	VideoWidget *widget_;

	DistanceBarShape(VideoWidget::Theme &theme, VideoWidget *widget) 
		: BarShape(theme)
		, bg_buf_(NULL)
		, fg_buf_(NULL)
		, widget_(widget) {
		no_value_ = false;
	}

	void initialize(cairo_t *cr);
	void ticklenwidth(int value, double *offset, double *len, double *width);
};


/**
 * Widget definition
 */

class DistanceWidget : public VideoWidget, ShapeBase {
public:
	virtual ~DistanceWidget() {
		delete shape_;
	}

	static DistanceWidget * create(GPXApplication &app, TelemetrySource *source = NULL) {
		DistanceWidget *widget;

		widget = new DistanceWidget(app, source);

		widget->setValueUnit(TelemetryData::UnitMiles);

		return widget;
	}

	ShapeBase * shape(void) {
		return shape_;
	}

	void setShape(VideoWidget::Shape type) {
		if (shape_)
			delete shape_;

		switch (type) {
		case VideoWidget::ShapeBar:
			shape_ = DistanceBarShape::create(this);
			break;

		case VideoWidget::ShapeText:
			shape_ = DistanceTextShape::create(this);
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

		os << "<value-unit>" << unit2string(valueUnit()) << "</value-unit>" << std::endl;
	}

private:
	ShapeBase *shape_;

	DistanceWidget(GPXApplication &app, TelemetrySource *source);
};

#endif

