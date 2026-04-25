#include <pango/pangocairo.h>

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

	width = theme_.width() - 1;
	height = theme_.height() - 1;

	border = theme_.border();
	memcpy(bordercolor, theme_.borderColor(), sizeof(bordercolor));

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

	width = theme_.width() - 1;
	height = theme_.height() - 1;

	border = theme_.border();
	memcpy(bgcolor, theme_.backgroundColor(), sizeof(bgcolor));

	if (bgcolor[3] != 0.0)
		OIIO::ImageBufAlgo::render_box(*buf, border, border, width - border, height - border, bgcolor, true);
}


//void ShapeBase::drawText(OIIO::ImageBuf *buf, int x, int y, int px, const char *label) {
//	bool result;
//
//	int x1, y1, x2, y2;
//	int text_width, text_height;
//
//	int border = theme_.border();
//	int padding_xl = theme_.padding(VideoWidget::Theme::PaddingLeft);
////	int padding_xr = theme_.padding(VideoWidget::Theme::PaddingRight);
//	int padding_yt = theme_.padding(VideoWidget::Theme::PaddingTop);
////	int padding_yb = theme_.padding(VideoWidget::Theme::PaddingBottom);
//
//	float color[4]; // = { 1.0, 1.0, 1.0, 1.0 };
//
//	// Compute text size
//	this->textSize(label, px,
//			x1, y1, x2, y2,
//			text_width, text_height);
//
//	// Text color
//	memcpy(color, theme_.labelColor(), sizeof(color));
//
//	// Text offset
//	x += -x1;
//	y += -y1 + theme_.textShadow();
//
//	// Text position
//	x += padding_xl;
//	x += border + theme_.textShadow();
//
//	y += border + padding_yt;
//
//	result = OIIO::ImageBufAlgo::render_text_shadow(*buf, 
//		x, 
//		y, 
//		label, 
//		px, theme_.font(), color, 
//		OIIO::ImageBufAlgo::TextAlignX::Left, 
//		OIIO::ImageBufAlgo::TextAlignY::Baseline, 
//		theme_.textShadow());
//
//	if (result == false)
//		fprintf(stderr, "render text error\n");
//}


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
		width = theme_.width() - theme_.border() - x;
		height = theme_.height() - theme_.border() - y;

		if (width * spec.height > spec.width * height)
			width = height * ratio;
		else
			height = width / ratio;
		break;

	case VideoWidget::ZoomFill:
		width = theme_.width() - theme_.border() - x;
		height = theme_.height() - theme_.border() - y;

		if (width * spec.height < spec.width * height)
			width = height * ratio;
		else
			height = width / ratio;
		break;

	case VideoWidget::ZoomStretch:
		width = theme_.width() - theme_.border() - x;
		height = theme_.height() - theme_.border() - y;
		break;

	case VideoWidget::ZoomCrop:
	default:
		width = spec.width;
		height = spec.height;
		break;
	}

	// Max width & height
	max_width = theme_.width() - (2 * theme_.border());
	max_height = theme_.height() - (2 * theme_.border());

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


//bool ShapeBase::textSize(std::string text, int fontsize, 
//	int &x1, int &y1, int &x2, int &y2,
//	int &width, int &height) {
//
//	OIIO::ROI roi = OIIO::ImageBufAlgo::text_size(text, fontsize, theme_.font());
//
//	x1 = roi.xbegin;
//	x2 = roi.xend;
//	y1 = roi.ybegin;
//	y2 = roi.yend;
//
//	width = roi.width();
//	height = roi.height();
//
//	return roi.defined();
//}


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
	if (data != NULL) {
		buf->set_pixels(OIIO::ROI(),
			buf->spec().format,
			data, 
			OIIO::AutoStride,
			stride);
	}

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


void ShapeBase::drawText(cairo_t *cr, int x, int y, ShapeBase::Font &font, 
		const float *fill, const float *outline, const char *text) {
	PangoLayout *layout;

	PangoFontDescription *desc;

	// Pango layout
	layout = pango_cairo_create_layout(cr);

	// Font loading
	desc = pango_font_description_new();
	pango_font_description_set_family(desc, font.family.c_str());
	pango_font_description_set_style(desc, (PangoStyle) font.style);
	pango_font_description_set_variant(desc, PANGO_VARIANT_NORMAL);
	pango_font_description_set_weight(desc, (PangoWeight) font.weight);
	pango_font_description_set_stretch(desc, PANGO_STRETCH_NORMAL);
//	pango_font_description_set_size(desc, fontsize * PANGO_SCALE);
	pango_font_description_set_absolute_size(desc, font.size * PANGO_SCALE);

	// Draw text into layout
	pango_layout_set_line_spacing(layout, 0);
	pango_layout_set_font_description(layout, desc);
	pango_layout_set_text(layout, text, -1);

	// Apply shadow effect
	if (font.shadow_distance > 0) {
		cairo_save(cr);
		cairo_move_to(cr, x + font.shadow_distance, y + font.shadow_distance);
		pango_cairo_layout_path(cr, layout);
		cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, font.shadow_opacity / 100.0);
		cairo_fill(cr);
		cairo_restore(cr);
	}

	// Apply color text
	cairo_save(cr);
	cairo_move_to(cr, x, y);
	pango_cairo_layout_path(cr, layout);
	cairo_set_source_rgba(cr, fill[0], fill[1], fill[2], fill[3]);
	cairo_fill_preserve(cr);

	// Draw text border
	if (font.border > 0) {
		cairo_set_line_width(cr, font.border / 10.0);
		cairo_set_source_rgba(cr, outline[0], outline[1], outline[2], outline[3]);
	}
	cairo_stroke(cr);
	cairo_restore(cr);

	pango_font_description_free(desc);

	g_object_unref(layout);
}


void ShapeBase::textSize(cairo_t *cr, ShapeBase::Font &font, const char *text,
		int &x, int &y, int &width, int &height) {
	PangoLayout *layout;

	PangoFontDescription *desc;

	PangoRectangle rectangle;

	// Pango layout
	layout = pango_cairo_create_layout(cr);

	// Font loading
	desc = pango_font_description_new();
	pango_font_description_set_family(desc, font.family.c_str());
	pango_font_description_set_style(desc, (PangoStyle) font.style);
	pango_font_description_set_variant(desc, PANGO_VARIANT_NORMAL);
	pango_font_description_set_weight(desc, (PangoWeight) font.weight);
	pango_font_description_set_stretch(desc, PANGO_STRETCH_NORMAL);
//	pango_font_description_set_size(desc, fontsize * PANGO_SCALE);
	pango_font_description_set_absolute_size(desc, font.size * PANGO_SCALE);

	// Draw text into layout
	pango_layout_set_line_spacing(layout, 0);
	pango_layout_set_font_description(layout, desc);
	pango_layout_set_text(layout, text, -1);

	// Get text size
	pango_layout_get_pixel_extents(layout, &rectangle, NULL);

	// Return text position & size
	x = rectangle.x - 1;
	y = rectangle.y - 1;
	width = rectangle.width;
	height = rectangle.height;

	pango_font_description_free(desc);

	g_object_unref(layout);
}

void ShapeBase::xmlwrite(std::ostream &os) {
	os << "<padding-left>" << theme_.padding(VideoWidget::Theme::PaddingLeft) << "</padding-left>" << std::endl;
	os << "<padding-right>" << theme_.padding(VideoWidget::Theme::PaddingRight) << "</padding-right>" << std::endl;
	os << "<padding-top>" << theme_.padding(VideoWidget::Theme::PaddingTop) << "</padding-top>" << std::endl;
	os << "<padding-bottom>" << theme_.padding(VideoWidget::Theme::PaddingBottom) << "</padding-bottom>" << std::endl;

	os << "<border>" << theme_.border() << "</border>" << std::endl;
	os << "<border-color>" << VideoWidget::Theme::color2hex(theme_.borderColor()) << "</border-color>" << std::endl;

	os << "<background-color>" << VideoWidget::Theme::color2hex(theme_.backgroundColor()) << "</background-color>" << std::endl;

	os << "<font>" << theme_.font() << "</font>" << std::endl;

	os << "<with-label>" << VideoWidget::bool2string(theme_.hasFlag(VideoWidget::Theme::FlagLabel)) << "</with-label>" << std::endl;
	os << "<label-font-size>" << theme_.labelFontSize() << "</label-font-size>" << std::endl;
	os << "<label-font-family>" << theme_.labelFontFamily() << "</label-font-family>" << std::endl;
	os << "<label-font-style>" << VideoWidget::fontstyle2string(theme_.labelFontStyle()) << "</label-font-style>" << std::endl;
	os << "<label-font-weight>" << VideoWidget::fontweight2string(theme_.labelFontWeight()) << "</label-font-weight>" << std::endl;
	os << "<label-align>" << VideoWidget::align2string(theme_.labelAlign()) << "</label-align>" << std::endl;
	os << "<label-color>" << VideoWidget::Theme::color2hex(theme_.labelColor()) << "</label-color>" << std::endl;
	os << "<label-shadow-opacity>" << theme_.labelShadowOpacity() << "</label-shadow-opacity>" << std::endl;
	os << "<label-shadow-distance>" << theme_.labelShadowDistance() << "</label-shadow-distance>" << std::endl;
	os << "<label-border-width>" << theme_.labelBorderWidth() << "</label-border-width>" << std::endl;
	os << "<label-border-color>" << VideoWidget::Theme::color2hex(theme_.labelBorderColor()) << "</label-border-color>" << std::endl;

	os << "<with-value>" << VideoWidget::bool2string(theme_.hasFlag(VideoWidget::Theme::FlagValue)) << "</with-value>" << std::endl;
	os << "<value-font-family>" << theme_.valueFontFamily() << "</value-font-family>" << std::endl;
	os << "<value-font-size>" << theme_.valueFontSize() << "</value-font-size>" << std::endl;
	os << "<value-font-style>" << VideoWidget::fontstyle2string(theme_.valueFontStyle()) << "</value-font-style>" << std::endl;
	os << "<value-font-weight>" << VideoWidget::fontweight2string(theme_.valueFontWeight()) << "</value-font-weight>" << std::endl;
	os << "<value-align>" << VideoWidget::align2string(theme_.valueAlign()) << "</value-align>" << std::endl;
	os << "<value-color>" << VideoWidget::Theme::color2hex(theme_.valueColor()) << "</value-color>" << std::endl;
	os << "<value-shadow-opacity>" << theme_.valueShadowOpacity() << "</value-shadow-opacity>" << std::endl;
	os << "<value-shadow-distance>" << theme_.valueShadowDistance() << "</value-shadow-distance>" << std::endl;
	os << "<value-border-width>" << theme_.valueBorderWidth() << "</value-border-width>" << std::endl;
	os << "<value-border-color>" << VideoWidget::Theme::color2hex(theme_.valueBorderColor()) << "</value-border-color>" << std::endl;
	os << "<value-min>" << theme_.valueMin() << "</value-min>" << std::endl;
	os << "<value-max>" << theme_.valueMax() << "</value-max>" << std::endl;
}

