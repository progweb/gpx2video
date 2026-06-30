#ifndef __GPX2VIDEO__SHAPE__BASE_H__
#define __GPX2VIDEO__SHAPE__BASE_H__

#include <pango/pangocairo.h>

#include "../utils.h"
#include "../oiio.h"
#include "../oiioutils.h"
#include "../telemetrydata.h"
#include "../videowidget.h"


class ShapeBase {
public:
	enum Feature {
		FeatureLabel,
		FeatureValue,
		FeatureUnit,
		FeatureGauge,
		FeatureNeedle,
		FeatureRoundCorner,
		FeatureUnknown
	};

	enum TextType {
		TextAlpha,
		TextNumeric,
		TextMultiLine
	};

	struct Font {
		double size;
		double border;
		int shadow_opacity;
		double shadow_distance;
		std::string family;
		VideoWidget::Theme::Align align;
		VideoWidget::Theme::FontStyle style;
		VideoWidget::Theme::FontWeight weight;
	};

	virtual ~ShapeBase() {
		clear();
	}
	
	VideoWidget::Shape type(void) const {
		return type_;
	}

	virtual bool hasFeature(Feature feature) const {
		(void) feature;

		return true;
	}

	virtual OIIO::ImageBuf * render(const TelemetryData &data, bool &is_update) = 0;

	virtual bool updated(const TelemetryData &data) const = 0;

	virtual void initialize(cairo_t *cr) {
		(void) cr;

		is_initialized_ = true;
	}

	virtual void setSize(int width, int height) {
		if (height > width) {
			size_factor_ = width;
			fontsize_factor_ = height / 2.0;
		}
		else {
			size_factor_ = height;
			fontsize_factor_ = height;
		}

		size_factor_ /= 100.0;
		fontsize_factor_ /= 100.0;

		width_ = width;
		height_ = height;
	}

	virtual void draw(cairo_t *cairo, const TelemetryData &data) = 0;

	virtual void clear(void) {
		if (surface_ != NULL)
			cairo_surface_destroy(surface_);

		surface_ = NULL;

		is_initialized_ = false;
	}

	virtual void xmlwrite(std::ostream &os);

protected:
	VideoWidget::Theme &theme_;

	VideoWidget::Shape type_;

	bool is_initialized_;

	int width_;
	int height_;

	ShapeBase(VideoWidget::Theme &theme, VideoWidget::Shape type = VideoWidget::ShapeNone)
   		: theme_(theme) 
		, type_(type) 
		, surface_(NULL) {
		is_initialized_ = false;
	}

	void createBox(OIIO::ImageBuf **buf, int width, int height);

	void drawImage(OIIO::ImageBuf *buf, int x, int y, const char *name, VideoWidget::Zoom zoom);

	cairo_t * createCairoContext(OIIO::ImageBuf *buf);
	void renderCairoContext(OIIO::ImageBuf *buf, cairo_t *cairo);
	void destroyCairoContext(cairo_t *cairo);

	void saveCairoSurface(cairo_t *cairo);
	bool restoreCairoSurface(cairo_t *cairo);

	void background(cairo_t *cr, double radius = 0.0);

	void text(cairo_t *cr, int x, int y, Font &font, 
			const float *fill, const float *outline, const char *text);

	void extents(cairo_t *cr, ShapeBase::Font &font, ShapeBase::TextType type, const char *text,
			int &x, int &y, int &width, int &height);

	double size2pixels(double size) {
		return size * size_factor_;
	}

	double fontsize2pixels(double size) {
		return size * PANGO_SCALE * fontsize_factor_;
	}

	double shadow2pixels(const Font &font) {
		return font.shadow_distance * fontsize2pixels(font.size) / 100000.0;
	}

private:
	cairo_surface_t *surface_;

	double size_factor_;
	double fontsize_factor_;
};

#endif

