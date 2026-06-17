#ifndef __GPX2VIDEO__WIDGETS__IMAGE_H__
#define __GPX2VIDEO__WIDGETS__IMAGE_H__

#include "../shape/base.h"


/**
 * Widget definition
 */

class ImageWidget : public VideoWidget, public ShapeBase {
public:
	virtual ~ImageWidget() {
		if (mask_ != NULL)
			delete mask_;

		if (fg_buf_ != NULL)
			delete fg_buf_;
		if (bg_buf_ != NULL)
			delete bg_buf_;
	}

	static ImageWidget * create(GPXApplication &app) {
		ImageWidget *widget;

		widget = new ImageWidget(app);

		return widget;
	}

	ShapeBase * shape(void) {
		return this;
	}

	bool hasFeature(ShapeBase::Feature feature) const {
		switch (feature) {
		case FeatureRoundCorner:
			return true;

		default:
			break;
		}

		return false;
	}

	OIIO::ImageBuf * render(const TelemetryData &data, bool &is_update) {
		cairo_t *cairo;

		OIIO::ImageBuf *img;

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

		// Draw image
		this->createBox(&img, theme().width(), theme().height());
		this->drawImage(img, theme().border(), theme().border(), this->source().c_str(), this->zoom());

		// Apply mask
		if (mask_ != NULL)
	    	OIIO::ImageBufAlgo::mul(*img, *img, *mask_);

		// Append image
		OIIO::ImageBufAlgo::over(*fg_buf_, *img, *fg_buf_, OIIO::ROI::All());

		// Release
		this->destroyCairoContext(cairo);

		delete img;

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

		os << "<zoom>" << VideoWidget::zoom2string(zoom()) << "</zoom>" << std::endl;
		os << "<source>" << source() << "</source>" << std::endl;
	}


private:
	OIIO::ImageBuf *bg_buf_;
	OIIO::ImageBuf *fg_buf_;
	OIIO::ImageBuf *mask_;

	ImageWidget(GPXApplication &app);

	void initialize(cairo_t *cr);
};

#endif

