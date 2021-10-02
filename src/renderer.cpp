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
	int x, y; //, w, h;

	OIIO::ImageBuf frame_buffer = frame->toImageBuf();

	// Open picto
//	OIIO::ImageInput *img = OIIO::ImageInput::open("./assets/picto.png");
	auto img = OIIO::ImageInput::open("./assets/picto.png");
	const OIIO::ImageSpec& spec = img->spec();
	VideoParams::Format img_fmt = OIIOUtils::getFormatFromOIIOBaseType((OIIO::TypeDesc::BASETYPE) spec.format.basetype);
	OIIO::TypeDesc::BASETYPE type = OIIOUtils::getOIIOBaseTypeFromFormat(img_fmt);

	OIIO::ImageBuf *buf = new OIIO::ImageBuf(OIIO::ImageSpec(spec.width, spec.height, spec.nchannels, type)); //, OIIO::InitializePixels::No);
	img->read_image(type, buf->localpixels());

	x = 50;
	y = 900;
//	w = spec.width;
//	h = spec.height;

	// Image over
	buf->specmod().x = x;
	buf->specmod().y = y;
	OIIO::ImageBufAlgo::over(frame_buffer, *buf, frame_buffer, OIIO::ROI());

	delete buf;

	frame->fromImageBuf(frame_buffer);
}


