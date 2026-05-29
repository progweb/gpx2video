#ifndef __GPX2VIDEO__WIDGETS__IMAGE_H__
#define __GPX2VIDEO__WIDGETS__IMAGE_H__

#include "../shape/base.h"


/**
 * Widget definition
 */

class ImageWidget : public VideoWidget, public ShapeBase {
public:
	virtual ~ImageWidget() {
	}

	static ImageWidget * create(GPXApplication &app) {
		ImageWidget *widget;

		widget = new ImageWidget(app);

		return widget;
	}

	ShapeBase * shape(void) {
		return this;
	}

	OIIO::ImageBuf * prepare(bool &is_update) {
//		if (bg_buf_ != NULL) {
//			is_update = false;
//			goto skip;
//		}
//
//		this->initialize();
//		this->createBox(&bg_buf_, theme().width(), theme().height());
//		this->drawImage(bg_buf_, theme().border(), theme().border(), this->source().c_str(), this->zoom());
//
//		is_update = true;
//skip:
//		return bg_buf_;

		is_update = false;

		this->initialize();

		return bg_buf_;
	}

	OIIO::ImageBuf * render(const TelemetryData &data, bool &is_update) {
		cairo_t *cairo;

		(void) data;

		// Refresh dynamic info
		if (fg_buf_ != NULL) {
			is_update = false;
			goto skip;
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

		// Append image
		this->drawImage(fg_buf_, theme().border(), theme().border(), this->source().c_str(), this->zoom());

		// Release
		this->destroyCairoContext(cairo);

		is_update = true;
skip:
		return fg_buf_;
	}

	bool updated(const TelemetryData &data) const;
	void draw(cairo_t *cr, const TelemetryData &data);

	void clear(void);

	bool isStatic(void) {
		return true;
	}

protected:
	void xmlwrite(std::ostream &os) {
		VideoWidget::xmlwrite(os);

		ShapeBase::xmlwrite(os);
	}


private:
	OIIO::ImageBuf *bg_buf_;
	OIIO::ImageBuf *fg_buf_;

	ImageWidget(GPXApplication &app);

	void initialize(void);
};

#endif

