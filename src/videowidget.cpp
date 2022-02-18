#include <iostream>
#include <memory>

#include <OpenImageIO/imageio.h>
#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>

#include "oiioutils.h"
#include "videowidget.h"


VideoWidget::Align VideoWidget::string2align(std::string &s) {
	VideoWidget::Align align;

	if (s.empty() || (s == "none"))
		align = VideoWidget::AlignNone;
	else if (s == "left")
		align = VideoWidget::AlignLeft;
	else if (s == "right")
		align = VideoWidget::AlignRight;
	else if (s == "bottom")
		align = VideoWidget::AlignBottom;
	else if (s == "top")
		align = VideoWidget::AlignTop;
	else if (s == "bottom-left")
		align = VideoWidget::AlignBottomLeft;
	else if (s == "bottom-right")
		align = VideoWidget::AlignBottomRight;
	else if (s == "top-left")
		align = VideoWidget::AlignTopLeft;
	else if (s == "top-right")
		align = VideoWidget::AlignTopRight;
	else
		align = VideoWidget::AlignUnknown;

	return align;
}


VideoWidget::Units VideoWidget::string2units(std::string &s) {
	VideoWidget::Units units;

	if (s.empty() || (s == "none"))
		units = VideoWidget::UnitNone;
	else if (s == "mph")
		units = VideoWidget::UnitMPH;
	else if (s == "kph")
		units = VideoWidget::UnitKPH;
	else if (s == "km")
		units = VideoWidget::UnitKm;
	else if (s == "m")
		units = VideoWidget::UnitMeter;
	else if (s == "miles")
		units = VideoWidget::UnitMiles;
	else
		units = VideoWidget::UnitUnknown;

	return units;
}


std::string VideoWidget::units2string(VideoWidget::Units units) {
	switch (units) {
	case VideoWidget::UnitMPH:
		return "m/h";
	case VideoWidget::UnitKPH:
		return "km/h";
	case VideoWidget::UnitKm:
		return "km";
	case VideoWidget::UnitMeter:
		return "m";
	case VideoWidget::UnitMiles:
		return "miles";
	case VideoWidget::UnitNone:
	case VideoWidget::UnitUnknown:
	default:
		return "";
	}
}


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

	w = spec.width * divider;
	h = spec.height * divider;

	// Image over
	dst.specmod().x = x;
	dst.specmod().y = y;
	OIIO::ImageBufAlgo::over(*frame, dst, *frame, OIIO::ROI());

	delete buf;

	// Add label (1 pt = 1.333 px)
	// +-------------
	// |  Label    px
	// |  Value    2 * px
	// +-------------
	//        h = px + 2 * px + 2 * padding
	int px = h / 2 - padding();
	int pt = 3 * px / 4;

	float white[] = { 1.0, 1.0, 1.0, 1.0 };

	OIIO::ROI roi = OIIO::ImageBufAlgo::text_size(label, pt, "./assets/fonts/Helvetica.ttf");

	if (OIIO::ImageBufAlgo::render_text(*frame, x + w + (w/10) + padding(), y + roi.height() + padding(), label, pt, "./assets/fonts/Helvetica.ttf", white) == false)
		fprintf(stderr, "render label text error\n");

	if (OIIO::ImageBufAlgo::render_text(*frame, x + w + (w/10) + padding(), y + h - padding(), value, 2 * pt, "./assets/fonts/Helvetica.ttf", white) == false)
		fprintf(stderr, "render value text error\n");
}

