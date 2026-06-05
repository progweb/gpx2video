#ifndef __GPX2VIDEO__WIDGETS__MAXSPEED_H__
#define __GPX2VIDEO__WIDGETS__MAXSPEED_H__

#include "../shape/text.h"


/**
 * MaxSpeed text & icon shape widget
 */

class MaxSpeedTextShape : public TextShape {
public:
	virtual ~MaxSpeedTextShape() {
		clear();
	}

	static MaxSpeedTextShape * create(VideoWidget *widget) {
		MaxSpeedTextShape *shape;

		shape = new MaxSpeedTextShape(widget);
		
		return shape;
	}

	OIIO::ImageBuf * render(const TelemetryData &data, bool &is_update) {
		cairo_t *cairo;

		// Refresh dynamic info
		if (fg_buf_ != NULL) {
			if (data.type() == TelemetryData::TypeUnchanged) {
				is_update = false;
				goto skip;
			}

			if (no_value_ && !data.hasValue(TelemetryData::DataMaxSpeed)) {
				is_update = false;
				goto skip;
			}
		}

		// Format data
		no_value_ = !data.hasValue(TelemetryData::DataMaxSpeed);

		// Check changes
		if (fg_buf_ != NULL) {
			if (data.maxspeed() == last_speed_) {
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
		last_speed_ = data.maxspeed();
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

	double last_speed_;

	std::string icon_filename_;

	MaxSpeedTextShape(VideoWidget *widget)
		: TextShape(widget->theme())
		, bg_buf_(NULL)
		, fg_buf_(NULL)
		, widget_(widget) {
		no_value_ = false;

		last_speed_ = 0;

		icon_filename_ = widget->getIconFilename(widget->type());
	}

	void initialize(cairo_t *cr);
};


/**
 * Widget definition
 */

class MaxSpeedWidget : public VideoWidget, ShapeBase {
public:
	virtual ~MaxSpeedWidget() {
		delete shape_;
	}

	static MaxSpeedWidget * create(GPXApplication &app) {
		MaxSpeedWidget *widget;

		widget = new MaxSpeedWidget(app);

		widget->setValueUnit(TelemetryData::UnitMilesPerHour);

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
			shape_ = MaxSpeedTextShape::create(this);
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

		os << "<with-unit>" << VideoWidget::bool2string(theme().hasFlag(VideoWidget::Theme::FlagUnit)) << "</with-unit>" << std::endl;
		os << "<value-unit>" << unit2string(valueUnit()) << "</value-unit>" << std::endl;
	}

private:
	ShapeBase *shape_;

	MaxSpeedWidget(GPXApplication &app);
};

#endif

