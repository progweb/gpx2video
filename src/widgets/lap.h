#ifndef __GPX2VIDEO__WIDGETS__LAP_H__
#define __GPX2VIDEO__WIDGETS__LAP_H__

#include "../shape/text.h"


/**
 * Lap text & icon shape widget
 */

class LapTextShape : public TextShape {
public:
	virtual ~LapTextShape() {
		clear();
	}

	static LapTextShape * create(VideoWidget *widget, int nbr_target_lap) {
		LapTextShape *shape;

		shape = new LapTextShape(widget);
		
		shape->initialize();
		shape->setTargetLap(nbr_target_lap);

		return shape;
	}

	void setTargetLap(int target) {
		nbr_target_lap_ = target;
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
			this->drawImage(bg_buf_, widget_->border(), widget_->border(), "./assets/picto/DataOverlay_icn_laps.png", VideoWidget::ZoomFit);
		this->drawLabel(bg_buf_, widget_->label().c_str());

		is_update = true;
skip:
		return bg_buf_;
	}

	OIIO::ImageBuf * render(const TelemetryData &data, bool &is_update) {
		char s[128];
		int lap = data.lap();

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

		if (!no_value_)
			sprintf(s, "%d/%d", lap, nbr_target_lap_);
		else
			sprintf(s, "--/--");

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

	int nbr_target_lap_;

	LapTextShape(VideoWidget *widget) 
		: TextShape(theme_, widget)
		, bg_buf_(NULL)
		, fg_buf_(NULL) {
		no_value_ = false;
	}
};


/**
 * Widget definition
 */

class LapWidget : public VideoWidget {
public:
	virtual ~LapWidget() {
		delete shape_;
	}

	static LapWidget * create(GPXApplication &app) {
		LapWidget *widget;

		widget = new LapWidget(app, "lap");

		return widget;
	}

	void setTargetLap(int target) {
		shape_->setTargetLap(target);

		nbr_target_lap_ = target;
	}

	void setShape(VideoWidget::Shape type) {
		(void) type;

		if (shape_)
			delete shape_;

		shape_ = LapTextShape::create(this, nbr_target_lap_);
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

		os << "<text-shadow>" << textShadow() << "</text-shadow>" << std::endl;
	}

private:
	LapTextShape *shape_;

	int nbr_target_lap_;

	LapWidget(GPXApplication &app, std::string name)
		: VideoWidget(app, name)
   		, shape_(NULL)
   		, nbr_target_lap_(1) {
		setShape(VideoWidget::ShapeText);
	}
};

#endif

