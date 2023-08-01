#ifndef __GPX2VIDEO__WIDGETS__GPX_H__
#define __GPX2VIDEO__WIDGETS__GPX_H__

#include "log.h"
#include "videowidget.h"


class GPXWidget : public VideoWidget {
public:
	virtual ~GPXWidget() {
		log_call();

		if (bg_buf_)
			delete bg_buf_;

		if (fg_buf_)
			delete fg_buf_;
	}

	static GPXWidget * create(GPX2Video &app) {
		GPXWidget *widget;

		log_call();

		widget = new GPXWidget(app, "gpx");

		return widget;
	}

	OIIO::ImageBuf * prepare(void) {
		if (bg_buf_ == NULL) {
			this->createBox(&bg_buf_, this->width(), this->height());
			this->drawBorder(bg_buf_);
			this->drawBackground(bg_buf_);
//			this->drawLabel(bg_buf_, 0, 0, label().c_str());
		}

		return bg_buf_;
//		// Image over
//		bg_buf_->specmod().x = this->x();
//		bg_buf_->specmod().y = this->y();
//		OIIO::ImageBufAlgo::over(*buf, *bg_buf_, *buf, OIIO::ROI());
	}

	OIIO::ImageBuf * render(const GPXData &data) {
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

		// Refresh dynamic info
		if ((fg_buf_ != NULL) && (data.type() == GPXData::TypeUnchanged))
			goto skip;

		// Format data
		space_x = padding_x + border;
		space_y = padding_yt + border;

		// Refresh dynamic info
		if (fg_buf_ != NULL)
			delete fg_buf_;

		this->createBox(&fg_buf_, this->width(), this->height());

		// title
		offset = 0;
		this->drawText(fg_buf_, this->x() + space_x, this->y() + space_y + offset, pt, "GPX WPT");

		// time
		offset += px;
		gmtime_r(&data.time(GPXData::PositionPrevious), &time);
		strftime(s, sizeof(s), "time: %H:%M:%S", &time);
		this->drawText(fg_buf_, this->x() + space_x, this->y() + space_y + offset, pt, s);

		// latitude
		offset += px;
		sprintf(s, "lat: %.4f", position.lat);
		this->drawText(fg_buf_, this->x() + space_x, this->y() + space_y + offset, pt, s);

		// longitude
		offset += px;
		sprintf(s, "lon: %.4f", position.lon);
		this->drawText(fg_buf_, this->x() + space_x, this->y() + space_y + offset, pt, s);

		// elevation
		offset += px;
		sprintf(s, "ele: %.4f", data.elevation(GPXData::PositionPrevious));
		this->drawText(fg_buf_, this->x() + space_x, this->y() + space_y + offset, pt, s);

		// line
		offset += px;
		sprintf(s, "line: %d", data.line());
		this->drawText(fg_buf_, this->x() + space_x, this->y() + space_y + offset, pt, s);

skip:
		return fg_buf_;
	}

private:
	OIIO::ImageBuf *bg_buf_;
	OIIO::ImageBuf *fg_buf_;

	GPXWidget(GPX2Video &app, std::string name)
		: VideoWidget(app, name)
   		, bg_buf_(NULL)
   		, fg_buf_(NULL) {
	}
};

#endif

