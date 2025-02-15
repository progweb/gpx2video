#include <iostream>
#include <memory>

#include <OpenImageIO/imageio.h>
#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>

#include "oiio.h"
#include "oiioutils.h"
#include "videowidget.h"



VideoWidget::Position VideoWidget::string2position(std::string &s) {
	VideoWidget::Position position;

	if (s.empty() || (s == "none"))
		position = VideoWidget::PositionNone;
	else if (s == "left")
		position = VideoWidget::PositionLeft;
	else if (s == "right")
		position = VideoWidget::PositionRight;
	else if (s == "bottom")
		position = VideoWidget::PositionBottom;
	else if (s == "top")
		position = VideoWidget::PositionTop;
	else if ((s == "bottom-left") || (s == "left-bottom"))
		position = VideoWidget::PositionBottomLeft;
	else if ((s == "bottom-right") || (s == "right-bottom"))
		position = VideoWidget::PositionBottomRight;
	else if ((s == "top-left") || (s == "left-top"))
		position = VideoWidget::PositionTopLeft;
	else if ((s == "top-right") || (s == "right-top"))
		position = VideoWidget::PositionTopRight;
	else
		position = VideoWidget::PositionUnknown;

	return position;
}


VideoWidget::Align VideoWidget::string2align(std::string &s) {
	VideoWidget::Align align;

	if (s.empty() || (s == "none"))
		align = VideoWidget::AlignNone;
	else if (s == "horizontal")
		align = VideoWidget::AlignHorizontal;
	else if (s == "vertical")
		align = VideoWidget::AlignVertical;
	else
		align = VideoWidget::AlignUnknown;

	return align;
}


VideoWidget::TextAlign VideoWidget::string2textAlign(std::string &s) {
	VideoWidget::TextAlign align;

	if (s.empty() || (s == "none") || (s == "left"))
		align = VideoWidget::TextAlignLeft;
	else if (s == "center")
		align = VideoWidget::TextAlignCenter;
	else if (s == "right")
		align = VideoWidget::TextAlignRight;
	else
		align = VideoWidget::TextAlignUnknown;

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
	else if (s == "ft")
		unit = VideoWidget::UnitFoot;
	else if (s == "miles")
		unit = VideoWidget::UnitMiles;
	else if ((s == "C") || (s == "celsius"))
		unit = VideoWidget::UnitCelsius;
	else if ((s == "F") || (s == "farenheit"))
		unit = VideoWidget::UnitFarenheit;
	else if ((s == "g") || (s == "G"))
		unit = VideoWidget::UnitG;
	else if (s == "mps2")
		unit = VideoWidget::UnitMeterPS2;
	else
		unit = VideoWidget::UnitUnknown;

	return unit;
}


VideoWidget::Zoom VideoWidget::string2zoom(std::string &s) {
	VideoWidget::Zoom zoom;

	if (s.empty() || (s == "none"))
		zoom = VideoWidget::ZoomNone;
	else if (s == "fit")
		zoom = VideoWidget::ZoomFit;
	else if (s == "fill")
		zoom = VideoWidget::ZoomFill;
	else if (s == "crop")
		zoom = VideoWidget::ZoomCrop;
	else if (s == "stretch")
		zoom = VideoWidget::ZoomStretch;
	else
		zoom = VideoWidget::ZoomUnknown;

	return zoom;
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
	case VideoWidget::UnitFoot:
		return "ft";
	case VideoWidget::UnitMiles:
		return "miles";
	case VideoWidget::UnitCelsius:
		return "°C";
	case VideoWidget::UnitFarenheit:
		return "F";
	case VideoWidget::UnitG:
		return "g";
	case VideoWidget::UnitMeterPS2:
		return "m/s²";
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


void VideoWidget::dump(void) {
	log_call();

	printf("[%s] x: %d / y: %d - size: %dx%d\n", 
			name().c_str(), x(), y(), width(), height());
}


void VideoWidget::initialize(void) {
	bool ok;

	int height;

	int px;
	int x1, y1, x2, y2;
	int text_width, text_height;

	double ratio = this->textRatio();

	int border = this->border();
	int padding_yt = this->padding(VideoWidget::PaddingTop);
	int padding_yb = this->padding(VideoWidget::PaddingBottom);

	bool with_label = this->hasFlag(VideoWidget::FlagLabel);
	bool with_value = this->hasFlag(VideoWidget::FlagValue);

	const char *text = "0123456789/";

	// Compute font size (1 pt = 1.333 px) for "label" text
	// +-----------------------+  /  no label:
	// |  Label    px          |  /  +-------------------+
	// |  Value    ratio * px  |  /  |  Value     px     |
	// +-----------------------+  /  +-------------------+
	//        h = px + ratio * px + padding_top + padding_bottom
	if (with_label && !label().empty()) {
		px = this->height() - 2 * border - padding_yt - this->textLineSpace() - padding_yb;
		px = (int) floor((double) px / (1.0 + ratio));
		// pt = 3 * px / 4;

		for (height=px;; px += 1) {
			this->textSize(label().c_str(), px,
					x1, y1, x2, y2,
					text_width, text_height);

			if (text_height > height)
				break;

			label_px_ = px;
			label_size_ = text_height;
		}
	}

	// Compute font size (1 pt = 1.333 px) for "value" text
	// +-------------------+  /  no label:
	// |  Label    px      |  /  +-------------------+
	// |  Value    2 * px  |  /  |  Value     px     |
	// +-------------------+  /  +-------------------+
	//        h = px + 2 * px + padding_top + padding_bottom
	if (with_value) {
		px = this->height() - 2 * border - padding_yt - padding_yb;
		px = with_label ? (int) floor((double) (px - this->textLineSpace()) * ratio / (1.0 + ratio)) : px;
		// pt = 3 * px / 4;

		for (height=px;; px += 1) {
			ok = this->textSize(text, px,
					x1, y1, x2, y2,
					text_width, text_height);

			if (!ok) {
				printf("ERROR: %s %d %d", text, height, px);
				sleep(2);
			}

			if (text_height > height)
				break;

			value_px_ = px;
			value_size_ = text_height;
			value_offset_ = y1;
		}
	}
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
	case ZoomFit:
		width = this->width() - this->border() - x;
		height = this->height() - this->border() - y;

		if (width * spec.height > spec.width * height)
			width = height * ratio;
		else
			height = width / ratio;
		break;

	case ZoomFill:
		width = this->width() - this->border() - x;
		height = this->height() - this->border() - y;

		if (width * spec.height < spec.width * height)
			width = height * ratio;
		else
			height = width / ratio;
		break;

	case ZoomStretch:
		width = this->width() - this->border() - x;
		height = this->height() - this->border() - y;
		break;

	case ZoomCrop:
	default:
		width = spec.width;
		height = spec.height;
		break;
	}

	// Max width & height
	max_width = this->width() - (2 * this->border());
	max_height = this->height() - (2 * this->border());

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


void VideoWidget::drawText(OIIO::ImageBuf *buf, int x, int y, int px, const char *label) {
	bool result;

	int x1, y1, x2, y2;
	int text_width, text_height;

	int border = this->border();
	int padding_xl = this->padding(VideoWidget::PaddingLeft);
//	int padding_xr = this->padding(VideoWidget::PaddingRight);
	int padding_yt = this->padding(VideoWidget::PaddingTop);
//	int padding_yb = this->padding(VideoWidget::PaddingBottom);

	float color[4]; // = { 1.0, 1.0, 1.0, 1.0 };

	// Compute text size
	this->textSize(label, px,
			x1, y1, x2, y2,
			text_width, text_height);

	// Text color
	memcpy(color, this->textColor(), sizeof(color));

	// Text offset
	x += -x1;
	y += -y1 + this->textShadow();

	// Text position
	x += padding_xl;
	x += border + this->textShadow();

	y += border + padding_yt;

	result = OIIO::ImageBufAlgo::render_text_shadow(*buf, 
		x, 
		y, 
		label, 
		px, this->font(), color, 
		OIIO::ImageBufAlgo::TextAlignX::Left, 
		OIIO::ImageBufAlgo::TextAlignY::Baseline, 
		this->textShadow());

	if (result == false)
		fprintf(stderr, "render text error\n");
}


void VideoWidget::drawLabel(OIIO::ImageBuf *buf, const char *label) {
	bool result;

	int x, y;

	int x1, y1, x2, y2;
	int text_width, text_height;

	int border = this->border();
	int padding_xl = this->padding(VideoWidget::PaddingLeft);
	int padding_xr = this->padding(VideoWidget::PaddingRight);
	int padding_yt = this->padding(VideoWidget::PaddingTop);
//	int padding_yb = this->padding(VideoWidget::PaddingBottom);

	float color[4]; // = { 1.0, 1.0, 1.0, 1.0 };

	bool with_label = this->hasFlag(VideoWidget::FlagLabel);
	bool with_picto = this->hasFlag(VideoWidget::FlagPicto);

	enum VideoWidget::TextAlign textAlign = this->labelAlign();

	if (!with_label)
		return;

	// Compute text size
	this->textSize(label, label_px_,
			x1, y1, x2, y2,
			text_width, text_height);

	// Text color
	memcpy(color, this->textColor(), sizeof(color));

	// Text offset
	x = -x1;
	y = -y1 + this->textShadow();

	// Text position
	if (textAlign == VideoWidget::TextAlignLeft) {
		x += padding_xl;
		x += (with_picto) ? this->height() + padding_xl : 0;
		x += border + this->textShadow();
	}
	else if (textAlign == VideoWidget::TextAlignCenter) {
		x += (with_picto) ? (this->width() - this->height())/2 : this->width()/2;
		x += (with_picto) ? this->height() : 0;
		x -= text_width / 2;
	}
	else if (textAlign == VideoWidget::TextAlignRight) {
		x += this->width() - padding_xr;
		x -= text_width + border + this->textShadow();
	}

	y += border + padding_yt;

	result = OIIO::ImageBufAlgo::render_text_shadow(*buf, 
		x, 
		y, 
		label, 
		label_px_, this->font(), color, 
		OIIO::ImageBufAlgo::TextAlignX::Left, 
		OIIO::ImageBufAlgo::TextAlignY::Baseline,
		this->textShadow());

	if (result == false)
		fprintf(stderr, "render label text error\n");
}


void VideoWidget::drawValue(OIIO::ImageBuf *buf, const char *value) {
	bool result;
	
	int x, y;

	int x1, y1, x2, y2;
	int text_width, text_height;

	int border = this->border();
	int padding_xl = this->padding(VideoWidget::PaddingLeft);
	int padding_xr = this->padding(VideoWidget::PaddingRight);
	int padding_yt = this->padding(VideoWidget::PaddingTop);
	int padding_yb = this->padding(VideoWidget::PaddingBottom);

	float color[4]; // = { 1.0, 1.0, 1.0, 1.0 };

	bool with_picto = this->hasFlag(VideoWidget::FlagPicto);
	bool with_label = this->hasFlag(VideoWidget::FlagLabel);
	bool with_value = this->hasFlag(VideoWidget::FlagValue);

	enum VideoWidget::TextAlign textAlign = this->valueAlign();

	if (!with_value)
		return;

	// Compute text size
	this->textSize(value, value_px_,
			x1, y1, x2, y2,
			text_width, text_height);

	// Text color
	memcpy(color, this->textColor(), sizeof(color));

	// Text offset
	x = 0;
	y = -value_offset_ + this->textShadow();

	// Text position
	if (textAlign == VideoWidget::TextAlignLeft) {
		x += padding_xl;
		x += (with_picto) ? this->height() + padding_xl : 0;
		x += border + this->textShadow();
	}
	else if (textAlign == VideoWidget::TextAlignCenter) {
		x += (with_picto) ? (this->width() - this->height())/2 : this->width()/2;
		x += (with_picto) ? this->height() : 0;
		x -= text_width / 2;
	}
	else if (textAlign == VideoWidget::TextAlignRight) {
		x += this->width() - padding_xr;
		x -= text_width + border + this->textShadow();
	}

	if (with_label)
		y += this->height() - border - this->textShadow() - padding_yb - value_size_;
	else
		y += border + padding_yt;

	result = OIIO::ImageBufAlgo::render_text_shadow(*buf, 
		x, 
		y, 
		value, 
		value_px_, this->font(), color, 
		OIIO::ImageBufAlgo::TextAlignX::Left, 
		OIIO::ImageBufAlgo::TextAlignY::Baseline, 
		this->textShadow());

	if (result == false)
		fprintf(stderr, "render value text error\n");
}


bool VideoWidget::textSize(std::string text, int fontsize, 
	int &x1, int &y1, int &x2, int &y2,
	int &width, int &height) {

	OIIO::ROI roi = OIIO::ImageBufAlgo::text_size(text, fontsize, this->font());

	x1 = roi.xbegin;
	x2 = roi.xend;
	y1 = roi.ybegin;
	y2 = roi.yend;

	width = roi.width();
	height = roi.height();

	return roi.defined();
}


