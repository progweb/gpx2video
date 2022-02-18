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
	enum Align {
		AlignNone = 0,
		AlignLeft,
		AlignRight,
		AlignBottom,
		AlignTop,
		AlignBottomLeft,
		AlignBottomRight,
		AlignTopLeft,
		AlignTopRight,
		AlignUnknown
	};

	enum Units {
		UnitNone,
		UnitMPH,
		UnitKPH,
		UnitKm,
		UnitMeter,
		UnitMiles,
		UnitUnknown
	};

	virtual ~VideoWidget() {
		log_call();
	}

	Align& align(void) {
		return align_;
	}

	virtual void setAlign(Align align) {
		align_ = align;
	}

	Units& units(void) {
		return units_;
	}

	virtual void setUnits(Units units) {
		units_ = units;
	}

	const std::string& format(void) {
		return format_;
	}

	void setFormat(std::string format) {
		format_ = format;
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

	const int& margin(void) const {
		return margin_;
	}

	virtual void setMargin(int margin) {
		margin_ = margin;
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

	virtual void run(void) {
		log_call();

		complete();
	}

	virtual void render(OIIO::ImageBuf *buf, const GPXData &data) = 0;

	static Align string2align(std::string &s);
	static Units string2units(std::string &s);
	static std::string units2string(Units units);

protected:
	VideoWidget(GPX2Video &app, std::string name)  
		: GPX2Video::Task(app)
		, app_(app) 
		, name_(name) {
		log_call();

		setAlign(AlignNone);
		setPosition(0, 0);
		setSize(64, 64);
		setMargin(10);
		setPadding(0);
		setLabel(name);
		setUnits(VideoWidget::UnitNone);
	}

	void add(OIIO::ImageBuf *frame, int x, int y, const char *picto, const char *label, const char *value, double divider);

	GPX2Video &app_;
	Align align_;
	Units units_;
	std::string format_;

	int x_;
	int y_;
	int width_;
	int height_;
	int margin_;
	int padding_;
	std::string label_;

private:
	std::string name_;
};

#endif

