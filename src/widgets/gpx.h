#ifndef __GPX2VIDEO__WIDGETS__GPX_H__
#define __GPX2VIDEO__WIDGETS__GPX_H__

#include "log.h"
#include "videowidget.h"


class GPXWidget : public VideoWidget {
public:
	virtual ~GPXWidget() {
		log_call();

		if (buf_)
			delete buf_;
	}

	static GPXWidget * create(GPX2Video &app) {
		GPXWidget *widget;

		log_call();

		widget = new GPXWidget(app, "gpx");

		return widget;
	}

	void prepare(OIIO::ImageBuf *buf) {
		this->createBox(&buf_, this->width(), this->height());
		this->drawBorder(buf_);
		this->drawBackground(buf_);
//		this->drawLabel(buf_, 0, 0, label().c_str());
//		this->drawValue(buf_, 0, 0, "22 km");

		// Image over
		buf_->specmod().x = this->x();
		buf_->specmod().y = this->y();
		OIIO::ImageBufAlgo::over(*buf, *buf_, *buf, OIIO::ROI());
	}

	void render(OIIO::ImageBuf *buf, const GPXData &data) {
		char s[128];

		struct tm time;

		int h;
		int offset;

		int space;
		int border = this->border();
		int padding = this->padding();

		// width x height
//		w = this->height() - 2 * border;
		h = this->height() - 2 * border;

		// Add text (1 pt = 1.333 px)
		// +-------------
		// |  Text    px
		// |  Text    px
		// |  ....    ..
		// +-------------
		//        h = n * px + 2 * padding
		//          (with n nbr of lines)
		int px = (h  - (2 * padding)) / 6;
		int pt = 3 * px / 4;

		struct GPXData::point position = data.position(GPXData::PositionPrevious);

		space = padding + border;

		// Append dynamic info
		offset = 0;
		this->drawText(buf, this->x() + space, this->y() + space + offset, pt, "GPX WPT");

		// time
		offset += px;
		gmtime_r(&data.time(GPXData::PositionPrevious), &time);
		strftime(s, sizeof(s), "time: %H:%M:%S", &time);
		this->drawText(buf, this->x() + space, this->y() + space + offset, pt, s);

		// latitude
		offset += px;
		sprintf(s, "lat: %.4f", position.lat);
		this->drawText(buf, this->x() + space, this->y() + space + offset, pt, s);

		// longitude
		offset += px;
		sprintf(s, "lon: %.4f", position.lon);
		this->drawText(buf, this->x() + space, this->y() + space + offset, pt, s);

		// elevation
		offset += px;
		sprintf(s, "ele: %.4f", data.elevation(GPXData::PositionPrevious));
		this->drawText(buf, this->x() + space, this->y() + space + offset, pt, s);

		// line
		offset += px;
		sprintf(s, "line: %d", data.line());
		this->drawText(buf, this->x() + space, this->y() + space + offset, pt, s);
	}

private:
	OIIO::ImageBuf *buf_;

	GPXWidget(GPX2Video &app, std::string name)
		: VideoWidget(app, name)
   		, buf_(NULL) {
	}
};

#endif

