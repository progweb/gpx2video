#ifndef __GPX2VIDEO__WIDGETS__SPEED_H__
#define __GPX2VIDEO__WIDGETS__SPEED_H__

#include "../shape/arc.h"
#include "../shape/text.h"


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
			this->drawImage(bg_buf_, widget_->border(), widget_->border(), "./assets/picto/DataOverlay_icn_speed.png", VideoWidget::ZoomFit);
		this->drawLabel(bg_buf_, widget_->label().c_str());

		is_update = true;
skip:
		return bg_buf_;
	}

	OIIO::ImageBuf * render(const TelemetryData &data, bool &is_update) {
		char s[128];
		bool pace_unit = false;
		double speed = data.speed();

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

		if (no_value_)
			sprintf(s, "-- %s", widget_->unit2string(widget_->unit()).c_str());
		else if (pace_unit) {
			double pace = 60.0 / speed;
			int min = (int) pace;
			int sec = (int) round((pace - min) * 60) % 60;

			sprintf(s, "%d:%02d %s", min, sec, widget_->unit2string(widget_->unit()).c_str());
		} 
		else
			sprintf(s, "%.1f %s", speed, widget_->unit2string(widget_->unit()).c_str());

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

	SpeedTextShape(VideoWidget *widget) 
		: TextShape(theme_, widget)
		, bg_buf_(NULL)
		, fg_buf_(NULL) {
		no_value_ = false;
	}
};


/**
 * Speed arc shape widget
 */

class SpeedArcShape : public ArcShape {
public:
	class Theme : public ArcShape::Theme {
	public:
		Theme() : ArcShape::Theme() {
			enableNeedle(true);
			setNeedleColor(1.0, 1.0, 1.0, 1.0);

			enableValue(true);
		}

		const bool& withNeedle(void) const {
			return with_needle_;
		}

		void enableNeedle(bool enable) {
			with_needle_ = enable;
		}

		const float * needleColor(void) const {
			return needle_color_;
		}

		void setNeedleColor(double r, double g, double b, double a) {
			needle_color_[0] = r;
			needle_color_[1] = g;
			needle_color_[2] = b;
			needle_color_[3] = a;
		}

		const bool& withValue(void) const {
			return with_value_;
		}

		void enableValue(bool enable) {
			with_value_ = enable;
		}

	private:
		bool with_needle_;
		float needle_color_[4];

		bool with_value_;
	};

	virtual ~SpeedArcShape() {
		clear();
	}

	static SpeedArcShape * create(VideoWidget *widget) {
		SpeedArcShape *shape;

		cairo_font_face_t *fontface;

		fontface = cairo_toy_font_face_create(
			"Roboto", 
			CAIRO_FONT_SLANT_NORMAL, 
			CAIRO_FONT_WEIGHT_NORMAL
		);

		shape = new SpeedArcShape(widget, fontface);

		shape->initialize();

		return shape;
	}

	Theme& theme(void) {
		return theme_;
	}

	void initialize(void) {
		int n;

		double range;

//		double start = 105; //30 - 330
//		double end = 255;

		arc_start_ = 30;
		arc_end_ = 330;

		size_ = widget_->width();

//		arc_start_ = 150;
//		arc_end_ = 290;

		range = arc_end_ - arc_start_;

		init(fontface_, size_, arc_start_, arc_end_);

		// Compute arc step
		// n markers for range [start:end] (ex. 12 [30:330]
		n = 12 * range / 300.0;

		arc_step_ = ((_vmax - _vmin) / n) / 4;
		arc_step_ = std::ceil(arc_step_ / 5.0) * 5;
		arc_step_ = std::max(1, arc_step_);

		if (((_vmax - _vmin) / arc_step_) < (10 * range / 300.0))
			arc_mstep_ = 1;
		else if (((_vmax - _vmin) / arc_step_) < (30 * range / 300.0))
			arc_mstep_ = 2;
		else
			arc_mstep_ = 4;
	}

	OIIO::ImageBuf * prepare(bool &is_update) {
		is_update = false;

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
		this->createBox(&fg_buf_, widget_->width(), widget_->height());

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

	Theme theme_;

	cairo_font_face_t *fontface_;

	int size_;

	double arc_start_;
	double arc_end_;

	int arc_step_;
	int arc_mstep_;

	OIIO::ImageBuf *bg_buf_;
	OIIO::ImageBuf *fg_buf_;

	int _vmin, _vmax;

	SpeedArcShape(VideoWidget *widget, cairo_font_face_t *fontface) 
		: ArcShape(theme_, widget, fontface)
		, fontface_(fontface)
		, bg_buf_(NULL)
		, fg_buf_(NULL) {
		no_value_ = false;

		_vmin = 0;
		_vmax = 120;

		cairo_font_face_reference(fontface);
	}

	void ticklenwidth(int value, int *len, int *width) {
		if (value % 10 == 0) {
			*len = size_ / 15 + size_ / 51;
			*width = size_ / 128;
		}
		else {
			*len = size_ / 15 - size_ / 51;
			*width = size_ / 256;
		}
	}

	void draw(cairo_t *cr, const TelemetryData &data) {
		int rotate = 180;

		double xa1, xa2;

		bool pace_unit = false;

		double speed = data.speed();
		double avgspeed = data.avgridespeed();
		double maxspeed = data.maxspeed();

		switch (widget_->unit()) {
		case VideoWidget::UnitMPH:
			speed *= 0.6213711922;
			avgspeed *= 0.6213711922;
			maxspeed *= 0.6213711922;
			break;
		case VideoWidget::UnitKPH:
			break;
		case VideoWidget::UnitMPM:
			speed *= 0.6213711922;
			avgspeed *= 0.6213711922;
			maxspeed *= 0.6213711922;
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

		// Draw background
		pieslice(cr, 0, 360, 0);

		// Draw color arc (avg speed)
		xa1 = scale(_vmin, _vmax, 0, rotate);
		xa2 = scale(_vmin, _vmax, avgspeed, rotate);

		arc(cr, 0, xa1, xa2, size_ / 150.0 + size_ / 33.0, size_ / 21.0, 0.0);

		// Draw color arc (max speed)
		xa1 = scale(_vmin, _vmax, 0, rotate);
		xa2 = scale(_vmin, _vmax, maxspeed, rotate);

		arc(cr, 1, xa1, xa2, size_ / 150.0, size_ / 33.0, 0.0);

		// Draw tick lines around arc line
		for (int value = _vmin; value < _vmax + arc_step_; value = value + arc_step_) {
			int ticklen;
			int tickwidth;

			double xa = scale(_vmin, _vmax, value, rotate);

			if (xa > (arc_end_ + rotate))
				break;

			ticklenwidth(value, &ticklen, &tickwidth);

			line(cr, xa, 0, ticklen);
		}

		// Draw arc label
		for (int value = _vmin; value < _vmax + (arc_mstep_ * arc_step_); value = value + (arc_mstep_ * arc_step_)) {
			double xa = scale(_vmin, _vmax, value, rotate);

			if (xa > (arc_end_ + rotate))
				break;

			std::string str = std::to_string(value);

			text(cr, xa, size_ / 6.5, str);
		}

		// Draw needle
		if (theme().withNeedle()) {
			double xa = scale(_vmin, _vmax, speed, rotate);

			struct ArcShape::point p;
		   
			const float *fill = theme().needleColor();

			cairo_save(cr);
			cairo_set_source_rgba(cr, fill[0], fill[1], fill[2], fill[3]);

			p = locate(xa - 0, 0);
			cairo_move_to(cr, p.x, p.y);

			p = locate(xa - 90, (size_ / 2) - 8);
			cairo_line_to(cr, p.x, p.y);

			p = locate(xa - 180, (size_ / 2) - 8);
			cairo_line_to(cr, p.x, p.y);

			p = locate(xa + 90, (size_ / 2) - 8);
			cairo_line_to(cr, p.x, p.y);

			cairo_close_path(cr);
			cairo_fill_preserve(cr);
			cairo_stroke(cr);

			cairo_restore(cr);
		}

		// Write speed
		if (theme().withValue()) {
			char value[128];

			cairo_text_extents_t extents;

			std::string unit = widget_->unit2string(widget_->unit());

			if (no_value_)
				sprintf(value, "--");
			else if (pace_unit) {
				double pace = 60.0 / speed;
				int min = (int) pace;
				int sec = (int) round((pace - min) * 60) % 60;

				sprintf(value, "%d:%02d", min, sec);
			} 
			else
				sprintf(value, "%d", int(std::round(speed)));

			cairo_save(cr);
			cairo_set_font_face(cr, fontface_);
			cairo_set_font_size(cr, size_ / 10);
			cairo_text_extents(cr, value, &extents);
			cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
			cairo_move_to(cr, size_ / 2.0 - (extents.width / 2), size_ - (extents.height / 2) - (size_ / 20.0));
			cairo_show_text(cr, value);
			cairo_stroke(cr);
			cairo_set_font_size(cr, size_ / 20);
			cairo_text_extents(cr, unit.c_str(), &extents);
			cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
			cairo_move_to(cr, size_ / 2.0 - (extents.width / 2), size_ - (extents.height / 2));
			cairo_show_text(cr, unit.c_str());
			cairo_stroke(cr);
			cairo_restore(cr);
		}
	}
};


/**
 * Widget definition
 */

class SpeedWidget : public VideoWidget {
public:
	virtual ~SpeedWidget() {
		delete shape_;
	}

	static SpeedWidget * create(GPXApplication &app) {
		SpeedWidget *widget;

		widget = new SpeedWidget(app, "speed");

		widget->setUnit(VideoWidget::UnitMPH);

		return widget;
	}
	
	void setShape(VideoWidget::Shape type) {
		if (shape_)
			delete shape_;

		switch (type) {
		case VideoWidget::ShapeArc:
			shape_ = SpeedArcShape::create(this);
			break;

		case VideoWidget::ShapeText:
		default:
			shape_ = SpeedTextShape::create(this);
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

	SpeedWidget(GPXApplication &app, std::string name)
		: VideoWidget(app, name) 
		, shape_(NULL) {
		setShape(VideoWidget::ShapeArc);
	}
};

#endif

