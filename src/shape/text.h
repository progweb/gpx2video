#ifndef __GPX2VIDEO__SHAPE__TEXT_H__
#define __GPX2VIDEO__SHAPE__TEXT_H__

#include "base.h"


class TextShape : public ShapeBase {
public:
	TextShape(VideoWidget::Theme &theme) 
		: ShapeBase(theme) {
		setSize(0);
		setPadding(0, 0, 0, 0);
	}

	virtual ~TextShape() {
		clear();
	}

	VideoWidget::Theme& theme(void) {
		return theme_;
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

	virtual OIIO::ImageBuf * prepare(bool &is_update) = 0;
	virtual OIIO::ImageBuf * render(const TelemetryData &data, bool &is_update) = 0;

//	void initialize(void);
//	void drawLabel(OIIO::ImageBuf *buf, const char *label);
//	void drawValue(OIIO::ImageBuf *buf, const char *value);

	void icon(cairo_t *cr, const char *filename);
	void label(cairo_t *cr, TextShape::Font &font, 
			const float *fill, const float *outline, const char *text);
	void value(cairo_t *cr, TextShape::Font &font, 
			const float *fill, const float *outline, const char *text);

	void xmlwrite(std::ostream &os);

private:
	int size_;

	int padding_left_;
	int padding_right_;
	int padding_top_;
	int padding_bottom_;

//	int label_px_;
//	int label_size_;
//	int value_px_;
//	int value_size_;
//	int value_offset_;
//	std::string text_;
};

#endif

