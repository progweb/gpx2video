#ifndef __GPX2VIDEO__SHAPE__ARC_H__
#define __GPX2VIDEO__SHAPE__ARC_H__

#include "base.h"


class ArcShape : public ShapeBase {
public:
	int center_;
	int correction_;

	double start_angle_;
	double end_angle_;

	struct point {
		double x;
		double y;
	};

	class Theme : public ShapeBase::Theme {
	public:
		Theme() : ShapeBase::Theme() {
			setArcColor(0, 0.0, 0.8, 0.0, 0.8);
			setArcColor(1, 1.0, 0.0, 0.0, 1.0);
			setTickColor(1.0, 1.0, 1.0, 1.0);
			setTextColor(1.0, 1.0, 1.0, 1.0);
		}

		const float * arcColor(int index) const {
			return arc_color_[index];
		}

		void setArcColor(int index, double r, double g, double b, double a) {
			arc_color_[index][0] = r;
			arc_color_[index][1] = g;
			arc_color_[index][2] = b;
			arc_color_[index][3] = a;
		}

		const float * tickColor(void) const {
			return tick_color_;
		}

		void setTickColor(double r, double g, double b, double a) {
			tick_color_[0] = r;
			tick_color_[1] = g;
			tick_color_[2] = b;
			tick_color_[3] = a;
		}

		const float * textColor(void) const {
			return text_color_;
		}

		void setTextColor(double r, double g, double b, double a) {
			text_color_[0] = r;
			text_color_[1] = g;
			text_color_[2] = b;
			text_color_[3] = a;
		}

	private:
		float arc_color_[2][4];
		float tick_color_[4];
		float text_color_[4];
	};

	ArcShape(Theme &theme, VideoWidget *widget, cairo_font_face_t *fontface = NULL, int size = 0,
			double start = 30.0, double end = 360.0 - 30.0)
		: ShapeBase(theme, widget)
		, theme_(theme) {
		correction_ = 90;

		init(fontface, size, start, end);
	}

	virtual ~ArcShape() {
		clear();
	}

	Theme& theme(void) {
		return theme_;
	}

	virtual OIIO::ImageBuf * prepare(bool &is_update) = 0;
	virtual OIIO::ImageBuf * render(const TelemetryData &data, bool &is_update) = 0;

	void init(cairo_font_face_t *fontface, int size, 
			double start = 30.0, double end = 360.0 - 30.0) {
		setSize(size, start, end);

		fontface_ = fontface;
	}

	void setSize(int size,
			double start = 30.0, double end = 360.0 - 30.0) {
		size_ = size;

		start_angle_ = start;
		end_angle_ = end; //360.0 - _start_angle;

		center_ = size / 2;
	}

	double scale(double min, double max, double value, int rotate = 0) {
		double a_range = end_angle_ - start_angle_;

		double v_point = (value - min) / (max - min);

		double a_point = a_range * v_point;

		return a_point + start_angle_ + rotate;
	}

#define DEG2RAD(a) ((a) * M_PI / 180.0)

	struct point locate(double angle, double r_delta) {
		return {
			.x = center_ + ((center_ - r_delta) * sin(DEG2RAD(angle))),
			.y = center_ - ((center_ - r_delta) * cos(DEG2RAD(angle)))
		};
	}

	void arc(cairo_t *cr, int index, double a1, double a2, double offset, double width, double border);
	void pieslice(cairo_t *cr, double a1, double a2, double border);
	void line(cairo_t *cr, double a, double d1, double d2);
	void text(cairo_t *cr, double a, double d, std::string str);

private:
	int size_;

	Theme &theme_;

	cairo_font_face_t *fontface_;
};

#endif

