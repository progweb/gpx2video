#include "image.h"


/**
 * Text shape
 */

void ImageWidget::initialize(void) {
//	setSize(theme().height());
//
//	setPadding(
//		theme().border() + theme().padding(VideoWidget::Theme::PaddingLeft),
//		theme().border() + theme().padding(VideoWidget::Theme::PaddingRight),
//   		theme().border() + theme().padding(VideoWidget::Theme::PaddingTop),
//   		theme().border() + theme().padding(VideoWidget::Theme::PaddingBottom));
}


void ImageWidget::draw(cairo_t *cr, const TelemetryData &data) {
	(void) cr;
	(void) data;
}

