#ifndef __GPX2VIDEO__WIDGETS__SPEED_H__
#define __GPX2VIDEO__WIDGETS__SPEED_H__

#include "../shape/arc.h"
#include "../shape/text.h"
#include "../videowidget.h"


/**
 * Speed text & icon shape widget
 */

class SpeedTextShape : public TextShape {
public:
	virtual ~SpeedTextShape() {
		clear();
	}

	static SpeedTextShape * create(VideoWidget *widget) {
		SpeedTextShape *shape;

		shape = new SpeedTextShape(widget);
		
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

			if (no_value_ && !data.hasValue(TelemetryData::DataSpeed)) {
				is_update = false;
				goto skip;
			}
		}

		// Format data
		no_value_ = !data.hasValue(TelemetryData::DataSpeed);

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

	SpeedTextShape(VideoWidget *widget)
		: TextShape(widget->theme())
		, bg_buf_(NULL)
		, fg_buf_(NULL)
		, widget_(widget) {
		no_value_ = false;

		icon_filename_ = widget->getIconFilename(widget->type());
	}

	void initialize(cairo_t *cr);
};


/**
 * Speed arc shape widget
 */

class SpeedArcShape : public ArcShape {
public:
	virtual ~SpeedArcShape() {
		clear();
	}

	static SpeedArcShape * create(VideoWidget *widget) {
		SpeedArcShape *shape;

		shape = new SpeedArcShape(widget->theme(), widget);

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

			if (no_value_ && !data.hasValue(TelemetryData::DataSpeed)) {
				is_update = false;
				goto skip;
			}
		}

		// Format data
		no_value_ = !data.hasValue(TelemetryData::DataSpeed);

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

	int size_;
	int width_;
	int height_;

	int tick_step_;
	int tick_mstep_;

	OIIO::ImageBuf *bg_buf_;
	OIIO::ImageBuf *fg_buf_;

	VideoWidget *widget_;

	SpeedArcShape(VideoWidget::Theme &theme, VideoWidget *widget) 
		: ArcShape(theme)
		, bg_buf_(NULL)
		, fg_buf_(NULL)
		, widget_(widget) {
		no_value_ = false;
	}

	void initialize(cairo_t *cr);
	void tickinit(int min, int max);
	void ticklenwidth(int value, int *len, int *width);
};


/**
 * Widget definition
 */

class SpeedWidget : public VideoWidget, ShapeBase {
public:
	virtual ~SpeedWidget() {
		delete shape_;
	}

	static SpeedWidget * create(GPXApplication &app) {
		SpeedWidget *widget;

		widget = new SpeedWidget(app);

		widget->setValueUnit(TelemetryData::UnitMilesPerHour);

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
			shape_ = SpeedArcShape::create(this);
			break;

		case VideoWidget::ShapeText:
			shape_ = SpeedTextShape::create(this);
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

	SpeedWidget(GPXApplication &app);
};

#endif

