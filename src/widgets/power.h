#ifndef __GPX2VIDEO__WIDGETS__POWER_H__
#define __GPX2VIDEO__WIDGETS__POWER_H__

#include "../shape/text.h"


/**
 * Power text & icon shape widget
 */

class PowerTextShape : public TextShape {
public:
	virtual ~PowerTextShape() {
		clear();
	}

	static PowerTextShape * create(VideoWidget *widget) {
		PowerTextShape *shape;

		shape = new PowerTextShape(widget);
		
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
			if (data.type() == TelemetryData::TypeUnchanged) {
				is_update = false;
				goto skip;
			}

			if (no_value_ && !data.hasValue(TelemetryData::DataPower)) {
				is_update = false;
				goto skip;
			}
		}

		// Format data
		no_value_ = !data.hasValue(TelemetryData::DataPower);

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

	PowerTextShape(VideoWidget *widget)
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
 * Widget definition
 */

class PowerWidget : public VideoWidget {
public:
	virtual ~PowerWidget() {
		delete shape_;
	}

	static PowerWidget * create(GPXApplication &app) {
		PowerWidget *widget;

		widget = new PowerWidget(app);

		return widget;
	}

	void setShape(VideoWidget::Shape type) {
		if (shape_)
			delete shape_;

		switch (type) {
		case VideoWidget::ShapeText:
			shape_ = PowerTextShape::create(this);
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

//		os << "<unit>" << unit2string(unit(), false) << "</unit>" << std::endl;
	}

private:
	ShapeBase *shape_;

	PowerWidget(GPXApplication &app)
		: VideoWidget(app, VideoWidget::WidgetPower) 
   		, shape_(NULL) {
		setShape(VideoWidget::ShapeText);
	}
};

#endif

