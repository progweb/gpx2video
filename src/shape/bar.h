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

	BarShape(VideoWidget::Theme &theme, cairo_font_face_t *fontface = NULL, 
			int width = 0, int height = 0)
		: ShapeBase(theme) {
		init(fontface, width, height);

		setPadding(0, 0, 0, 0);
	}

	virtual ~BarShape() {
		clear();
	}

	VideoWidget::Theme& theme(void) {
		return theme_;
	}

	void init(cairo_font_face_t *fontface, int width, int height, int size = 0) {
		setOffset(0);
		setSize(width, height, size);

		fontface_ = fontface;
	}

	void setSize(int width, int height, int size) {
		// Vertical
		size_ = size;
		length_ = height;

		center_ = width / 2;
//		margin_ = size / 10;
	}

	void setOffset(int offset) {
		offset_ = offset;
	}

	void setPadding(int left, int right, int top, int bottom) {
		padding_left_ = left;
		padding_right_ = right;
		padding_top_ = top;
		padding_bottom_ = bottom;
	}

	double scale(double min, double max, double value, int rotate = 0) {
		double v_point = (value - min) / (max - min);

		(void) rotate;

		return v_point;
	}

	struct point locate(double value, double distance) {
//		double size = length_ - 3.0 * margin_ - (size_ / 9);
		double size = length_ - padding_top_ - padding_bottom_; // - (size_ / 9);

		return {
			.x = center_ + offset_ + distance,
//			.y = size + margin_ - value * size 
			.y = size + padding_top_ - value * size 
		};
	}

	virtual OIIO::ImageBuf * prepare(bool &is_update) = 0;
	virtual OIIO::ImageBuf * render(const TelemetryData &data, bool &is_update) = 0;

	void bar(cairo_t *cr, double v1, double v2, double width, double border,
			const float *fill, const float *outline = NULL);
	void cursor(cairo_t *cr, double v, double width);
	void line(cairo_t *cr, double v, double d1, double d2, const float *fill);
	void text(cairo_t *cr, double v, double d, int size, const float *color, std::string str);
	void marker(cairo_t *cr, double v, 
			int border, const float *color, const float *fill, const float *outline, std::string str);

	void label(cairo_t *cr, int x, int y, const float *color, std::string str);

	void xmlwrite(std::ostream &os);

private:
	int size_;
	int length_;
//	int margin_;
	int offset_;

	int padding_left_;
	int padding_right_;
	int padding_top_;
	int padding_bottom_;

	cairo_font_face_t *fontface_;
};

#endif

