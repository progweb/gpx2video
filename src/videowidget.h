#ifndef __GPX2VIDEO__VIDEOWIDGET_H__
#define __GPX2VIDEO__VIDEOWIDGET_H__

#include <iostream>
#include <memory>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <list>

#include <OpenImageIO/imagebuf.h>

#include "log.h"
#include "gpx.h"
#include "gpx2video.h"


class VideoWidget : public GPX2Video::Task {
public:
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

	enum Align {
		AlignNone = 0,
		AlignHorizontal,
		AlignVertical,
		AlignUnknown
	};

	enum Margin {
		MarginAll,
		MarginLeft,
		MarginRight,
		MarginBottom,
		MarginTop
	};

	enum Padding {
		PaddingAll,
		PaddingLeft,
		PaddingRight,
		PaddingBottom,
		PaddingTop
	};

	enum Unit {
		UnitNone,
		UnitMPH,
		UnitKPH,
		UnitKm,
		UnitMeter,
		UnitFoot,
		UnitMiles,
		UnitCelsius,
		UnitFarenheit,
		UnitUnknown
	};

	enum Zoom {
		ZoomNone,
		ZoomFit,
		ZoomFill,
		ZoomCrop,
		ZoomStretch,
		ZoomUnknown
	};

	enum Flag {
		FlagNone = 0,
		FlagLabel = (1 << 0),
		FlagValue = (1 << 1),
		FlagPicto = (1 << 2),
		FlagUnit = (1 << 3),
		FlagUnknown = (1 << 4),
	};

	enum TextAlign {
		TextAlignLeft,
		TextAlignCenter,
		TextAlignRight,
		TextAlignUnknown
	};

	virtual ~VideoWidget() {
		log_call();
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

	Position& position(void) {
		return position_;
	}

	virtual void setPosition(Position position) {
		position_ = position;
	}

	Align& align(void) {
		return align_;
	}

	virtual void setAlign(Align align) {
		align_ = align;
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

	virtual void setSize(int width, int height) {
		width_ = width;
		height_ = height;
	}

	const int& width(void) const {
		return width_;
	}

	const int& height(void) const {
		return height_;
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

	const std::string& name(void) const {
		return name_;
	}

	const std::string& label(void) const {
		return label_;
	}

	virtual void setLabel(std::string label) {
		label_ = label;
	}

	const std::string& font(void) const {
		return font_;
	}

	virtual void setFont(std::string font) {
		if (font.empty())
			return;

		font_ = font;
	}

	const std::string& text(void) const {
		return text_;
	}

	virtual void setText(std::string text) {
		text_ = text;
	}

	const float * textColor(void) const {
		return txtcolor_;
	}

	virtual bool setTextColor(std::string color) {
		return hex2color(txtcolor_, color);
	}

	double textRatio(void) const {
		return txtratio_;
	}

	virtual void setTextRatio(double ratio) {
		if (ratio == 0.0)
			return;

		txtratio_ = ratio;
	}

	int textShadow(void) const {
		return txtshadow_;
	}

	virtual void setTextShadow(int shadow) {
		txtshadow_ = shadow;
	}

	int textLineSpace(void) const {
		return txtlinespace_;
	}

	virtual void setTextLineSpace(int linespace) {
		if (linespace < 0)
			return;

		txtlinespace_ = linespace;
	}

	enum TextAlign labelAlign(void) const {
		return label_align_;
	}

	virtual void setLabelAlign(TextAlign align) {
		label_align_ = align;
	}

	enum TextAlign valueAlign(void) const {
		return value_align_;
	}

	virtual void setValueAlign(TextAlign align) {
		value_align_ = align;
	}

	int border(void) const {
		return border_;
	}

	virtual void setBorder(int border) {
		border_ = border;
	}

	const float * borderColor(void) const {
		return bordercolor_;
	}

	virtual bool setBorderColor(std::string color) {
		return hex2color(bordercolor_, color);
	}

	const float * backgroundColor(void) const {
		return bgcolor_;
	}

	virtual bool setBackgroundColor(std::string color) {
		return hex2color(bgcolor_, color);
	}

	virtual void setFlags(int flags) {
		flags_ = flags;
	}

	virtual void addFlag(VideoWidget::Flag flag) {
		flags_ |= flag;
	}

	virtual bool hasFlag(VideoWidget::Flag flag) {
		return ((flags_ & flag) != 0);
	}

	virtual bool run(void) {
		log_call();

		complete();

		return true;
	}

	virtual void initialize(void);
	virtual OIIO::ImageBuf * prepare(void) = 0; 
	virtual OIIO::ImageBuf * render(const GPXData &data, bool &is_update) = 0;

	static Position string2position(std::string &s);
	static Align string2align(std::string &s);
	static TextAlign string2textAlign(std::string &s);
	static Unit string2unit(std::string &s);
	static Zoom string2zoom(std::string &s);
	static std::string unit2string(Unit unit);
	static bool hex2color(float color[4], std::string html);

protected:
	VideoWidget(GPX2Video &app, std::string name)  
		: GPX2Video::Task(app)
		, app_(app) 
		, at_begin_time_(0)
		, at_end_time_(0)
		, label_px_(0)
   		, label_size_(0)
		, value_px_(0)
   		, value_size_(0)
		, value_offset_(0)
		, txtratio_(0.0)
		, name_(name) {
		log_call();

		setPosition(PositionNone);
		setAlign(AlignNone);
		setAtTime(0, 0);
		setPosition(0, 0);
		setSize(64, 64);
		setMargin(MarginAll, 10);
		setPadding(PaddingAll, 0);
		setFont("./assets/fonts/Helvetica.ttf");
		setLabel(name);
		setTextRatio(5.0);
		setTextShadow(0);
		setTextLineSpace(10);
		setUnit(VideoWidget::UnitNone);
		setTextColor("#ffffffff");
		setLabelAlign(VideoWidget::TextAlignLeft);
		setValueAlign(VideoWidget::TextAlignLeft);
		setBorder(0);
		setBorderColor("#00000000");
		setBackgroundColor("#00000000");
		setFlags(VideoWidget::FlagLabel | VideoWidget::FlagValue | VideoWidget::FlagPicto | VideoWidget::FlagUnit);
	}

	void createBox(OIIO::ImageBuf **buf, int width, int height);

	void drawBorder(OIIO::ImageBuf *buf);
	void drawBackground(OIIO::ImageBuf *buf);
	void drawImage(OIIO::ImageBuf *buf, int x, int y, const char *name, Zoom zoom);
	void drawText(OIIO::ImageBuf *buf, int x, int y, int pt, const char *label);
	void drawLabel(OIIO::ImageBuf *buf, const char *label);
	void drawValue(OIIO::ImageBuf *buf, const char *value);

	void textSize(std::string text, int fontsize, 
		int &x1, int &y1, int &x2, int &y2,
		int &width, int &height);

	GPX2Video &app_;
	Position position_;
	Align align_;
	Unit unit_;
	Zoom zoom_;
	std::string format_;
	std::string source_;

	const int null_ = 0;

	uint64_t at_begin_time_;
	uint64_t at_end_time_;

	int x_;
	int y_;
	int width_;
	int height_;
	int margin_top_;
	int margin_bottom_;
	int margin_left_;
	int margin_right_;
	int padding_;
	int padding_top_;
	int padding_bottom_;
	int padding_left_;
	int padding_right_;

	std::string font_;

	int label_px_;
	int label_size_;
	int value_px_;
	int value_size_;
	int value_offset_;
	std::string text_;

	std::string label_;
	double txtratio_;
	int txtshadow_;
	int txtlinespace_;
	float txtcolor_[4];

	enum TextAlign label_align_;
	enum TextAlign value_align_;

	int border_;
	float bordercolor_[4];
	float bgcolor_[4];

	int flags_;

private:
	std::string name_;
};

#endif

