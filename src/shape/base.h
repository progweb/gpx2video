#ifndef __GPX2VIDEO__SHAPE__BASE_H__
#define __GPX2VIDEO__SHAPE__BASE_H__

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
		FeatureNeedle,
		FeatureRoundCorner,
		FeatureUnknown
	};

	struct Font {
		int size;
		int border;
		int shadow_opacity;
		int shadow_distance;
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

	virtual void draw(cairo_t *cairo, const TelemetryData &data) = 0;

	virtual void clear(void) {
		is_initialized_ = false;
	}

	virtual void xmlwrite(std::ostream &os);

protected:
	VideoWidget::Theme &theme_;

	VideoWidget::Shape type_;

	bool is_initialized_;

	ShapeBase(VideoWidget::Theme &theme, VideoWidget::Shape type = VideoWidget::ShapeNone)
   		: theme_(theme) 
		, type_(type) {
		is_initialized_ = false;
	}

	void createBox(OIIO::ImageBuf **buf, int width, int height);

	void drawImage(OIIO::ImageBuf *buf, int x, int y, const char *name, VideoWidget::Zoom zoom);

	cairo_t * createCairoContext(OIIO::ImageBuf *buf);
	void renderCairoContext(OIIO::ImageBuf *buf, cairo_t *cairo);
	void destroyCairoContext(cairo_t *cairo);

	void background(cairo_t *cr, double radius = 0.0);

	void text(cairo_t *cr, int x, int y, Font &font, const float *fill, const float *outline, const char *text);

	void extents(cairo_t *cr, ShapeBase::Font &font, const char *text,
			int &x, int &y, int &width, int &height);
};

#endif

