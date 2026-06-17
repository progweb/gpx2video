#ifndef __GPX2VIDEO__SHAPE__TEXT_H__
#define __GPX2VIDEO__SHAPE__TEXT_H__

#include "base.h"


class TextShape : public ShapeBase {
public:
	TextShape(VideoWidget::Theme &theme) 
		: ShapeBase(theme, VideoWidget::ShapeText) {
		setSize(0);

		clear();
	}

	virtual ~TextShape() {
		clear();
	}

	VideoWidget::Theme& theme(void) {
		return theme_;
	}

	virtual bool hasFeature(Feature feature) const {
		switch (feature) {
		case FeatureNeedle:
			return false;

		default:
			break;
		}

		return true;
	}

	void setSize(int size) {
		size_ = size;
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

	void setValueExtents(int x, int y, int width, int height) {
		value_x_ = x;
		value_y_ = y;

		value_width_ = width;
		value_height_ = height;
	}

	virtual OIIO::ImageBuf * render(const TelemetryData &data, bool &is_update) = 0;

//	void initialize(void);
//	void drawLabel(OIIO::ImageBuf *buf, const char *label);
//	void drawValue(OIIO::ImageBuf *buf, const char *value);

	void icon(cairo_t *cr, const std::string &filename, const float *fill);
	void label(cairo_t *cr, TextShape::Font &font, 
			const float *fill, const float *outline, const char *text);
	void value(cairo_t *cr, TextShape::Font &font, 
			const float *fill, const float *outline, const char *text);

	void clear(void) {
		ShapeBase::clear();

		setPadding(0, 0, 0, 0);
		setLabelExtents(0, 0, 0, 0);
		setValueExtents(0, 0, 0, 0);
	}

	void xmlwrite(std::ostream &os);

private:
	int size_;

	int padding_left_;
	int padding_right_;
	int padding_top_;
	int padding_bottom_;

	int label_x_, label_y_, label_width_, label_height_;
	int value_x_, value_y_, value_width_, value_height_;

//	int label_px_;
//	int label_size_;
//	int value_px_;
//	int value_size_;
//	int value_offset_;
//	std::string text_;
};

#endif

