#ifndef __GPX2VIDEO__WIDGETS__DATE_H__
#define __GPX2VIDEO__WIDGETS__DATE_H__

#include "../shape/text.h"


/**
 * Date text & icon shape widget
 */

class DateTextShape : public TextShape {
public:
	virtual ~DateTextShape() {
		clear();
	}

	static DateTextShape * create(VideoWidget *widget) {
		DateTextShape *shape;

		shape = new DateTextShape(widget);
		
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
			this->drawImage(bg_buf_, widget_->border(), widget_->border(), "./assets/picto/DataOverlay_icn_date.png", VideoWidget::ZoomFit);
		this->drawLabel(bg_buf_, widget_->label().c_str());

		is_update = true;
skip:
		return bg_buf_;
	}

	OIIO::ImageBuf * render(const TelemetryData &data, bool &is_update) {
		char s[128];

		time_t t;
		struct tm time;

		// Compute time
		t = data.datetime() / 1000;

		// Refresh dynamic info
		if (fg_buf_ != NULL) {
			if (t == last_time_) {
				is_update = false;
				goto skip;
			}
		}

		// Format data
		// Don't use gps time, but camera time!
		// Indeed, with garmin devices, gpx time has an offset.
		localtime_r(&t, &time);

		strftime(s, sizeof(s), widget_->format().c_str(), &time);

		// Refresh dynamic info
		if (fg_buf_ != NULL)
			delete fg_buf_;

		this->createBox(&fg_buf_, widget_->width(), widget_->height());
		this->drawValue(fg_buf_, s);

		is_update = true;
		last_time_ = t;

skip:
		return fg_buf_;
	}

	void clear(void) {
		if (bg_buf_)
			delete bg_buf_;

		if (fg_buf_)
			delete fg_buf_;

		bg_buf_ = NULL;
		fg_buf_ = NULL;
	}

private:
	Theme theme_;

	time_t last_time_;

	OIIO::ImageBuf *bg_buf_;
	OIIO::ImageBuf *fg_buf_;

	DateTextShape(VideoWidget *widget) 
		: TextShape(theme_, widget)
		, bg_buf_(NULL)
		, fg_buf_(NULL) {
		last_time_ = 0;
	}
};


/**
 * Widget definition
 */

class DateWidget : public VideoWidget {
public:
	virtual ~DateWidget() {
		delete shape_;
	}

	static DateWidget * create(GPXApplication &app) {
		DateWidget *widget;

		widget = new DateWidget(app, "date");

		widget->setFormat("%Y-%m-%d");

		return widget;
	}

	void setShape(VideoWidget::Shape type) {
		if (shape_)
			delete shape_;

		switch (type) {
		case VideoWidget::ShapeText:
		default:
			shape_ = DateTextShape::create(this);
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

	DateWidget(GPXApplication &app, std::string name)
		: VideoWidget(app, name) 
   		, shape_(NULL) {
		setShape(VideoWidget::ShapeText);
	}
};

#endif

