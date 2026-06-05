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

	bool updated(const TelemetryData &data) const;
	void draw(cairo_t *cr, const TelemetryData &data);
	void clear(void);

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

	int label_x_, label_y_, label_width_, label_height_;
	int value_x_, value_y_, value_width_, value_height_;

	GPXWidget(GPXApplication &app);

	void initialize(cairo_t *cr);

	int label(cairo_t *cr, ShapeBase::Font &font, 
			const float *fill, const float *outline, const char *text);
	int value(cairo_t *cr, int y, ShapeBase::Font &font, 
			const float *fill, const float *outline, const char *text);
};

#endif

