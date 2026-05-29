#ifndef __GPX2VIDEO__WIDGETS__TEXT_H__
#define __GPX2VIDEO__WIDGETS__TEXT_H__

#include <pango/pangocairo.h>

#include "../oiio.h"
#include "../shape/base.h"


/**
 * Widget definition
 */

class TextWidget : public VideoWidget, public ShapeBase {
public:
	virtual ~TextWidget() {
	}

	static TextWidget * create(GPXApplication &app) {
		TextWidget *widget;

		widget = new TextWidget(app);

		return widget;
	}

	ShapeBase * shape(void) {
		return this;
	}

	void setPadding(int left, int right, int top, int bottom) {
		padding_left_ = left;
		padding_right_ = right;
		padding_top_ = top;
		padding_bottom_ = bottom;
	}

	OIIO::ImageBuf * prepare(bool &is_update) {
		if (bg_buf_ != NULL) {
			is_update = false;
			goto skip;
		}

		this->initialize();
		this->createBox(&bg_buf_, theme().width(), theme().height());

		is_update = true;
skip:
		return bg_buf_;
	}

	OIIO::ImageBuf * render(const TelemetryData &data, bool &is_update) {
		cairo_t *cairo;

		if (fg_buf_ != NULL) {
			is_update = false;
			goto skip;
		}

		// Image buffer
		this->createBox(&fg_buf_, theme().width(), theme().height());

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

	bool updated(const TelemetryData &data) const;

	void draw(cairo_t *cairo, const TelemetryData &data);

	void clear(void);

	bool isStatic(void) {
		return true;
	}

protected:
	void xmlwrite(std::ostream &os) {
		VideoWidget::xmlwrite(os);

		ShapeBase::xmlwrite(os);

		os << "<text>" << VideoWidget::value() << "</text>" << std::endl;
	}

private:
	OIIO::ImageBuf *bg_buf_;
	OIIO::ImageBuf *fg_buf_;

	int padding_left_;
	int padding_right_;
	int padding_top_;
	int padding_bottom_;

	TextWidget(GPXApplication &app);

	void initialize(void);

	void label(cairo_t *cr, ShapeBase::Font &font, 
			const float *fill, const float *outline, const char *text);

	void value(cairo_t *cr, ShapeBase::Font &font, 
			const float *fill, const float *outline, const char *text);
};

#endif

