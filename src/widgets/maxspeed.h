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
		
		shape->initialize();

		return shape;
	}

	OIIO::ImageBuf * prepare(bool &is_update) {
		bool with_picto = widget_->hasFlag(VideoWidget::FlagPicto);

		if (bg_buf_ != NULL) {
			is_update = false;
			goto skip;
		}

		this->createBox(&bg_buf_, widget_->width(), widget_->height());
		this->drawBorder(bg_buf_);
		this->drawBackground(bg_buf_);
		if (with_picto)
			this->drawImage(bg_buf_, widget_->border(), widget_->border(), "./assets/picto/DataOverlay_icn_maxspeed.png", VideoWidget::ZoomFit);
		this->drawLabel(bg_buf_, widget_->label().c_str());

		is_update = true;
skip:
		return bg_buf_;
	}

	OIIO::ImageBuf * render(const TelemetryData &data, bool &is_update) {
		char s[128];
		bool pace_unit = false;
		double speed = data.maxspeed();

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

		switch (widget_->unit()) {
		case VideoWidget::UnitMPH:
			speed *= 0.6213711922;
			break;
		case VideoWidget::UnitKPH:
			break;
		case VideoWidget::UnitMPM:
			speed *= 0.6213711922;
			pace_unit = true;
			break;
		case VideoWidget::UnitMPK:
			pace_unit = true;
			break;
		default:
			break;
		}

		if (pace_unit) {
			if (speed <= 0.0)
				no_value_ = true;
		}

		if (!no_value_)
			sprintf(s, "%.0f %s", speed, widget_->unit2string(widget_->unit()).c_str());
		else if (pace_unit) {
			double pace = 60.0 / speed;
			int min = (int) pace;
			int sec = (int) round((pace - min) * 60) % 60;

			sprintf(s, "%d:%02d %s", min, sec, widget_->unit2string(widget_->unit()).c_str());
		} 
		else
			sprintf(s, "-- %s", widget_->unit2string(widget_->unit()).c_str());

		// Refresh dynamic info
		if (fg_buf_ != NULL)
			delete fg_buf_;

		this->createBox(&fg_buf_, widget_->width(), widget_->height());
		this->drawValue(fg_buf_, s);

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

	Theme theme_;

	OIIO::ImageBuf *bg_buf_;
	OIIO::ImageBuf *fg_buf_;

	MaxSpeedTextShape(VideoWidget *widget) 
		: TextShape(theme_, widget)
		, bg_buf_(NULL)
		, fg_buf_(NULL) {
		no_value_ = false;
	}
};


/**
 * Widget definition
 */

class MaxSpeedWidget : public VideoWidget {
public:
	virtual ~MaxSpeedWidget() {
		delete shape_;
	}

	static MaxSpeedWidget * create(GPXApplication &app) {
		MaxSpeedWidget *widget;

		widget = new MaxSpeedWidget(app, "maxspeed");

		widget->setUnit(VideoWidget::UnitMPH);

		return widget;
	}

	void setShape(VideoWidget::Shape type) {
		if (shape_)
			delete shape_;

		switch (type) {
		case VideoWidget::ShapeText:
		default:
			shape_ = MaxSpeedTextShape::create(this);
			break;
		}
	}

	bool setBackgroundColor(std::string color) {
		bool result = VideoWidget::setBackgroundColor(color);

		const float *c = backgroundColor();

		shape_->theme().setBackgroundColor(c[0], c[1], c[2], c[3]);

		return result;
	}

	void initialize(void) {
		shape_->initialize();
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

		os << "<unit>" << unit2string(unit(), false) << "</unit>" << std::endl;
		os << "<text-shadow>" << textShadow() << "</text-shadow>" << std::endl;
	}

private:
	ShapeBase *shape_;

	MaxSpeedWidget(GPXApplication &app, std::string name)
		: VideoWidget(app, name) 
   		, shape_(NULL) {
		setShape(VideoWidget::ShapeText);
	}
};

#endif

