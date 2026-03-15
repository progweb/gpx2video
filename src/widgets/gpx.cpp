#include "gpx.h"


/**
 * Text shape
 */

void GPXTextShape::initialize(void) {
	setSize(theme().height());

	setPadding(
		theme().border() + theme().padding(VideoWidget::Theme::PaddingLeft),
		theme().border() + theme().padding(VideoWidget::Theme::PaddingRight),
   		theme().border() + theme().padding(VideoWidget::Theme::PaddingTop),
   		theme().border() + theme().padding(VideoWidget::Theme::PaddingBottom));
}


void GPXTextShape::draw(cairo_t *cr, const TelemetryData &data) {
//		char s[128];
//
//		time_t t;
//		struct tm time;
//
//		int h;
//		int offset;
//
//		int border = theme().border();
//		int padding_yt = theme().padding(VideoWidget::Theme::PaddingTop);
//		int padding_yb = theme().padding(VideoWidget::Theme::PaddingBottom);
//
//		// width x height
////		w = theme().height() - 2 * border;
//		h = theme().height() - 2 * border;
//
//		// Add text (1 pt = 1.333 px)
//		// +-------------
//		// |  Text    px
//		// |  Text    px
//		// |  ....    ..
//		// +-------------
//		//          (with n nbr of lines)
//		//        h = n * px + padding_top + padding_bottom
//		int px = (h - padding_yt - padding_yb) / 6;

//		// title
//		offset = 0;
//		this->drawText(fg_buf_, 0, offset, px, "GPX WPT");
//
//		// time
//		offset += px;
//		t = data.timestamp() / 1000;
//		gmtime_r(&t, &time);
//		strftime(s, sizeof(s), "time: %H:%M:%S", &time);
//		this->drawText(fg_buf_, 0, offset, px, s);
//
//		// latitude
//		offset += px;
//		sprintf(s, "lat: %.4f", data.latitude());
//		this->drawText(fg_buf_, 0, offset, px, s);
//
//		// longitude
//		offset += px;
//		sprintf(s, "lon: %.4f", data.longitude());
//		this->drawText(fg_buf_, 0, offset, px, s);
//
//		// elevation
//		offset += px;
//		sprintf(s, "ele: %.4f", data.elevation());
//		this->drawText(fg_buf_, 0, offset, px, s);
//
//		// line
//		offset += px;
//		sprintf(s, "line: %d", data.line());
//		this->drawText(fg_buf_, 0, offset, px, s);
}

