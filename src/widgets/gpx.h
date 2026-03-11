#ifndef __GPX2VIDEO__WIDGETS__GPX_H__
#define __GPX2VIDEO__WIDGETS__GPX_H__

#include "../shape/text.h"


/**
 * GPX text & icon shape widget
 */

class GPXTextShape : public TextShape {
public:
	virtual ~GPXTextShape() {
		clear();
	}

	static GPXTextShape * create(VideoWidget *widget) {
		GPXTextShape *shape;

		shape = new GPXTextShape(widget);
		
		shape->initialize();

		return shape;
	}

	OIIO::ImageBuf * prepare(bool &is_update) {
		if (bg_buf_ != NULL) {
			is_update = false;
			goto skip;
		}

		this->createBox(&bg_buf_, widget_->width(), widget_->height());
		this->drawBorder(bg_buf_);
		this->drawBackground(bg_buf_);

		is_update = true;
skip:
		return bg_buf_;
	}

	OIIO::ImageBuf * render(const TelemetryData &data, bool &is_update) {
		char s[128];

		time_t t;
		struct tm time;

		int h;
		int offset;

		int border = widget_->border();
		int padding_yt = widget_->padding(VideoWidget::PaddingTop);
		int padding_yb = widget_->padding(VideoWidget::PaddingBottom);

		// width x height
//		w = widget_->height() - 2 * border;
		h = widget_->height() - 2 * border;

		// Add text (1 pt = 1.333 px)
		// +-------------
		// |  Text    px
		// |  Text    px
		// |  ....    ..
		// +-------------
		//          (with n nbr of lines)
		//        h = n * px + padding_top + padding_bottom
		int px = (h - padding_yt - padding_yb) / 6;

		// Refresh dynamic info
		if ((fg_buf_ != NULL) && (data.type() == TelemetryData::TypeUnchanged)) {
			is_update = true;
			goto skip;
		}

		// Refresh dynamic info
		if (fg_buf_ != NULL)
			delete fg_buf_;

		this->createBox(&fg_buf_, widget_->width(), widget_->height());

		// title
		offset = 0;
		this->drawText(fg_buf_, 0, offset, px, "GPX WPT");

		// time
		offset += px;
		t = data.timestamp() / 1000;
		gmtime_r(&t, &time);
		strftime(s, sizeof(s), "time: %H:%M:%S", &time);
		this->drawText(fg_buf_, 0, offset, px, s);

		// latitude
		offset += px;
		sprintf(s, "lat: %.4f", data.latitude());
		this->drawText(fg_buf_, 0, offset, px, s);

		// longitude
		offset += px;
		sprintf(s, "lon: %.4f", data.longitude());
		this->drawText(fg_buf_, 0, offset, px, s);

		// elevation
		offset += px;
		sprintf(s, "ele: %.4f", data.elevation());
		this->drawText(fg_buf_, 0, offset, px, s);

		// line
		offset += px;
		sprintf(s, "line: %d", data.line());
		this->drawText(fg_buf_, 0, offset, px, s);

		is_update = true;
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

	OIIO::ImageBuf *bg_buf_;
	OIIO::ImageBuf *fg_buf_;

	GPXTextShape(VideoWidget *widget) 
		: TextShape(theme_, widget)
		, bg_buf_(NULL)
		, fg_buf_(NULL) {
	}
};


/**
 * Widget definition
 */

class GPXWidget : public VideoWidget {
public:
	virtual ~GPXWidget() {
		delete shape_;
	}

	static GPXWidget * create(GPXApplication &app) {
		GPXWidget *widget;

		widget = new GPXWidget(app, "gpx");

		return widget;
	}

	void setShape(VideoWidget::Shape type) {
		if (shape_)
			delete shape_;

		switch (type) {
		case VideoWidget::ShapeText:
		default:
			shape_ = GPXTextShape::create(this);
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

	GPXWidget(GPXApplication &app, std::string name)
		: VideoWidget(app, name)
   		, shape_(NULL) {
		setShape(VideoWidget::ShapeText);
	}
};

#endif

