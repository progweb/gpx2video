#ifndef __GPX2VIDEO__WIDGETS__ELEVATION_H__
#define __GPX2VIDEO__WIDGETS__ELEVATION_H__

#include "../shape/bar.h"
#include "../shape/text.h"


/**
 * Elevation text & icon shape widget
 */

class ElevationTextShape : public TextShape {
public:
	virtual ~ElevationTextShape() {
		clear();
	}

	static ElevationTextShape * create(VideoWidget *widget) {
		ElevationTextShape *shape;

		shape = new ElevationTextShape(widget);
		
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

		// Refresh dynamic info
		if (fg_buf_ != NULL) {
			if ((data.type() == TelemetryData::TypeUnchanged)) {
				is_update = false;
				goto skip;
			}

			if (no_value_ && !data.hasValue(TelemetryData::DataElevation)) {
				is_update = false;
				goto skip;
			}
		}

		// Format data
		no_value_ = !data.hasValue(TelemetryData::DataElevation);

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

	void clear(void) {
		no_value_ = false;

		if (bg_buf_)
			delete bg_buf_;

		if (fg_buf_)
			delete fg_buf_;

		bg_buf_ = NULL;
		fg_buf_ = NULL;
	}

private:
	bool no_value_;

	OIIO::ImageBuf *bg_buf_;
	OIIO::ImageBuf *fg_buf_;

	VideoWidget *widget_;

	ElevationTextShape(VideoWidget *widget)
		: TextShape(widget->theme())
		, bg_buf_(NULL)
		, fg_buf_(NULL)
		, widget_(widget) {
		no_value_ = false;
	}

	void initialize(void);
	void draw(cairo_t *cr, const TelemetryData &data);
};


/**
 * Elevation bar shape widget
 */

class ElevationBarShape : public BarShape {
public:
	virtual ~ElevationBarShape() {
		clear();
	}

	static ElevationBarShape * create(VideoWidget *widget) {
		ElevationBarShape *shape;

		cairo_font_face_t *fontface;

		fontface = cairo_toy_font_face_create(
			"Roboto", 
			CAIRO_FONT_SLANT_NORMAL, 
			CAIRO_FONT_WEIGHT_NORMAL
		);

		shape = new ElevationBarShape(widget->theme(), fontface, widget);

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

		// Refresh dynamic info
		if (fg_buf_ != NULL) {
			if ((data.type() == TelemetryData::TypeUnchanged)) {
				is_update = false;
				goto skip;
			}

			if (no_value_ && !data.hasValue(TelemetryData::DataElevation)) {
				is_update = false;
				goto skip;
			}
		}

		// Format data
		no_value_ = !data.hasValue(TelemetryData::DataElevation);

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

	void clear(void) {
		no_value_ = false;

		if (bg_buf_)
			delete bg_buf_;

		if (fg_buf_)
			delete fg_buf_;

		bg_buf_ = NULL;
		fg_buf_ = NULL;
	}

private:
	bool no_value_;

	cairo_font_face_t *fontface_;

	int size_;
	int width_;
	int height_;

	int tick_step_;
	int tick_mstep_;

	OIIO::ImageBuf *bg_buf_;
	OIIO::ImageBuf *fg_buf_;

	VideoWidget *widget_;

	ElevationBarShape(VideoWidget::Theme &theme, cairo_font_face_t *fontface, VideoWidget *widget) 
		: BarShape(theme, fontface)
		, fontface_(fontface)
		, bg_buf_(NULL)
		, fg_buf_(NULL)
		, widget_(widget) {
		no_value_ = false;

		cairo_font_face_reference(fontface);
	}

	void initialize(void);
	void ticklenwidth(int value, int *offset, int *len, int *width);
	void draw(cairo_t *cr, const TelemetryData &data);
};


/**
 * Widget definition
 */

class ElevationWidget : public VideoWidget {
public:
	virtual ~ElevationWidget() {
		delete shape_;
	}

	static ElevationWidget * create(GPXApplication &app) {
		ElevationWidget *widget;

		widget = new ElevationWidget(app, "elevation");

		widget->setUnit(VideoWidget::UnitMiles);

		return widget;
	}

	void setShape(VideoWidget::Shape type) {
		VideoWidget::setShape(type);

		if (shape_)
			delete shape_;

		switch (type) {
		case VideoWidget::ShapeBar:
			shape_ = ElevationBarShape::create(this);
			break;

		case VideoWidget::ShapeText:
			shape_ = ElevationTextShape::create(this);
			break;

		default:
			// TODO raise exception
			break;
		}
	}

	bool isShapeSupported(VideoWidget::Shape type) {
		switch (type) {
		case VideoWidget::ShapeBar:
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

		os << "<unit>" << unit2string(unit(), false) << "</unit>" << std::endl;
	}

private:
	ShapeBase *shape_;

	ElevationWidget(GPXApplication &app, std::string name)
		: VideoWidget(app, name) 
   		, shape_(NULL) {
		setShape(VideoWidget::ShapeText);
	}
};

#endif

