#ifndef __GPX2VIDEO__VIDEOWIDGET_H__
#define __GPX2VIDEO__VIDEOWIDGET_H__

#include <iostream>
#include <memory>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <list>

#include <cairo.h>
#include <OpenImageIO/imagebuf.h>

#include "telemetrymedia.h"
#include "application.h"


class VideoWidget : public GPXApplication::Task {
public:
	enum Margin {
		MarginAll,
		MarginLeft,
		MarginRight,
		MarginBottom,
		MarginTop
	};

	enum Orientation {
		OrientationNone = 0,
		OrientationHorizontal,
		OrientationVertical,
		OrientationUnknown
	};

	enum Position {
		PositionNone = 0,
		PositionLeft,
		PositionRight,
		PositionBottom,
		PositionTop,
		PositionBottomLeft,
		PositionBottomRight,
		PositionTopLeft,
		PositionTopRight,
		PositionUnknown
	};

	enum Unit {
		UnitNone,
		UnitMPS, // Meter / S
		UnitMPH, // Mile / H
		UnitKPH, // Km / H
		UnitMPM, // Min / Mile
		UnitMPK, // Min / Km
		UnitKm,
		UnitMeter,
		UnitFoot,
		UnitMiles,
		UnitCelsius,
		UnitFarenheit,
		UnitG,
		UnitMeterPS2,
		UnitUnknown
	};

	enum Shape {
		ShapeNone = 0,
		ShapeText,
		ShapeArc,
		ShapeBar,
		ShapeUnknown
	};

	enum Zoom {
		ZoomNone,
		ZoomFit,
		ZoomFill,
		ZoomCrop,
		ZoomStretch,
		ZoomUnknown
	};

	virtual ~VideoWidget() {
	}

	class Theme {
	public:
		enum Align {
			AlignLeft = 0,
			AlignCenter,
			AlignRight,
			AlignUnknown
		};

		enum Padding {
			PaddingAll,
			PaddingLeft,
			PaddingRight,
			PaddingBottom,
			PaddingTop
		};

		enum Flag {
			FlagNone = 0,
			FlagLabel = (1 << 0),
			FlagValue = (1 << 1),
			FlagIcon = (1 << 2),
			FlagUnit = (1 << 3),
			FlagNeedle = (1 << 4),
			FlagTick = (1 << 5),
			FlagTickLabel = (1 << 6),
			FlagCursor = (1 << 7),
			FlagGauge = (1 << 8),
			FlagUnknown = (1 << 9),
			FlagAll = ~0x0
		};

		enum FontStyle {
			FontStyleNormal,
			FontStyleOblique,
			FontStyleItalic,
			FontStyleUnknown
		};

		enum FontWeight {
			FontWeightThin = 100,
			FontWeightUltraLight = 200,
			FontWeightLight = 300,
			FontWeightSemiLight = 350,
			FontWeightBook = 380,
			FontWeightNormal = 400,
			FontWeightMedium = 500,
			FontWeightSemiBold = 600,
			FontWeightBold = 700,
			FontWeightUltraBold = 800,
			FontWeightHeavy = 900,
			FontWeightUltraHeavy = 1000,
			FontWeightUnknown
		};

		enum NeedleType {
			NeedleTypeThin,
			NeedleTypeLight,
			NeedleTypeBasic,
			NeedleTypeDesign,
			NeedleTypeUnknown
		};

		Theme();

		void setSize(int width, int height) {
			width_ = width;
			height_ = height;
		}

		const int& width(void) const {
			return width_;
		}

		const int& height(void) const {
			return height_;
		}

		void setFlags(int flags) {
			flags_ = flags;
		}

		void addFlag(VideoWidget::Theme::Flag flag) {
			flags_ |= flag;
		}

		void removeFlag(VideoWidget::Theme::Flag flag) {
			flags_ &= ~flag;
		}

		bool hasFlag(VideoWidget::Theme::Flag flag) {
			return ((flags_ & flag) != 0);
		}

		const int& padding(enum Padding side) const {
			switch (side) {
			case PaddingLeft:
				return padding_left_;
			case PaddingRight:
				return padding_right_;
			case PaddingTop:
				return padding_top_;
			case PaddingBottom:
				return padding_bottom_;
			default:
				return null_;
			}
		}

		virtual void setPadding(enum Padding side, int padding) {
			if (padding < 0)
				return;

			switch (side) {
			case PaddingAll:
				padding_left_ = padding;
				padding_right_ = padding;
				padding_top_ = padding;
				padding_bottom_ = padding;
				break;
			case PaddingLeft:
				padding_left_ = padding;
				break;
			case PaddingRight:
				padding_right_ = padding;
				break;
			case PaddingTop:
				padding_top_ = padding;
				break;
			case PaddingBottom:
				padding_bottom_ = padding;
				break;
			default:
				break;
			}
		}

		const float * backgroundColor(void) const {
			return bg_color_;
		}

		bool setBackgroundColor(std::string color) {
			return hex2color(bg_color_, color);
		}

		bool setBackgroundColor(double r, double g, double b, double a) {
			bg_color_[0] = r;
			bg_color_[1] = g;
			bg_color_[2] = b;
			bg_color_[3] = a;
			return true;
		}

		int border(void) const {
			return border_;
		}

		void setBorder(int border) {
			border_ = border;
		}

		const float * borderColor(void) const {
			return border_color_;
		}

		bool setBorderColor(std::string color) {
			return hex2color(border_color_, color);
		}

		bool setBorderColor(double r, double g, double b, double a) {
			border_color_[0] = r;
			border_color_[1] = g;
			border_color_[2] = b;
			border_color_[3] = a;
			return true;
		}

		const float * gaugeColor(int index) const {
			return gauge_color_[index];
		}

		bool setGaugeColor(int index, std::string color) {
			return hex2color(gauge_color_[index], color);
		}

		bool setGaugeColor(int index, double r, double g, double b, double a) {
			gauge_color_[index][0] = r;
			gauge_color_[index][1] = g;
			gauge_color_[index][2] = b;
			gauge_color_[index][3] = a;
			return true;
		}

		const float * tickColor(void) const {
			return tick_color_;
		}

		bool setTickColor(std::string color) {
			return hex2color(tick_color_, color);
		}

		bool setTickColor(double r, double g, double b, double a) {
			tick_color_[0] = r;
			tick_color_[1] = g;
			tick_color_[2] = b;
			tick_color_[3] = a;
			return true;
		}

		const float * tickLabelColor(void) const {
			return tick_label_color_;
		}

		bool setTickLabelColor(std::string color) {
			return hex2color(tick_label_color_, color);
		}

		bool setTickLabelColor(double r, double g, double b, double a) {
			tick_label_color_[0] = r;
			tick_label_color_[1] = g;
			tick_label_color_[2] = b;
			tick_label_color_[3] = a;
			return true;
		}

		const std::string& font(void) const {
			return font_;
		}

		virtual void setFont(std::string font) {
			if (font.empty())
				return;

			font_ = font;
		}

		enum Align labelAlign(void) const {
			return label_align_;
		}

		virtual void setLabelAlign(Align align) {
			label_align_ = align;
		}

		int labelFontSize(void) const {
			return label_font_size_;
		}

		void setLabelFontSize(int size) {
			if (size < 0)
				return;

			label_font_size_ = size;
		}

		enum FontStyle labelFontStyle(void) const {
			return label_font_style_;
		}

		void setLabelFontStyle(FontStyle style) {
			label_font_style_ = style;
		}

		enum FontWeight labelFontWeight(void) const {
			return label_font_weight_;
		}

		void setLabelFontWeight(FontWeight weight) {
			label_font_weight_ = weight;
		}

		const float * labelColor(void) const {
			return label_color_;
		}

		bool setLabelColor(std::string color) {
			return hex2color(label_color_, color);
		}

		bool setLabelColor(double r, double g, double b, double a) {
			label_color_[0] = r;
			label_color_[1] = g;
			label_color_[2] = b;
			label_color_[3] = a;
			return true;
		}

		int labelShadowDistance(void) const {
			return label_shadow_distance_;
		}

		void setLabelShadowDistance(int distance) {
			if (distance < 0)
				return;

			label_shadow_distance_ = distance;
		}

		int labelShadowOpacity(void) const {
			return label_shadow_opacity_;
		}

		void setLabelShadowOpacity(int opacity) {
			if (opacity < 0)
				return;

			label_shadow_opacity_ = opacity;
		}

		int labelBorderWidth(void) const {
			return label_border_width_;
		}

		void setLabelBorderWidth(int width) {
			if (width < 0)
				return;

			label_border_width_ = width;
		}

		const float * labelBorderColor(void) const {
			return label_border_color_;
		}

		bool setLabelBorderColor(std::string color) {
			return hex2color(label_border_color_, color);
		}

		bool setLabelBorderColor(double r, double g, double b, double a) {
			label_border_color_[0] = r;
			label_border_color_[1] = g;
			label_border_color_[2] = b;
			label_border_color_[3] = a;
			return true;
		}

//		int textShadow(void) const {
//			return text_shadow_;
//		}
//
//		virtual void setTextShadow(int shadow) {
//			text_shadow_ = shadow;
//		}
//
//		double textRatio(void) const {
//			return text_ratio_;
//		}
//
//		virtual void setTextRatio(double ratio) {
//			if (ratio == 0.0)
//				return;
//
//			text_ratio_ = ratio;
//		}
//
//		int textLineSpace(void) const {
//			return text_linespace_;
//		}
//
//		virtual void setTextLineSpace(int linespace) {
//			if (linespace < 0)
//				return;
//
//			text_linespace_ = linespace;
//		}

		NeedleType needleType(void) const {
			return needle_type_;
		}

		void setNeedleType(NeedleType type) {
			needle_type_ = type;
		}

		const float * needlePrimaryColor(void) const {
			return needle_primary_color_;
		}

		bool setNeedlePrimaryColor(std::string color) {
			return hex2color(needle_primary_color_, color);
		}

		bool setNeedlePrimaryColor(double r, double g, double b, double a) {
			needle_primary_color_[0] = r;
			needle_primary_color_[1] = g;
			needle_primary_color_[2] = b;
			needle_primary_color_[3] = a;
			return true;
		}

		const float * needleSecondaryColor(void) const {
			return needle_secondary_color_;
		}

		bool setNeedleSecondaryColor(std::string color) {
			return hex2color(needle_secondary_color_, color);
		}

		bool setNeedleSecondaryColor(double r, double g, double b, double a) {
			needle_secondary_color_[0] = r;
			needle_secondary_color_[1] = g;
			needle_secondary_color_[2] = b;
			needle_secondary_color_[3] = a;
			return true;
		}

		const float * cursorColor(void) const {
			return cursor_color_;
		}

		bool setCursorColor(std::string color) {
			return hex2color(cursor_color_, color);
		}

		bool setCursorColor(double r, double g, double b, double a) {
			cursor_color_[0] = r;
			cursor_color_[1] = g;
			cursor_color_[2] = b;
			cursor_color_[3] = a;
			return true;
		}

		const int& gaugeBorder(void) const {
			return gauge_border_;
		}

		void setGaugeBorder(int border) {
			gauge_border_ = border;
		}

		const float * gaugeBorderColor(void) const {
			return gauge_border_color_;
		}

		bool setGaugeBorderColor(std::string color) {
			return hex2color(gauge_border_color_, color);
		}

		bool setGaugeBorderColor(double r, double g, double b, double a) {
			gauge_border_color_[0] = r;
			gauge_border_color_[1] = g;
			gauge_border_color_[2] = b;
			gauge_border_color_[3] = a;
			return true;
		}

		const float * gaugeBackgroundColor(void) const {
			return gauge_bg_color_;
		}

		bool setGaugeBackgroundColor(std::string color) {
			return hex2color(gauge_bg_color_, color);
		}

		bool setGaugeBackgroundColor(double r, double g, double b, double a) {
			gauge_bg_color_[0] = r;
			gauge_bg_color_[1] = g;
			gauge_bg_color_[2] = b;
			gauge_bg_color_[3] = a;
			return true;
		}

		const Align& tickAlign(void) const {
			return tick_align_;
		}

		void setTickAlign(Align align) {
			tick_align_ = align;
		}

		const int& valueMin(void) const {
			return value_min_;
		}

		void setValueMin(int value) {
			value_min_ = value;
		}

		const int& valueMax(void) const {
			return value_max_;
		}

		void setValueMax(int value) {
			value_max_ = value;
		}

		enum Align valueAlign(void) const {
			return value_align_;
		}

		virtual void setValueAlign(Align align) {
			value_align_ = align;
		}

		int valueFontSize(void) const {
			return value_font_size_;
		}

		void setValueFontSize(int size) {
			if (size < 0)
				return;

			value_font_size_ = size;
		}

		enum FontStyle valueFontStyle(void) const {
			return value_font_style_;
		}

		void setValueFontStyle(FontStyle style) {
			value_font_style_ = style;
		}

		enum FontWeight valueFontWeight(void) const {
			return value_font_weight_;
		}

		void setValueFontWeight(FontWeight weight) {
			value_font_weight_ = weight;
		}

		const float * valueColor(void) const {
			return value_color_;
		}

		bool setValueColor(std::string color) {
			return hex2color(value_color_, color);
		}

		bool setValueColor(double r, double g, double b, double a) {
			value_color_[0] = r;
			value_color_[1] = g;
			value_color_[2] = b;
			value_color_[3] = a;
			return true;
		}

		int valueShadowDistance(void) const {
			return value_shadow_distance_;
		}

		void setValueShadowDistance(int distance) {
			if (distance < 0)
				return;

			value_shadow_distance_ = distance;
		}

		int valueShadowOpacity(void) const {
			return value_shadow_opacity_;
		}

		void setValueShadowOpacity(int opacity) {
			if (opacity < 0)
				return;

			value_shadow_opacity_ = opacity;
		}

		const int& valueBorderWidth(void) const {
			return value_border_width_;
		}

		void setValueBorderWidth(int width) {
			if (width < 0)
				return;

			value_border_width_ = width;
		}

		const float * valueBorderColor(void) const {
			return value_border_color_;
		}

		bool setValueBorderColor(std::string color) {
			return hex2color(value_border_color_, color);
		}

		bool setValueBorderColor(double r, double g, double b, double a) {
			value_border_color_[0] = r;
			value_border_color_[1] = g;
			value_border_color_[2] = b;
			value_border_color_[3] = a;
			return true;
		}

		const float * valueBackgroundColor(void) const {
			return value_bg_color_;
		}

		bool setValueBackgroundColor(std::string color) {
			return hex2color(value_bg_color_, color);
		}

		bool setValueBackgroundColor(double r, double g, double b, double a) {
			value_bg_color_[0] = r;
			value_bg_color_[1] = g;
			value_bg_color_[2] = b;
			value_bg_color_[3] = a;
			return true;
		}

		static bool hex2color(float color[4], std::string html);
		static std::string color2hex(const float color[4]);

	private:
		const int null_ = 0;

		int width_;
		int height_;

		int flags_;

		int padding_;
		int padding_top_;
		int padding_bottom_;
		int padding_left_;
		int padding_right_;

		float bg_color_[4];

		int border_;
		float border_color_[4];

		float cursor_color_[4];

		int gauge_border_;
		float gauge_border_color_[4];
		float gauge_bg_color_[4];
		float gauge_color_[2][4];

		Align tick_align_;
		float tick_color_[4];
		float tick_label_color_[4];

		std::string font_;

		Align label_align_;
		int label_font_size_;
		FontStyle label_font_style_;
		FontWeight label_font_weight_;
		int label_border_width_;
		int label_shadow_opacity_;
		int label_shadow_distance_;
		float label_color_[4];
		float label_border_color_[4];

		NeedleType needle_type_;
		float needle_primary_color_[4];
		float needle_secondary_color_[4];

		int value_min_;
		int value_max_;
		Align value_align_;
		int value_font_size_;
		FontStyle value_font_style_;
		FontWeight value_font_weight_;
		int value_border_width_;
		int value_shadow_opacity_;
		int value_shadow_distance_;
		float value_color_[4];
		float value_bg_color_[4];
		float value_border_color_[4];

//		int text_shadow_;
//		double text_ratio_;
//		int text_linespace_;
	};

	Shape& shape(void) {
		return shape_;
	}

	Theme& theme(void) {
		return theme_;
	}

	virtual void setShape(Shape shape) {
		shape_ = shape;
	}

	uint64_t& atBeginTime(void) {
		return at_begin_time_;
	}

	uint64_t& atEndTime(void) {
		return at_end_time_;
	}

	virtual void setAtTime(const uint64_t begintime, const uint64_t endtime) {
		at_begin_time_ = begintime;

		if (begintime != endtime)
			at_end_time_ = endtime;
	}

	virtual void setSize(int width, int height) {
		theme().setSize(width, height);
	}

	Position& position(void) {
		return position_;
	}

	virtual void setPosition(Position position) {
		position_ = position;
	}

	Orientation& orientation(void) {
		return orientation_;
	}

	virtual void setOrientation(Orientation orientation) {
		orientation_ = orientation;
	}

	Unit& unit(void) {
		return unit_;
	}

	virtual void setUnit(Unit unit) {
		unit_ = unit;
	}

	Zoom& zoom(void) {
		return zoom_;
	}

	virtual void setZoom(Zoom zoom) {
		zoom_ = zoom;
	}

	const std::string& format(void) {
		return format_;
	}

	virtual void setFormat(std::string format) {
		format_ = format;
	}

	const std::string& source(void) {
		return source_;
	}

	virtual void setSource(std::string source) {
		source_ = source;
	}

	virtual void setPosition(int x, int y) {
		x_ = x;
		y_ = y;
	}

	const int& x(void) const {
		return x_;
	}

	const int& y(void) const {
		return y_;
	}

	const int& margin(enum Margin side) const {
		switch (side) {
		case MarginLeft:
			return margin_left_;
		case MarginRight:
			return margin_right_;
		case MarginTop:
			return margin_top_;
		case MarginBottom:
			return margin_bottom_;
		default:
			return null_;
		}
	}

	virtual void setMargin(enum Margin side, int margin) {
		if (margin < 0)
			return;

		switch (side) {
		case MarginAll:
			margin_left_ = margin;
			margin_right_ = margin;
			margin_top_ = margin;
			margin_bottom_ = margin;
			break;
		case MarginLeft:
			margin_left_ = margin;
			break;
		case MarginRight:
			margin_right_ = margin;
			break;
		case MarginTop:
			margin_top_ = margin;
			break;
		case MarginBottom:
			margin_bottom_ = margin;
			break;
		default:
			break;
		}
	}

	const std::string& name(void) const {
		return name_;
	}

	const std::string& label(void) const {
		return label_;
	}

	virtual void setLabel(std::string label) {
		label_ = label;
	}

	const std::string& text(void) const {
		return text_;
	}

	virtual void setText(std::string text) {
		text_ = text;
	}

	virtual bool isStatic(void) {
		return false;
	}

	virtual bool run(void) {
		complete();

		return true;
	}

	void dump(void);

	virtual OIIO::ImageBuf * prepare(bool &is_update) = 0; 
	virtual OIIO::ImageBuf * render(const TelemetryData &data, bool &is_update) = 0;
	virtual void clear(void) = 0;

	virtual void save(std::ostream &os);

	static Shape string2shape(std::string &s);
	static Position string2position(std::string &s);
	static Orientation string2orientation(std::string &s);
	static Theme::Align string2align(std::string &s);
	static Theme::FontStyle string2fontstyle(std::string &s);
	static Theme::FontWeight string2fontweight(std::string &s);
	static Theme::NeedleType string2needletype(std::string &);
	static Unit string2unit(std::string &s);
	static Zoom string2zoom(std::string &s);

	static std::string bool2string(bool value);
	static std::string position2string(Position position);
	static std::string orientation2string(Orientation orientation);
	static std::string align2string(Theme::Align align);
	static std::string fontstyle2string(Theme::FontStyle style);
	static std::string fontweight2string(Theme::FontWeight weight);
	static std::string needletype2string(Theme::NeedleType type);
	static std::string unit2string(Unit unit, bool label=true);

protected:
	VideoWidget(GPXApplication &app, std::string name)  
		: GPXApplication::Task(app, name)
		, app_(app) 
		, at_begin_time_(0)
		, at_end_time_(0)
//		, label_px_(0)
//   		, label_size_(0)
//		, value_px_(0)
//   		, value_size_(0)
//		, value_offset_(0)
		, name_(name) {
		setShape(ShapeText),
		setPosition(PositionNone);
		setOrientation(OrientationNone);
		setAtTime(0, 0);
		setPosition(0, 0);
		setMargin(MarginAll, 10);
		setLabel(name);
		setUnit(VideoWidget::UnitNone);
	}

//	void createBox(OIIO::ImageBuf **buf, int width, int height);
//
//	void drawBorder(OIIO::ImageBuf *buf);
//	void drawBackground(OIIO::ImageBuf *buf);
//	void drawImage(OIIO::ImageBuf *buf, int x, int y, const char *name, Zoom zoom);
//	void drawText(OIIO::ImageBuf *buf, int x, int y, int pt, const char *label);
//	void drawLabel(OIIO::ImageBuf *buf, const char *label);
//	void drawValue(OIIO::ImageBuf *buf, const char *value);
//
//	bool textSize(std::string text, int fontsize, 
//		int &x1, int &y1, int &x2, int &y2,
//		int &width, int &height);

	virtual void xmlopen(std::ostream &os);
	virtual void xmlwrite(std::ostream &os);
	virtual void xmlclose(std::ostream &os);

	GPXApplication &app_;
	Shape shape_;
	Position position_;
	Orientation orientation_;
	Unit unit_;
	Zoom zoom_;
	std::string format_;
	std::string source_;

	const int null_ = 0;

	uint64_t at_begin_time_;
	uint64_t at_end_time_;

	int x_;
	int y_;
	int margin_top_;
	int margin_bottom_;
	int margin_left_;
	int margin_right_;

//	int label_px_;
//	int label_size_;
//	int value_px_;
//	int value_size_;
//	int value_offset_;
	std::string text_;

	std::string label_;

//	int flags_;

	Theme theme_;

private:
	std::string name_;
};

#endif

