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
		AlignTop
	};

	~VideoWidget() {
		log_call();
	}

	Align& align(void) {
		return align_;
	}

	void setAlign(Align align) {
		align_ = align;
	}

	void setPosition(int x, int y) {
		x_ = x;
		y_ = y;
	}

	const int& x(void) const {
		return x_;
	}

	const int& y(void) const {
		return y_;
	}

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

	const int& margin(void) const {
		return margin_;
	}

	void setMargin(int margin) {
		margin_ = margin;
	}

	const std::string& name(void) const {
		return name_;
	}

	virtual void run(void) {
		log_call();

		complete();
	}

	virtual void draw(OIIO::ImageBuf *buf, const GPXData &data) = 0;

protected:
	VideoWidget(GPX2Video &app, std::string &name)  
		: GPX2Video::Task(app)
		, app_(app) 
		, name_(name) {
		log_call();

		setAlign(AlignNone);
		setPosition(0, 0);
		setSize(64, 64);
		setMargin(10);
	}

	void add(OIIO::ImageBuf *frame, int x, int y, const char *picto, const char *label, const char *value, double divider);

	GPX2Video &app_;
	Align align_;
	int x_;
	int y_;
	int width_;
	int height_;
	int margin_;

private:
	std::string name_;
};

#endif

