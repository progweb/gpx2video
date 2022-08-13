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
//		AlignLeft,
//		AlignRight,
//		AlignBottom,
//		AlignTop,
//		AlignBottomLeft,
//		AlignBottomRight,
//		AlignTopLeft,
//		AlignTopRight,
		AlignUnknown
	};

	enum Margin {
		MarginAll,
		MarginLeft,
		MarginRight,
		MarginBottom,
		MarginTop
	};

	enum Unit {
		UnitNone,
		UnitMPH,
		UnitKPH,
		UnitKm,
		UnitMeter,
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

	virtual ~VideoWidget() {
		log_call();
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

	const int& padding(void) const {
		return padding_;
	}

	virtual void setPadding(int padding) {
		padding_ = padding;
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

	const float * textColor(void) const {
		return txtcolor_;
	}

	virtual bool setTextColor(std::string color) {
		return hex2color(txtcolor_, color);
	}

	int textShadow(void) const {
		return txtshadow_;
	}

	virtual void setTextShadow(int shadow_) {
		txtshadow_ = shadow_;
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

	virtual bool run(void) {
		log_call();

		complete();

		return true;
	}

	virtual void prepare(OIIO::ImageBuf *buf) = 0;
	virtual void render(OIIO::ImageBuf *buf, const GPXData &data) = 0;

	static Position string2position(std::string &s);
	static Align string2align(std::string &s);
	static Unit string2unit(std::string &s);
	static Zoom string2zoom(std::string &s);
	static std::string unit2string(Unit unit);
	static bool hex2color(float color[4], std::string html);

protected:
	VideoWidget(GPX2Video &app, std::string name)  
		: GPX2Video::Task(app)
		, app_(app) 
		, name_(name) {
		log_call();

		setPosition(PositionNone);
		setAlign(AlignNone);
		setPosition(0, 0);
		setSize(64, 64);
		setMargin(MarginAll, 10);
		setPadding(0);
		setLabel(name);
		setTextShadow(0);
		setUnit(VideoWidget::UnitNone);
		setTextColor("#ffffffff");
		setBorder(0);
		setBorderColor("#00000000");
		setBackgroundColor("#00000000");
	}

	void createBox(OIIO::ImageBuf **buf, int width, int height);

	void drawBorder(OIIO::ImageBuf *buf);
	void drawBackground(OIIO::ImageBuf *buf);
	void drawImage(OIIO::ImageBuf *buf, int x, int y, const char *name, Zoom zoom);
	void drawText(OIIO::ImageBuf *buf, int x, int y, int pt, const char *label);
	void drawLabel(OIIO::ImageBuf *buf, int x, int y, const char *label);
	void drawValue(OIIO::ImageBuf *buf, int x, int y, const char *value);

	GPX2Video &app_;
	Position position_;
	Align align_;
	Unit unit_;
	Zoom zoom_;
	std::string format_;
	std::string source_;

	const int null_ = 0;

	int x_;
	int y_;
	int width_;
	int height_;
	int margin_top_;
	int margin_bottom_;
	int margin_left_;
	int margin_right_;
	int padding_;

	std::string label_;
	int txtshadow_;
	float txtcolor_[4];

	int border_;
	float bordercolor_[4];
	float bgcolor_[4];

private:
	std::string name_;
};

#endif

