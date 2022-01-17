#include <iostream>
#include <memory>

#include <OpenImageIO/imageio.h>
#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>

#include "oiioutils.h"
#include "videowidget.h"


void VideoWidget::add(OIIO::ImageBuf *frame, int x, int y, const char *picto, const char *label, const char *value, double divider) {
	int w, h;

	// Open picto
	auto img = OIIO::ImageInput::open(picto);
	const OIIO::ImageSpec& spec = img->spec();
	VideoParams::Format img_fmt = OIIOUtils::getFormatFromOIIOBaseType((OIIO::TypeDesc::BASETYPE) spec.format.basetype);
	OIIO::TypeDesc::BASETYPE type = OIIOUtils::getOIIOBaseTypeFromFormat(img_fmt);

	OIIO::ImageBuf *buf = new OIIO::ImageBuf(OIIO::ImageSpec(spec.width, spec.height, spec.nchannels, type)); //, OIIO::InitializePixels::No);
	img->read_image(type, buf->localpixels());

	// Resize picto
	OIIO::ImageBuf dst(OIIO::ImageSpec(spec.width * divider, spec.height * divider, spec.nchannels, type)); //, OIIO::InitializePixels::No);
	OIIO::ImageBufAlgo::resize(dst, *buf);

//	x = 50;
//	y = 900;
	w = spec.width * divider;
	h = spec.height * divider;

	// Image over
	dst.specmod().x = x;
	dst.specmod().y = y;
	OIIO::ImageBufAlgo::over(*frame, dst, *frame, OIIO::ROI());

	delete buf;

	// Add label
	int pt;
	int space = h / 10;
	pt = (h - 3 * space) / 3;
	float white[] = { 1.0, 1.0, 1.0, 1.0 };
	if (OIIO::ImageBufAlgo::render_text(*frame, x + w + (w/10), y + space + pt, label, pt, "./assets/fonts/Helvetica.ttf", white) == false)
		fprintf(stderr, "render label text error\n");
	pt *= 2;
	if (OIIO::ImageBufAlgo::render_text(*frame, x + w + (w/10), y + h - (pt/2), value, pt, "./assets/fonts/Helvetica.ttf", white) == false)
		fprintf(stderr, "render value text error\n");
}
