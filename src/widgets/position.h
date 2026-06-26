#ifndef __GPX2VIDEO__WIDGETS__POSITION_H__
#define __GPX2VIDEO__WIDGETS__POSITION_H__

#include "../shape/text.h"


/**
 * Position text & icon shape widget
 */

class PositionTextShape : public TextShape {
public:
	virtual ~PositionTextShape() {
		clear();
	}

	static PositionTextShape * create(VideoWidget *widget) {
		PositionTextShape *shape;

		shape = new PositionTextShape(widget);
		
		return shape;
	}

	bool hasFeature(ShapeBase::Feature feature) const {
		switch (feature) {
		case FeatureUnit:
			return false;

		default:
			break;
		}

		return TextShape::hasFeature(feature);
	}

	OIIO::ImageBuf * render(const TelemetryData &data, bool &is_update) {
		cairo_t *cairo;

		// Refresh dynamic info
		if (fg_buf_ != NULL) {
			if (data.type() == TelemetryData::TypeUnchanged) {
				is_update = false;
				goto skip;
			}

			if (no_value_ && !data.hasValue(TelemetryData::DataFix)) {
				is_update = false;
				goto skip;
			}
		}

		// Format data
		no_value_ = !data.hasValue(TelemetryData::DataFix);

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

	PositionTextShape(VideoWidget *widget)
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

class PositionWidget : public VideoWidget, ShapeBase {
public:
	virtual ~PositionWidget() {
		delete shape_;
	}

	static PositionWidget * create(GPXApplication &app, TelemetrySource *source = NULL) {
		PositionWidget *widget;

		widget = new PositionWidget(app, source);

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
			shape_ = PositionTextShape::create(this);
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

	PositionWidget(GPXApplication &app, TelemetrySource *source);
};

#endif

