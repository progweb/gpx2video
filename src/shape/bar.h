#ifndef __GPX2VIDEO__SHAPE__BAR_H__
#define __GPX2VIDEO__SHAPE__BAR_H__

#include "base.h"


class BarShape : public ShapeBase {
public:
	int center_;

	struct point {
		double x;
		double y;
	};

	BarShape(VideoWidget::Theme &theme, int width = 0, int height = 0)
		: ShapeBase(theme, VideoWidget::ShapeBar) {
		setOrientation(VideoWidget::OrientationVertical);
		setSize(width, height);

		clear();
	}

	virtual ~BarShape() {
		clear();
	}

	VideoWidget::Theme& theme(void) {
		return theme_;
	}

	int size(void) {
		return size_;
	}

	void setSize(int width, int height) {
		width_ = width;
		height_ = height;

		size_ = std::min(width_, height_);

		if (orientation_ == VideoWidget::OrientationVertical) {
			// Vertical
			length_ = height;

			center_ = width / 2;
		}
		else {
			// Horizontal
			length_ = width;

			center_ = height / 2;
		}
	}

	void setOffset(int offset) {
		offset_ = offset;
	}

	void setOrientation(VideoWidget::Orientation orientation) {
		orientation_ = orientation;

		setSize(width_, height_);
	}

	void setPadding(int left, int right, int top, int bottom) {
		padding_left_ = left;
		padding_right_ = right;
		padding_top_ = top;
		padding_bottom_ = bottom;
	}

	void setLabelExtents(int x, int y, int width, int height) {
		label_x_ = x;
		label_y_ = y;

		label_width_ = width;
		label_height_ = height;
	}

	double scale(double min, double max, double value, int rotate = 0) {
		double v_point = (value - min) / (max - min);

		(void) rotate;

		return v_point;
	}

	struct point locate(double value, double distance) {
		double x, y;

		double size = length_;

		bool flip = theme().gaugeFlip();
	   
		if (orientation_ == VideoWidget::OrientationVertical) {
			size -= (padding_top_ + padding_bottom_);

			y = flip ? value * size : size - value * size;

			return {
				.x = center_ + offset_ + distance,
				.y = padding_top_ + y
			};
		}
		else {
			size -= (padding_left_ + padding_right_);

			x = flip ? size - value * size : value * size;

			return {
				.x = padding_left_ + x,
				.y = center_ + offset_ + distance
			};
		}
	}

	void bar(cairo_t *cr, double v1, double v2, double width, double border,
			const float *fill, const float *outline = NULL);
	void cursor(cairo_t *cr, double v, double width, const float *fill);
	void line(cairo_t *cr, double v, double d1, double d2, double width, const float *fill);
	void ticklabel(cairo_t *cr, double v, double d, Font &font, const float *fill, const float *outline, const char *text);
//	void text(cairo_t *cr, double v, double d, int size, const float *color, std::string str);

	void label(cairo_t *cr, Font &font, 
			const float *fill, const float *outline, const char *text);
	void value(cairo_t *cr, double v, Font &font,
			const float *fill, const float *outline, const char *text);

	void clear(void) {
		ShapeBase::clear();

		setOffset(0);
		setPadding(0, 0, 0, 0);
		setLabelExtents(0, 0, 0, 0);
	}

	void xmlwrite(std::ostream &os);

private:
	int size_;
	int length_;
//	int margin_;
	int offset_;

	int width_;
	int height_;

	VideoWidget::Orientation orientation_;

	int padding_left_;
	int padding_right_;
	int padding_top_;
	int padding_bottom_;

	int label_x_, label_y_, label_width_, label_height_;
};

#endif

