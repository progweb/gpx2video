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

	static GPXWidget * create(GPXApplication &app) {
		GPXWidget *widget;

		log_call();

		widget = new GPXWidget(app, "gpx");

		return widget;
	}

	OIIO::ImageBuf * prepare(bool &is_update) {
		if (bg_buf_ != NULL) {
			is_update = false;
			goto skip;
		}

		this->createBox(&bg_buf_, this->width(), this->height());
		this->drawBorder(bg_buf_);
		this->drawBackground(bg_buf_);
//		this->drawLabel(bg_buf_, label().c_str());
	
		is_update = true;
skip:
		return bg_buf_;
	}

	OIIO::ImageBuf * render(const TelemetryData &data, bool &is_update) {
		char s[128];

		time_t t;
		struct tm time;

		int h;
		int offset;

		int border = this->border();
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

		// Refresh dynamic info
		if ((fg_buf_ != NULL) && (data.type() == TelemetryData::TypeUnchanged)) {
			is_update = true;
			goto skip;
		}

		// Refresh dynamic info
		if (fg_buf_ != NULL)
			delete fg_buf_;

		this->createBox(&fg_buf_, this->width(), this->height());

		// title
		offset = 0;
		this->drawText(fg_buf_, 0, offset, px, "GPX WPT");

		// time
		offset += px;
		t = data.timestamp() / 1000;
		gmtime_r(&t, &time);
		strftime(s, sizeof(s), "time: %H:%M:%S", &time);
		this->drawText(fg_buf_, 0, offset, px, s);

		// latitude
		offset += px;
		sprintf(s, "lat: %.4f", data.latitude());
		this->drawText(fg_buf_, 0, offset, px, s);

		// longitude
		offset += px;
		sprintf(s, "lon: %.4f", data.longitude());
		this->drawText(fg_buf_, 0, offset, px, s);

		// elevation
		offset += px;
		sprintf(s, "ele: %.4f", data.elevation());
		this->drawText(fg_buf_, 0, offset, px, s);

		// line
		offset += px;
		sprintf(s, "line: %d", data.line());
		this->drawText(fg_buf_, 0, offset, px, s);

		is_update = true;
skip:
		return fg_buf_;
	}

private:
	OIIO::ImageBuf *bg_buf_;
	OIIO::ImageBuf *fg_buf_;

	GPXWidget(GPXApplication &app, std::string name)
		: VideoWidget(app, name)
   		, bg_buf_(NULL)
   		, fg_buf_(NULL) {
	}
};

#endif

