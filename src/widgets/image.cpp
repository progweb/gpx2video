#include "image.h"


/**
 * Text shape
 */

ImageWidget::ImageWidget(GPXApplication &app)
	: VideoWidget(app, VideoWidget::WidgetImage) 
	, ShapeBase(theme()) 
	, bg_buf_(NULL)
	, fg_buf_(NULL) {
	setShape(VideoWidget::ShapeNone);
}


bool ImageWidget::updated(const TelemetryData &data) const {
	if (is_initialized_)
		return false;

	(void) data;

	return true;
}


void ImageWidget::initialize(cairo_t *cr) {
	if (is_initialized_)
		return;

	(void) cr;

//	setSize(theme().height());
//
//	setPadding(
//		theme().border() + theme().padding(VideoWidget::Theme::PaddingLeft),
//		theme().border() + theme().padding(VideoWidget::Theme::PaddingRight),
//   		theme().border() + theme().padding(VideoWidget::Theme::PaddingTop),
//   		theme().border() + theme().padding(VideoWidget::Theme::PaddingBottom));

	is_initialized_ = true;
}


void ImageWidget::draw(cairo_t *cr, const TelemetryData &data) {
	(void) data;

	// Initialize
	initialize(cr);

	// Draw background
	background(cr);
}


void ImageWidget::clear(void) {
	is_initialized_ = false;

	if (bg_buf_)
		delete bg_buf_;

	if (fg_buf_)
		delete fg_buf_;

	bg_buf_ = NULL;
	fg_buf_ = NULL;
}

