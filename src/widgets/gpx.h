#ifndef __GPX2VIDEO__WIDGETS__GPX_H__
#define __GPX2VIDEO__WIDGETS__GPX_H__

#include "../oiio.h"
#include "../shape/base.h"


/**
 * Widget definition
 */

class GPXWidget : public VideoWidget, public ShapeBase {
public:
	virtual ~GPXWidget() {
	}

	static GPXWidget * create(GPXApplication &app) {
		GPXWidget *widget;

		widget = new GPXWidget(app);

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
		this->drawBorder(bg_buf_);
		this->drawBackground(bg_buf_);

		is_update = true;
skip:
		return bg_buf_;
	}

	OIIO::ImageBuf * render(const TelemetryData &data, bool &is_update) {
		cairo_t *cairo;

		// Refresh dynamic info
		if (fg_buf_ != NULL) {
			if (data.type() == TelemetryData::TypeUnchanged) {
				is_update = false;
				goto skip;
			}
		}

		// Refresh dynamic info
		if (fg_buf_ != NULL)
			delete fg_buf_;

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

	void clear(void) {
		if (bg_buf_)
			delete bg_buf_;

		if (fg_buf_)
			delete fg_buf_;

		bg_buf_ = NULL;
		fg_buf_ = NULL;
	}

protected:
	void xmlwrite(std::ostream &os) {
		VideoWidget::xmlwrite(os);

		ShapeBase::xmlwrite(os);
	}

private:
	OIIO::ImageBuf *bg_buf_;
	OIIO::ImageBuf *fg_buf_;

	int padding_left_;
	int padding_right_;
	int padding_top_;
	int padding_bottom_;

	GPXWidget(GPXApplication &app);

	void initialize(void);
	void draw(cairo_t *cr, const TelemetryData &data);

	int label(cairo_t *cr, ShapeBase::Font &font, 
			const float *fill, const float *outline, const char *text);
	int value(cairo_t *cr, int y, ShapeBase::Font &font, 
			const float *fill, const float *outline, const char *text);
};

#endif

