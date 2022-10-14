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
		if (buf_ == NULL) {
			this->createBox(&buf_, this->width(), this->height());
			this->drawBorder(buf_);
			this->drawBackground(buf_);
//			this->drawLabel(buf_, 0, 0, label().c_str());
		}

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

		int space_x, space_y;
		int border = this->border();
		int padding_x = this->padding(VideoWidget::PaddingLeft);
		int padding_yt = this->padding(VideoWidget::PaddingTop);
		int padding_yb = this->padding(VideoWidget::PaddingBottom);

		// width x height
//		w = this->height() - 2 * border;
		h = this->height() - 2 * border;

		// Add text (1 pt = 1.333 px)
		// +-------------
		// |  Text    px
		// |  Text    px
		// |  ....    ..
		// +-------------
		//          (with n nbr of lines)
		//        h = n * px + padding_top + padding_bottom
		int px = (h - padding_yt - padding_yb) / 6;
		int pt = 3 * px / 4;

		struct GPXData::point position = data.position(GPXData::PositionPrevious);

		space_x = padding_x + border;
		space_y = padding_yt + border;

		// Append dynamic info
		offset = 0;
		this->drawText(buf, this->x() + space_x, this->y() + space_y + offset, pt, "GPX WPT");

		// time
		offset += px;
		gmtime_r(&data.time(GPXData::PositionPrevious), &time);
		strftime(s, sizeof(s), "time: %H:%M:%S", &time);
		this->drawText(buf, this->x() + space_x, this->y() + space_y + offset, pt, s);

		// latitude
		offset += px;
		sprintf(s, "lat: %.4f", position.lat);
		this->drawText(buf, this->x() + space_x, this->y() + space_y + offset, pt, s);

		// longitude
		offset += px;
		sprintf(s, "lon: %.4f", position.lon);
		this->drawText(buf, this->x() + space_x, this->y() + space_y + offset, pt, s);

		// elevation
		offset += px;
		sprintf(s, "ele: %.4f", data.elevation(GPXData::PositionPrevious));
		this->drawText(buf, this->x() + space_x, this->y() + space_y + offset, pt, s);

		// line
		offset += px;
		sprintf(s, "line: %d", data.line());
		this->drawText(buf, this->x() + space_x, this->y() + space_y + offset, pt, s);
	}

private:
	OIIO::ImageBuf *buf_;

	GPXWidget(GPX2Video &app, std::string name)
		: VideoWidget(app, name)
   		, buf_(NULL) {
	}
};

#endif

