#include "image.h"


/**
 * Text shape
 */

ImageWidget::ImageWidget(GPXApplication &app)
	: VideoWidget(app, VideoWidget::WidgetImage) 
	, ShapeBase(theme()) 
	, bg_buf_(NULL)
	, fg_buf_(NULL) 
	, mask_(NULL) {
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

	int width = theme().width();
	int height = theme().height();

    float r = theme().roundCorner();
    float r2 = r * r;

	(void) cr;

//	setSize(theme().height());
//
//	setPadding(
//		theme().border() + theme().padding(VideoWidget::Theme::PaddingLeft),
//		theme().border() + theme().padding(VideoWidget::Theme::PaddingRight),
//   		theme().border() + theme().padding(VideoWidget::Theme::PaddingTop),
//   		theme().border() + theme().padding(VideoWidget::Theme::PaddingBottom));

	if (mask_ != NULL)
		delete mask_;

	if (r > 0) {
		int w = width - 2 * theme().border();
		int h = height - 2 * theme().border();

		mask_ = new OIIO::ImageBuf(OIIO::ImageSpec(width, height, 4, OIIO::TypeDesc::FLOAT));

		for (int y = 0; y < h; y++) {
			for (int x = 0; x < w; x++) {
				float m;

				bool inside = true;

				// Check each corner
				if (x < r && y < r) // top-left
					inside = ((x - r) * (x - r) + (y - r) * (y - r)) <= r2;
				else if (x > (w - r) && y < r) // top-right
					inside = ((x - (w - r)) * (x - (w - r)) + (y - r) * (y - r)) <= r2;
				else if (x < r && y > (h - r)) // bottom-left
					inside = ((x - r) * (x - r) + (y - (h - r)) * (y - (h - r))) <= r2;
				else if (x > (w - r) && y > (h - r)) // bottom-right
					inside = ((x - (w - r)) * (x - (w - r)) + (y - (h - r)) * (y - (h - r))) <= r2;

				m = inside ? 1.0f : 0.0f;
				mask_->setpixel(x + theme().border(), y + theme().border(), { m, m, m, m });
			}
		}
	}
	else
		mask_ = NULL;

	is_initialized_ = true;
}


void ImageWidget::draw(cairo_t *cr, const TelemetryData &data) {
	(void) data;

	// Initialize
	initialize(cr);

	// Draw background
	background(cr, theme().roundCorner());
}


void ImageWidget::clear(void) {
	ShapeBase::clear();

	if (bg_buf_)
		delete bg_buf_;

	if (fg_buf_)
		delete fg_buf_;

	bg_buf_ = NULL;
	fg_buf_ = NULL;
}

