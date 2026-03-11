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
			this->drawImage(bg_buf_, widget_->border(), widget_->border(), "./assets/picto/DataOverlay_icn_duration.png", VideoWidget::ZoomFit);
		this->drawLabel(bg_buf_, widget_->label().c_str());

		is_update = true;
skip:
		return bg_buf_;
	}

	OIIO::ImageBuf * render(const TelemetryData &data, bool &is_update) {
		char s[128];

		int hours = 0;
		int minutes = 0;
		int seconds = 0;

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

		if (duration > 0) {
			seconds = duration % 60;
			duration = duration / 60;
			minutes = duration % 60;
			hours = duration / 60;
		}

		if (!no_value_)
			sprintf(s, "%d:%02d:%02d", hours, minutes, seconds);
		else
			sprintf(s, "--:--:--");

		// Refresh dynamic info
		if (fg_buf_ != NULL)
			delete fg_buf_;

		this->createBox(&fg_buf_, widget_->width(), widget_->height());
		this->drawValue(fg_buf_, s);

		is_update = true;
		last_duration_ = duration;
skip:
		return fg_buf_;
	}

	void clear(void) {
		no_value_ = false;

		last_duration_ = 0;

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

	int last_duration_;

	DurationTextShape(VideoWidget *widget) 
		: TextShape(theme_, widget)
		, bg_buf_(NULL)
		, fg_buf_(NULL) {
		no_value_ = false;

		last_duration_ = 0;
	}
};


/**
 * Widget definition
 */

class DurationWidget : public VideoWidget {
public:
	virtual ~DurationWidget() {
		delete shape_;
	}

	static DurationWidget * create(GPXApplication &app) {
		DurationWidget *widget;

		widget = new DurationWidget(app, "duration");

		return widget;
	}

	void setShape(VideoWidget::Shape type) {
		if (shape_)
			delete shape_;

		switch (type) {
		case VideoWidget::ShapeText:
		default:
			shape_ = DurationTextShape::create(this);
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

		os << "<text-shadow>" << textShadow() << "</text-shadow>" << std::endl;
	}

private:
	ShapeBase *shape_;

	DurationWidget(GPXApplication &app, std::string name)
		: VideoWidget(app, name) 
   		, shape_(NULL) {
		setShape(VideoWidget::ShapeText);
	}
};

#endif

