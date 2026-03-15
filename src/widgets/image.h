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

		widget = new ImageWidget(app, "image");

		return widget;
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
		this->drawImage(bg_buf_, theme().border(), theme().border(), this->source().c_str(), this->zoom());

		is_update = true;
skip:
		return bg_buf_;
	}

	OIIO::ImageBuf * render(const TelemetryData &data, bool &is_update) {
		(void) data;

		is_update = false;

		return NULL;
	}

	void clear(void) {
		if (bg_buf_)
			delete bg_buf_;

		bg_buf_ = NULL;
	}

	bool isStatic(void) {
		return true;
	}

protected:
	void xmlwrite(std::ostream &os) {
		VideoWidget::xmlwrite(os);

		IndentingOStreambuf indent(os, 4);

		ShapeBase::xmlwrite(os);
	}


private:
	OIIO::ImageBuf *bg_buf_;

	ImageWidget(GPXApplication &app, std::string name)
		: VideoWidget(app, name) 
		, ShapeBase(theme()) {
		setShape(VideoWidget::ShapeNone);
	}

	void initialize(void);
	void draw(cairo_t *cr, const TelemetryData &data);
};

#endif

