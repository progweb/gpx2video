#ifndef __GPX2VIDEO__SHAPE__ARC_H__
#define __GPX2VIDEO__SHAPE__ARC_H__

#include "base.h"


class ArcShape : public ShapeBase {
public:
	int center_x_;
	int center_y_;
	int correction_;

	double start_angle_;
	double end_angle_;

	struct point {
		double x;
		double y;
	};

	ArcShape(VideoWidget::Theme &theme, int width = 0, int height = 0)
		: ShapeBase(theme, VideoWidget::ShapeArc) {
		correction_ = 90;

		// Default arc range
		setArcRange(30.0, 360.0 - 30.0);

		init(width, height);
	}

	virtual ~ArcShape() {
		clear();
	}

	VideoWidget::Theme& theme(void) {
		return theme_;
	}

	virtual bool hasFeature(Feature feature) const {
		switch (feature) {
		case FeatureLabel:
			return false;

		default:
			break;
		}

		return true;
	}

	void init(int width, int height, int size = 0) {
		setSize(width, height, size);
		setPadding(0, 0, 0, 0);
	}

	void setSize(int width, int height, int size = 0) {
		size_ = size;

		center_x_ = width / 2;
		center_y_ = height / 2;
	}

	void setPadding(int left, int right, int top, int bottom) {
		padding_left_ = left;
		padding_right_ = right;
		padding_top_ = top;
		padding_bottom_ = bottom;
	}

	const double& start(void) const {
		return start_angle_;
	}

	const double &end(void) const {
		return end_angle_;
	}

	void setArcRange(double start, double end) {
		start_angle_ = start;
		end_angle_ = end;
	}

	double scale(double min, double max, double value, int rotate = 0) {
		double a;

		double a_range = end_angle_ - start_angle_;

		double v_point = (value - min) / (max - min);

		double a_point = a_range * v_point;

		if (theme().gaugeFlip())
			a = end_angle_ - a_point;
		else
			a = a_point + start_angle_;
	   
		a += rotate;

		return a;
	}

#define DEG2RAD(a) ((a) * M_PI / 180.0)

	struct point locate(double angle, double r_delta) {
		return {
			.x = center_x_ + ((size_ / 2 - r_delta) * sin(DEG2RAD(angle))),
			.y = center_y_ - ((size_ / 2 - r_delta) * cos(DEG2RAD(angle)))
		};
	}

//	virtual OIIO::ImageBuf * prepare(bool &is_update) = 0;
//	virtual OIIO::ImageBuf * render(const TelemetryData &data, bool &is_update) = 0;

	void background(cairo_t *cr, double border, const float *fill, const float *outline);
	void arc(cairo_t *cr, double a1, double a2, double offset, double width, double border, const float *fill, const float *outline = NULL);
	void pieslice(cairo_t *cr, double a1, double a2, double border, const float *fill, const float *outline = NULL);
	void line(cairo_t *cr, double a, double d1, double d2, double width, const float *fill);
	void ticklabel(cairo_t *cr, double a, double d, Font &font, const float *fill, const float *outline, const char *text);
	void value(cairo_t *cr, Font &font, const float *fill, const float *outline, const char *text);
	void unit(cairo_t *cr, Font &font, const float *fill, const float *outline, const char *text);
	void needle(cairo_t *cr, VideoWidget::Theme::NeedleType type,
			double xa, double len, bool design, const float *color1, const float *color2);

	void xmlwrite(std::ostream &os);

private:
	int size_;

	int padding_left_;
	int padding_right_;
	int padding_top_;
	int padding_bottom_;

	void arc_i(cairo_t *cr, double a1, double a2, double offset, double width, double border, const float *fill, const float *outline = NULL);
};

#endif

