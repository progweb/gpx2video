#include "log.h"
#include "base.h"


void ShapeBase::createBox(OIIO::ImageBuf **buf, int width, int height) {
	// Create an image buffer with static render
	*buf = new OIIO::ImageBuf(OIIO::ImageSpec(width, height, 4, OIIO::TypeDesc::UINT8));
}


void ShapeBase::drawBorder(OIIO::ImageBuf *buf) {
	int i;
	int width, height;

	int border;
	float bordercolor[4];

	width = widget_->width() - 1;
	height = widget_->height() - 1;

	border = widget_->border();
	memcpy(bordercolor, widget_->borderColor(), sizeof(bordercolor));

	// Draw border
	if ((border > 0) && (bordercolor[3] != 0.0)) {
		for (i=0; i<border; i++)
			OIIO::ImageBufAlgo::render_box(*buf, i, i, width - i, height - i, bordercolor, false);
	}
}


void ShapeBase::drawBackground(OIIO::ImageBuf *buf) {
	int width, height;

	int border;
	float bgcolor[4];

	width = widget_->width() - 1;
	height = widget_->height() - 1;

	border = widget_->border();
	memcpy(bgcolor, widget_->backgroundColor(), sizeof(bgcolor));

	if (bgcolor[3] != 0.0)
		OIIO::ImageBufAlgo::render_box(*buf, border, border, width - border, height - border, bgcolor, true);
}


void ShapeBase::drawText(OIIO::ImageBuf *buf, int x, int y, int px, const char *label) {
	bool result;

	int x1, y1, x2, y2;
	int text_width, text_height;

	int border = widget_->border();
	int padding_xl = widget_->padding(VideoWidget::PaddingLeft);
//	int padding_xr = widget_->padding(VideoWidget::PaddingRight);
	int padding_yt = widget_->padding(VideoWidget::PaddingTop);
//	int padding_yb = widget_->padding(VideoWidget::PaddingBottom);

	float color[4]; // = { 1.0, 1.0, 1.0, 1.0 };

	// Compute text size
	this->textSize(label, px,
			x1, y1, x2, y2,
			text_width, text_height);

	// Text color
	memcpy(color, widget_->textColor(), sizeof(color));

	// Text offset
	x += -x1;
	y += -y1 + widget_->textShadow();

	// Text position
	x += padding_xl;
	x += border + widget_->textShadow();

	y += border + padding_yt;

	result = OIIO::ImageBufAlgo::render_text_shadow(*buf, 
		x, 
		y, 
		label, 
		px, widget_->font(), color, 
		OIIO::ImageBufAlgo::TextAlignX::Left, 
		OIIO::ImageBufAlgo::TextAlignY::Baseline, 
		widget_->textShadow());

	if (result == false)
		fprintf(stderr, "render text error\n");
}


void ShapeBase::drawImage(OIIO::ImageBuf *buf, int x, int y, const char *name, VideoWidget::Zoom zoom) {
	bool ok;

	double ratio;

	int width, height;
	int max_width, max_height;

	if ((name == NULL) || (name[0] == '\0'))
		return;

	// Open image
	auto img = OIIO::ImageInput::open(name);
	const OIIO::ImageSpec& spec = img->spec();
	VideoParams::Format img_fmt = OIIOUtils::getFormatFromOIIOBaseType((OIIO::TypeDesc::BASETYPE) spec.format.basetype);
	OIIO::TypeDesc::BASETYPE type = OIIOUtils::getOIIOBaseTypeFromFormat(img_fmt);

	OIIO::ImageBuf *inbuf = new OIIO::ImageBuf(OIIO::ImageSpec(spec.width, spec.height, spec.nchannels, type)); //, OIIO::InitializePixels::No);
	ok = img->read_image(img->current_subimage(), img->current_miplevel(), 0, -1, type, inbuf->localpixels());

	if (!ok)
		log_warn("Read '%s' image (%dx%d) failure!", name, spec.width, spec.height);

	// Input image ratio
	ratio = (double) spec.width / (double) spec.height;

	// Compute new size
	switch (zoom) {
	case VideoWidget::ZoomFit:
		width = widget_->width() - widget_->border() - x;
		height = widget_->height() - widget_->border() - y;

		if (width * spec.height > spec.width * height)
			width = height * ratio;
		else
			height = width / ratio;
		break;

	case VideoWidget::ZoomFill:
		width = widget_->width() - widget_->border() - x;
		height = widget_->height() - widget_->border() - y;

		if (width * spec.height < spec.width * height)
			width = height * ratio;
		else
			height = width / ratio;
		break;

	case VideoWidget::ZoomStretch:
		width = widget_->width() - widget_->border() - x;
		height = widget_->height() - widget_->border() - y;
		break;

	case VideoWidget::ZoomCrop:
	default:
		width = spec.width;
		height = spec.height;
		break;
	}

	// Max width & height
	max_width = widget_->width() - (2 * widget_->border());
	max_height = widget_->height() - (2 * widget_->border());

	// Resize picto
	OIIO::ImageBuf outbuf(OIIO::ImageSpec(width, height, spec.nchannels, type)); //, OIIO::InitializePixels::No);
	OIIO::ImageBufAlgo::resize(outbuf, *inbuf);

	// Add alpha channel
	if (spec.nchannels != 4) {
		int channelorder[] = { 0, 1, 2, -1 /*use a float value*/ };
		float channelvalues[] = { 0 /*ignore*/, 0 /*ignore*/, 0 /*ignore*/, 1.0 };
		std::string channelnames[] = { "", "", "", "A" };

		outbuf = OIIO::ImageBufAlgo::channels(outbuf, 4, channelorder, channelvalues, channelnames);
	}

	// Image over
	outbuf.specmod().x = x;
	outbuf.specmod().y = y;
	OIIO::ImageBufAlgo::over(*buf, outbuf, *buf, OIIO::ROI(x, x + max_width, y, y + max_height));

	delete inbuf;
}


bool ShapeBase::textSize(std::string text, int fontsize, 
	int &x1, int &y1, int &x2, int &y2,
	int &width, int &height) {

	OIIO::ROI roi = OIIO::ImageBufAlgo::text_size(text, fontsize, widget_->font());

	x1 = roi.xbegin;
	x2 = roi.xend;
	y1 = roi.ybegin;
	y2 = roi.yend;

	width = roi.width();
	height = roi.height();

	return roi.defined();
}

cairo_t * ShapeBase::createCairoContext(OIIO::ImageBuf *buf) {
	// Create the cairo destination surface
	cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, buf->spec().width, buf->spec().height);

	// Cairo context
	cairo_t *cairo = cairo_create(surface);

	return cairo;
}


void ShapeBase::renderCairoContext(OIIO::ImageBuf *buf, cairo_t *cairo) {
	int stride;
	unsigned char *data;

	cairo_surface_t *surface = cairo_get_target(cairo);

	data = cairo_image_surface_get_data(surface);
	stride = cairo_image_surface_get_stride(surface);

	// Cairo to OIIO
	buf->set_pixels(OIIO::ROI(),
		buf->spec().format,
		data, 
		OIIO::AutoStride,
		stride);

	// BGRA => RGBA
	int channelorder[] = { 2, 1, 0, 3 };
	float channelvalues[] = { };
	std::string channelnames[] = { "B", "G", "R", "A" };

	OIIO::ImageBufAlgo::channels(*buf, *buf, 4, channelorder, channelvalues, channelnames);
}


void ShapeBase::destroyCairoContext(cairo_t *cairo) {
	cairo_surface_t *surface = cairo_get_target(cairo);

	cairo_surface_destroy(surface);
	cairo_destroy(cairo);
}


