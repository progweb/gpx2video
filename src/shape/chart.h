#ifndef __GPX2VIDEO__SHAPE__CHART_H__
#define __GPX2VIDEO__SHAPE__CHART_H__

#include "base.h"


class ChartShape : public ShapeBase {
public:
	struct point {
		double x;
		double y;
	};

	struct curve {
		cairo_t *cr;
		double width;
		double border;
		const float *color;
		const float *outline;
		const float *fill;
	};

	ChartShape(VideoWidget::Theme &theme, int width = 0, int height = 0)
		: ShapeBase(theme, VideoWidget::ShapeChart) {
		setSize(width, height);

		clear();
	}

	virtual ~ChartShape() {
		clear();
	}

	VideoWidget::Theme& theme(void) {
		return theme_;
	}

	int size(void) {
		return size_;
	}

	void setSize(int width, int height) {
		ShapeBase::setSize(width, height);

		size_ = std::min(width, height);
	}

	void setPadding(int left, int right, int top, int bottom) {
		padding_left_ = left;
		padding_right_ = right;
		padding_top_ = top;
		padding_bottom_ = bottom;
	}

	void getXRange(double &min, double &max) {
		min = x_min_;
		max = x_max_;
	}

	void setXRange(double min, double max) {
		x_min_ = min;
		x_max_ = max;
	}

	void getYRange(double &min, double &max) {
		min = y_min_;
		max = y_max_;
	}

	void setYRange(double min, double max) {
		y_min_ = min;
		y_max_ = max;
	}

	double scale(double min, double max, double value) {
		double v_point = (value - min) / (max - min);

		return v_point;
	}


	struct point locate(double x, double y) {
		double xsize = width_;
		double ysize = height_;

		// padding
		xsize -= (padding_left_ + padding_right_);
		ysize -= (padding_top_ + padding_bottom_);

		x = x * xsize;
		y = ysize - y * ysize;

		return {
			.x = padding_left_ + x,
			.y = padding_top_ + y
		};
	}

	void axis(cairo_t *cr, double width, double border, const float *fill, const float *outline);
	void cursor(cairo_t *cr, double x, double y, double width, const float *fill);
	void value(cairo_t *cr, double y, ChartShape::Font &font,
			const float *fill, const float *outline, const char *text);
	void value(cairo_t *cr, double x, double y, Font &font,
			const float *fill, const float *outline, const char *text);
	void unit(cairo_t *cr, ChartShape::Font &font,
			const float *fill, const float *outline, const char *text);
	void needle(cairo_t *cr, VideoWidget::Theme::NeedleType type,
		double x, double y, Font &font, const char *text,
		double width, double border, 
		const float *color, const float *fill, const float *outline);
	void icon(cairo_t *cr, double x, double y, const std::string &filename, const float *fill);

	void curve_create(struct curve **curvep, cairo_t *cr, double width, double border, 
			const float *color, const float *outline, const float *fill);
	void curve_point(struct curve *curve, double x, double y);
	void curve_stroke(struct curve *curve);
	void curve_free(struct curve *curve);

	void clear(void) {
		ShapeBase::clear();

		setPadding(0, 0, 0, 0);
	}

	void xmlwrite(std::ostream &os);

private:
	int size_;

	int padding_left_;
	int padding_right_;
	int padding_top_;
	int padding_bottom_;

	double x_min_, x_max_;
	double y_min_, y_max_;
};

#endif

