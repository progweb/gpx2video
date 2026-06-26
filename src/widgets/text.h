#ifndef __GPX2VIDEO__WIDGETS__TEXT_H__
#define __GPX2VIDEO__WIDGETS__TEXT_H__

#include <pango/pangocairo.h>

#include "../oiio.h"
#include "../shape/text.h"


/**
 * Widget definition
 */

class TextWidget : public VideoWidget, public TextShape {
public:
	virtual ~TextWidget() {
	}

	static TextWidget * create(GPXApplication &app, TelemetrySource *source = NULL) {
		TextWidget *widget;

		widget = new TextWidget(app, source);

		return widget;
	}

	ShapeBase * shape(void) {
		return this;
	}

	VideoWidget::Theme& theme(void) {
		return VideoWidget::theme();
	}

	bool hasFeature(ShapeBase::Feature feature) const {
		switch (feature) {
		case FeatureUnit:
			return false;

		default:
			break;
		}

		return TextShape::hasFeature(feature);
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

	std::string icon_filename_;

	TextWidget(GPXApplication &app, TelemetrySource *source);

	void initialize(cairo_t *cr);
};

#endif

