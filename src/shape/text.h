#ifndef __GPX2VIDEO__SHAPE__TEXT_H__
#define __GPX2VIDEO__SHAPE__TEXT_H__

#include "base.h"


class TextShape : public ShapeBase {
public:
	TextShape(Theme &theme, VideoWidget *widget) 
		: ShapeBase(theme, widget) {
	}

	virtual ~TextShape() {
		clear();
	}

	virtual void initialize(void);

	virtual OIIO::ImageBuf * prepare(bool &is_update) = 0;
	virtual OIIO::ImageBuf * render(const TelemetryData &data, bool &is_update) = 0;

	void drawLabel(OIIO::ImageBuf *buf, const char *label);
	void drawValue(OIIO::ImageBuf *buf, const char *value);

private:
	int label_px_;
	int label_size_;
	int value_px_;
	int value_size_;
	int value_offset_;
	std::string text_;
};

#endif

