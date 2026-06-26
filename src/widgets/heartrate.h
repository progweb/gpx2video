#ifndef __GPX2VIDEO__WIDGETS__HEARTRATE_H__
#define __GPX2VIDEO__WIDGETS__HEARTRATE_H__

#include "../shape/text.h"


/**
 * HeartRate text & icon shape widget
 */

class HeartRateTextShape : public TextShape {
public:
	virtual ~HeartRateTextShape() {
		clear();
	}

	static HeartRateTextShape * create(VideoWidget *widget) {
		HeartRateTextShape *shape;

		shape = new HeartRateTextShape(widget);
		
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

			if (no_value_ && !data.hasValue(TelemetryData::DataHeartrate)) {
				is_update = false;
				goto skip;
			}
		}

		// Format data
		no_value_ = !data.hasValue(TelemetryData::DataHeartrate);

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

	HeartRateTextShape(VideoWidget *widget)
		: TextShape(widget->theme())
		, bg_buf_(NULL)
		, fg_buf_(NULL)
		, widget_(widget) {
		no_value_ = false;
	}

	void initialize(cairo_t *cr);
};


/**
 * Widget definition
 */

class HeartRateWidget : public VideoWidget, ShapeBase {
public:
	virtual ~HeartRateWidget() {
		delete shape_;
	}

	static HeartRateWidget * create(GPXApplication &app, TelemetrySource *source = NULL) {
		HeartRateWidget *widget;

		widget = new HeartRateWidget(app, source);

		widget->setValueUnit(TelemetryData::UnitBPM);

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
			shape_ = HeartRateTextShape::create(this);
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

	HeartRateWidget(GPXApplication &app, TelemetrySource *source);
};

#endif

