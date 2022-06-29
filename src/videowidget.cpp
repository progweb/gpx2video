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


VideoWidget::Unit VideoWidget::string2unit(std::string &s) {
	VideoWidget::Unit unit;

	if (s.empty() || (s == "none"))
		unit = VideoWidget::UnitNone;
	else if (s == "mph")
		unit = VideoWidget::UnitMPH;
	else if (s == "kph")
		unit = VideoWidget::UnitKPH;
	else if (s == "km")
		unit = VideoWidget::UnitKm;
	else if (s == "m")
		unit = VideoWidget::UnitMeter;
	else if (s == "miles")
		unit = VideoWidget::UnitMiles;
	else
		unit = VideoWidget::UnitUnknown;

	return unit;
}


std::string VideoWidget::unit2string(VideoWidget::Unit unit) {
	switch (unit) {
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


bool VideoWidget::hex2color(float color[4], std::string hex) {
	if (hex.empty())
		return false;

	if (hex.at(0) == '#')
		hex.erase(0, 1);

	while (hex.length() < 8)
		hex += "0";

	color[0] = std::stoi(hex.substr(0, 2), NULL, 16) / 255.0;
	color[1] = std::stoi(hex.substr(2, 2), NULL, 16) / 255.0;
	color[2] = std::stoi(hex.substr(4, 2), NULL, 16) / 255.0;
	color[3] = std::stoi(hex.substr(6, 2), NULL, 16) / 255.0;

	return true;
}


void VideoWidget::createBox(OIIO::ImageBuf **buf, int width, int height) {
	// Create an image buffer with static render
	*buf = new OIIO::ImageBuf(OIIO::ImageSpec(width, height, 4, OIIO::TypeDesc::UINT8));
}


void VideoWidget::drawBorder(OIIO::ImageBuf *buf) {
	int i;
	int width, height;

	int border;
	float bordercolor[4];

	width = this->width() - 1;
	height = this->height() - 1;

	border = this->border();
	memcpy(bordercolor, this->borderColor(), sizeof(bordercolor));

	// Draw border
	if ((border > 0) && (bordercolor[3] != 0.0)) {
		for (i=0; i<border; i++)
			OIIO::ImageBufAlgo::render_box(*buf, i, i, width - i, height - i, bordercolor, false);
	}
}


void VideoWidget::drawBackground(OIIO::ImageBuf *buf) {
	int width, height;

	int border;
	float bgcolor[4];

	width = this->width() - 1;
	height = this->height() - 1;

	border = this->border();
	memcpy(bgcolor, this->backgroundColor(), sizeof(bgcolor));

	if (bgcolor[3] != 0.0)
		OIIO::ImageBufAlgo::render_box(*buf, border, border, width - border, height - border, bgcolor, true);
}


void VideoWidget::drawImage(OIIO::ImageBuf *buf, int x, int y, const char *name, VideoWidget::Zoom zoom) {
	double ratio;

	int width, height;

	// Open image
	auto img = OIIO::ImageInput::open(name);
	const OIIO::ImageSpec& spec = img->spec();
	VideoParams::Format img_fmt = OIIOUtils::getFormatFromOIIOBaseType((OIIO::TypeDesc::BASETYPE) spec.format.basetype);
	OIIO::TypeDesc::BASETYPE type = OIIOUtils::getOIIOBaseTypeFromFormat(img_fmt);

	OIIO::ImageBuf *b = new OIIO::ImageBuf(OIIO::ImageSpec(spec.width, spec.height, spec.nchannels, type)); //, OIIO::InitializePixels::No);
	img->read_image(type, b->localpixels());

	// Ratio
	ratio = spec.width / spec.height;

	// Compute new size
	switch(zoom) {
	case ZoomFit:
		width = this->width() - this->border() - x;
		height = this->height() - this->border() - y;

		if (width * spec.height > spec.width * height)
			width = height * ratio;
		else
			height = width / ratio;
		break;

	default:
		width = spec.width;
		height = spec.height;
		break;
	}

	// Resize picto
	OIIO::ImageBuf d(OIIO::ImageSpec(width, height, spec.nchannels, type)); //, OIIO::InitializePixels::No);
	OIIO::ImageBufAlgo::resize(d, *b);

	// Image over
	d.specmod().x = x;
	d.specmod().y = y;
	OIIO::ImageBufAlgo::over(*buf, d, *buf, OIIO::ROI());

	delete b;
}


void VideoWidget::drawText(OIIO::ImageBuf *buf, int x, int y, int pt, const char *label) {
	bool result;

	int padding = this->padding();

	// Add text (1 pt = 1.333 px)

	float color[4]; // = { 1.0, 1.0, 1.0, 1.0 };

	memcpy(color, this->textColor(), sizeof(color));

	result = OIIO::ImageBufAlgo::render_text(*buf, 
		x + padding, 
		y + padding, 
		label, 
		pt, "./assets/fonts/Helvetica.ttf", color, 
		OIIO::ImageBufAlgo::TextAlignX::Left, 
		OIIO::ImageBufAlgo::TextAlignY::Top, 
		this->textShadow());

	if (result == false)
		fprintf(stderr, "render label text error\n");
}


void VideoWidget::drawLabel(OIIO::ImageBuf *buf, int x, int y, const char *label) {
	bool result;

	int h;

	int border = this->border();
	int padding = this->padding();

	// Apply border
	x += border;
	y += border;

	// width x height
	h = this->height() - 2 * border;

	// Add label (1 pt = 1.333 px)
	// +-------------
	// |  Label    px
	// |  Value    2 * px
	// +-------------
	//        h = px + 2 * px + 2 * padding
	int px = h / 2 - padding;
	int pt = 3 * px / 4;

	float color[4]; // = { 1.0, 1.0, 1.0, 1.0 };

	memcpy(color, this->textColor(), sizeof(color));

	result = OIIO::ImageBufAlgo::render_text(*buf, 
		x + padding, 
		y + padding, 
		label, 
		pt, "./assets/fonts/Helvetica.ttf", color, 
		OIIO::ImageBufAlgo::TextAlignX::Left, 
		OIIO::ImageBufAlgo::TextAlignY::Top, 
		this->textShadow());

	if (result == false)
		fprintf(stderr, "render label text error\n");
}


void VideoWidget::drawValue(OIIO::ImageBuf *buf, int x, int y, const char *value) {
	bool result;

	int h;

	int border = this->border();
	int padding = this->padding();

	// Apply border
	x += border;
	y += border;

	// width x height
	h = this->height() - 2 * border;

	// Add label (1 pt = 1.333 px)
	// +-------------
	// |  Label    px
	// |  Value    2 * px
	// +-------------
	//        h = px + 2 * px + 2 * padding
	int px = h / 2 - padding;
	int pt = 3 * px / 4;

	float color[4]; // = { 1.0, 1.0, 1.0, 1.0 };

	memcpy(color, this->textColor(), sizeof(color));

	result = OIIO::ImageBufAlgo::render_text(*buf, 
		x + padding, 
		y + h - padding, 
		value, 
		2 * pt, "./assets/fonts/Helvetica.ttf", color, 
		OIIO::ImageBufAlgo::TextAlignX::Left, 
		OIIO::ImageBufAlgo::TextAlignY::Baseline, 
		this->textShadow());

	if (result == false)
		fprintf(stderr, "render value text error\n");
}

