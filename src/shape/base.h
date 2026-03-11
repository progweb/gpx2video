#ifndef __GPX2VIDEO__SHAPE__BASE_H__
#define __GPX2VIDEO__SHAPE__BASE_H__

#include "../utils.h"
#include "../oiio.h"
#include "../oiioutils.h"
#include "../videowidget.h"


class ShapeBase {
public:
	class Theme {
	public:
		Theme() {
			setBorderColor(0.0, 0.0, 0.0, 0.5);
			setBackgroundColor(0.0, 0.0, 0.0, 0.5);
		}

		const float * backgroundColor(void) const {
			return bg_color_;
		}

		void setBackgroundColor(double r, double g, double b, double a) {
			bg_color_[0] = r;
			bg_color_[1] = g;
			bg_color_[2] = b;
			bg_color_[3] = a;
		}

		const float * borderColor(void) const {
			return border_color_;
		}

		void setBorderColor(double r, double g, double b, double a) {
			border_color_[0] = r;
			border_color_[1] = g;
			border_color_[2] = b;
			border_color_[3] = a;
		}

	private:
		float bg_color_[4];
		float border_color_[4];
	};

	virtual ~ShapeBase() {
		clear();
	}
	
	Theme& theme(void) {
		return theme_;
	}

	virtual void initialize(void) {
	}

	virtual OIIO::ImageBuf * prepare(bool &is_update) = 0;
	virtual OIIO::ImageBuf * render(const TelemetryData &data, bool &is_update) = 0;

	virtual void clear(void) {
	}

protected:
	Theme &theme_;

	VideoWidget *widget_;

	ShapeBase(Theme &theme, VideoWidget *widget)
   		: theme_(theme)
		, widget_(widget) {
	}

	void createBox(OIIO::ImageBuf **buf, int width, int height);

	void drawBorder(OIIO::ImageBuf *buf);
	void drawBackground(OIIO::ImageBuf *buf);
	void drawText(OIIO::ImageBuf *buf, int x, int y, int px, const char *label);
	void drawImage(OIIO::ImageBuf *buf, int x, int y, const char *name, VideoWidget::Zoom zoom);

	bool textSize(std::string text, int fontsize, 
		int &x1, int &y1, int &x2, int &y2,
		int &width, int &height);

	cairo_t * createCairoContext(OIIO::ImageBuf *buf);
	void renderCairoContext(OIIO::ImageBuf *buf, cairo_t *cairo);
	void destroyCairoContext(cairo_t *cairo);
};

#endif

