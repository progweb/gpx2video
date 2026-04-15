#ifndef __GPX2VIDEO__SHAPE__BASE_H__
#define __GPX2VIDEO__SHAPE__BASE_H__

#include "../utils.h"
#include "../oiio.h"
#include "../oiioutils.h"
#include "../telemetrydata.h"
#include "../videowidget.h"


class ShapeBase {
public:
	struct Font {
		int size;
		int border;
		int shadow_opacity;
		int shadow_distance;
		std::string family;
		VideoWidget::Theme::FontStyle style;
		VideoWidget::Theme::FontWeight weight;
	};

	virtual ~ShapeBase() {
		clear();
	}
	
	virtual OIIO::ImageBuf * prepare(bool &is_update) = 0;
	virtual OIIO::ImageBuf * render(const TelemetryData &data, bool &is_update) = 0;

	virtual void clear(void) {
	}

	virtual void xmlwrite(std::ostream &os);

protected:
	VideoWidget::Theme &theme_;

	ShapeBase(VideoWidget::Theme &theme)
   		: theme_(theme) {
	}

	void createBox(OIIO::ImageBuf **buf, int width, int height);

	void drawBorder(OIIO::ImageBuf *buf);
	void drawBackground(OIIO::ImageBuf *buf);
//	void drawText(OIIO::ImageBuf *buf, int x, int y, int px, const char *label);
	void drawImage(OIIO::ImageBuf *buf, int x, int y, const char *name, VideoWidget::Zoom zoom);

//	bool textSize(std::string text, int fontsize, 
//		int &x1, int &y1, int &x2, int &y2,
//		int &width, int &height);

	cairo_t * createCairoContext(OIIO::ImageBuf *buf);
	void renderCairoContext(OIIO::ImageBuf *buf, cairo_t *cairo);
	void destroyCairoContext(cairo_t *cairo);

	void drawText(cairo_t *cr, int x, int y, Font &font, const float *fill, const float *outline, const char *text);

	void textSize(cairo_t *cr, ShapeBase::Font &font, const char *text,
			int &x, int &y, int &width, int &height);
};

#endif

