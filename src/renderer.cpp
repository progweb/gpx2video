#include <iostream>
#include <memory>

#include <OpenImageIO/imageio.h>
#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>

#include "oiioutils.h"
#include "renderer.h"


Renderer::Renderer() {
}


Renderer::~Renderer() {
}


Renderer * Renderer::create(void) {
	Renderer *renderer = new Renderer();

	return renderer;
}


void Renderer::draw(FramePtr frame) {
	int x, y, w, h;

	OIIO::ImageBuf frame_buffer = frame->toImageBuf();

	// Open picto
//	OIIO::ImageInput *img = OIIO::ImageInput::open("./assets/picto.png");
	auto img = OIIO::ImageInput::open("./assets/picto/DataOverlay_icn_speed.png");
	const OIIO::ImageSpec& spec = img->spec();
	VideoParams::Format img_fmt = OIIOUtils::getFormatFromOIIOBaseType((OIIO::TypeDesc::BASETYPE) spec.format.basetype);
	OIIO::TypeDesc::BASETYPE type = OIIOUtils::getOIIOBaseTypeFromFormat(img_fmt);

	OIIO::ImageBuf *buf = new OIIO::ImageBuf(OIIO::ImageSpec(spec.width, spec.height, spec.nchannels, type)); //, OIIO::InitializePixels::No);
	img->read_image(type, buf->localpixels());

	// Resize picto
	OIIO::ImageBuf dst(OIIO::ImageSpec(spec.width * 2.5, spec.height * 2.5, spec.nchannels, type)); //, OIIO::InitializePixels::No);
	OIIO::ImageBufAlgo::resize(dst, *buf);

	x = 50;
	y = 900;
	w = spec.width * 2.5;
	h = spec.height * 2.5;

	// Image over
	dst.specmod().x = x;
	dst.specmod().y = y;
	OIIO::ImageBufAlgo::over(frame_buffer, dst, frame_buffer, OIIO::ROI());

	delete buf;


	// Add label
	int pt;
	int space = h / 10;
	pt = (h - (3 *space)) / 3;
	float white[] = { 1.0, 1.0, 1.0, 1.0 };
	if (OIIO::ImageBufAlgo::render_text(frame_buffer, x + w + (w/10), y + space + pt, "VITESSE", pt, "./assets/fonts/Helvetica.ttf", white) == false)
		fprintf(stderr, "render text error\n");
	pt *= 2;
	if (OIIO::ImageBufAlgo::render_text(frame_buffer, x + w + (w/10), y + (pt/2) + space + pt + space, "32 km/h", pt, "./assets/fonts/Helvetica.ttf", white) == false)
		fprintf(stderr, "render text error\n");


	frame->fromImageBuf(frame_buffer);
}


