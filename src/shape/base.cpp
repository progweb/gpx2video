#include "../log_i.h"
#include "base.h"


void ShapeBase::createBox(OIIO::ImageBuf **buf, int width, int height) {
	// Create an image buffer with static render
	*buf = new OIIO::ImageBuf(OIIO::ImageSpec(width, height, 4, OIIO::TypeDesc::UINT8));
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

	if (!img) {
		log_error("Can't open '%s' image file!", name);
		return;
	}

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

	cairo_destroy(cairo);

	if (surface != NULL)
		cairo_surface_destroy(surface);
}


void ShapeBase::saveCairoSurface(cairo_t *cairo) {
	int width, height;

	cairo_surface_t *src, *dst;

	// Get surface
	src = cairo_get_target(cairo);

	width = cairo_image_surface_get_width(src);
	height = cairo_image_surface_get_height(src);

	// Create & copy new surface
	dst = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);

	cairo_t *cr = cairo_create(dst);
	cairo_set_source_surface(cr, src, 0, 0);
	cairo_paint(cr);
	cairo_destroy(cr);

	surface_ = dst;
}


bool ShapeBase::restoreCairoSurface(cairo_t *cairo) {
	if (surface_ == NULL)
		return false;

	cairo_set_source_surface(cairo, surface_, 0, 0);
	cairo_paint(cairo);

	return true;
}


void ShapeBase::background(cairo_t *cr, double radius) {
	int x, y;
	int width, height;

	int border;

	double degrees = M_PI / 180.0;

	const float *fill = theme_.backgroundColor();
	const float *outline = theme_.borderColor();

	border = size2pixels(theme_.border()) / 2.0;

	width = theme_.width() - border;
	height = theme_.height() - border;

	radius = size2pixels(radius) / 2.0;

	x = border / 2;
	y = border / 2;

	cairo_save(cr);
	if (radius > 0) {
		cairo_new_sub_path(cr);
		cairo_arc(cr, x + width - radius, y + radius, radius, -90 * degrees, 0 * degrees);
		cairo_arc(cr, x + width - radius, y + height - radius, radius, 0 * degrees, 90 * degrees);
		cairo_arc(cr, x + radius, y + height - radius, radius, 90 * degrees, 180 * degrees);
		cairo_arc(cr, x + radius, y + radius, radius, 180 * degrees, 270 * degrees);
		cairo_close_path(cr);
	}
	else
		cairo_rectangle(cr, x, y, width, height);
	cairo_set_source_rgba(cr, fill[0], fill[1], fill[2], fill[3]);
	cairo_fill_preserve(cr);
	if ((border > 0) && (outline != NULL)) {
		cairo_set_line_width(cr, border);
		cairo_set_source_rgba(cr, outline[0], outline[1], outline[2], outline[3]);
	}
	cairo_stroke(cr);
	cairo_restore(cr);
}


void ShapeBase::text(cairo_t *cr, int x, int y, ShapeBase::Font &font, 
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
	pango_font_description_set_absolute_size(desc, fontsize2pixels(font.size));

//	// Linespace
//	if (linespace > 0) {
//		PangoAttrList *attrs = pango_attr_list_new();
//		PangoAttribute *lh = pango_attr_line_height_new(linespace / 10.0);
//		pango_attr_list_insert(attrs, lh);
//		pango_layout_set_attributs(layout, attrs);
//	}

	// Draw text into layout
	pango_layout_set_alignment(layout, (PangoAlignment) font.align);
	pango_layout_set_spacing(layout, 0);
	pango_layout_set_line_spacing(layout, font.linespace);
	pango_layout_set_font_description(layout, desc);
	pango_layout_set_text(layout, text, -1);

	// Apply shadow effect
	if (font.shadow_distance > 0) {
		double distance = shadow2pixels(font);

		cairo_save(cr);
		cairo_move_to(cr, x + distance, y + distance);
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
		cairo_set_line_width(cr, font.border * fontsize2pixels(font.size) / 100000.0);
		cairo_set_source_rgba(cr, outline[0], outline[1], outline[2], outline[3]);
	}
	cairo_stroke(cr);
	cairo_restore(cr);

	pango_font_description_free(desc);

	g_object_unref(layout);
}


void ShapeBase::extents(cairo_t *cr, ShapeBase::Font &font, ShapeBase::TextType type, const char *text,
		int &x, int &y, int &width, int &height) {
	std::string dummy;
   
	PangoLayout *layout;

	PangoFontDescription *desc;

	PangoRectangle rectangle;

	// Alpha / Numeric
	dummy = 
		"0123456789";

	if (type == ShapeBase::TextAlpha) {
		dummy += 
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz";
	}

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
	pango_font_description_set_absolute_size(desc, fontsize2pixels(font.size));

	// Text properties
	pango_layout_set_alignment(layout, (PangoAlignment) font.align);
	pango_layout_set_spacing(layout, 0);
	pango_layout_set_line_spacing(layout, font.linespace);
	pango_layout_set_font_description(layout, desc);

	// Get text size
	pango_layout_set_text(layout, text, -1);
	pango_layout_get_pixel_extents(layout, &rectangle, NULL);

	// Return text position & size
	x = (rectangle.x - 1);
	y = (rectangle.y - 1);
	width = rectangle.width;
	height = rectangle.height;

	if (type != ShapeBase::TextMultiLine) {
		// Get dummy size
		pango_layout_set_text(layout, dummy.c_str(), -1);
		pango_layout_get_pixel_extents(layout, &rectangle, NULL);

		// Return dummy position & size
		y = (rectangle.y - 1);
		height = rectangle.height;
	}

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

	if (hasFeature(ShapeBase::FeatureRoundCorner))
		os << "<round-corner>" << theme_.roundCorner() << "</round-corner>" << std::endl;

	os << "<background-color>" << VideoWidget::Theme::color2hex(theme_.backgroundColor()) << "</background-color>" << std::endl;

	if (hasFeature(ShapeBase::FeatureLabel)) {
		os << "<with-label>" << VideoWidget::bool2string(theme_.hasFlag(VideoWidget::Theme::FlagLabel)) << "</with-label>" << std::endl;
		os << "<label-font-size>" << theme_.labelFontSize() << "</label-font-size>" << std::endl;
		os << "<label-font-family>" << theme_.labelFontFamily() << "</label-font-family>" << std::endl;
		os << "<label-font-style>" << VideoWidget::fontstyle2string(theme_.labelFontStyle()) << "</label-font-style>" << std::endl;
		os << "<label-font-weight>" << VideoWidget::fontweight2string(theme_.labelFontWeight()) << "</label-font-weight>" << std::endl;
		os << "<label-horizontal-align>" << VideoWidget::align2string(theme_.labelHorizontalAlign()) << "</label-horizontal-align>" << std::endl;
		os << "<label-vertical-align>" << VideoWidget::align2string(theme_.labelVerticalAlign()) << "</label-vertical-align>" << std::endl;
		os << "<label-color>" << VideoWidget::Theme::color2hex(theme_.labelColor()) << "</label-color>" << std::endl;
		os << "<label-shadow-opacity>" << theme_.labelShadowOpacity() << "</label-shadow-opacity>" << std::endl;
		os << "<label-shadow-distance>" << theme_.labelShadowDistance() << "</label-shadow-distance>" << std::endl;
		os << "<label-border-width>" << theme_.labelBorderWidth() << "</label-border-width>" << std::endl;
		os << "<label-border-color>" << VideoWidget::Theme::color2hex(theme_.labelBorderColor()) << "</label-border-color>" << std::endl;
	}

	if (hasFeature(ShapeBase::FeatureValue)) {
		os << "<with-value>" << VideoWidget::bool2string(theme_.hasFlag(VideoWidget::Theme::FlagValue)) << "</with-value>" << std::endl;
		os << "<value-font-family>" << theme_.valueFontFamily() << "</value-font-family>" << std::endl;
		os << "<value-font-size>" << theme_.valueFontSize() << "</value-font-size>" << std::endl;
		os << "<value-font-style>" << VideoWidget::fontstyle2string(theme_.valueFontStyle()) << "</value-font-style>" << std::endl;
		os << "<value-font-weight>" << VideoWidget::fontweight2string(theme_.valueFontWeight()) << "</value-font-weight>" << std::endl;
		os << "<value-horizontal-align>" << VideoWidget::align2string(theme_.valueHorizontalAlign()) << "</value-horizontal-align>" << std::endl;
		os << "<value-vertical-align>" << VideoWidget::align2string(theme_.valueVerticalAlign()) << "</value-vertical-align>" << std::endl;
		os << "<value-color>" << VideoWidget::Theme::color2hex(theme_.valueColor()) << "</value-color>" << std::endl;
		os << "<value-shadow-opacity>" << theme_.valueShadowOpacity() << "</value-shadow-opacity>" << std::endl;
		os << "<value-shadow-distance>" << theme_.valueShadowDistance() << "</value-shadow-distance>" << std::endl;
		os << "<value-border-width>" << theme_.valueBorderWidth() << "</value-border-width>" << std::endl;
		os << "<value-border-color>" << VideoWidget::Theme::color2hex(theme_.valueBorderColor()) << "</value-border-color>" << std::endl;
		os << "<value-min>" << theme_.valueMin() << "</value-min>" << std::endl;
		os << "<value-max>" << theme_.valueMax() << "</value-max>" << std::endl;
	}

	if (hasFeature(ShapeBase::FeatureUnit)) {
		os << "<with-unit>" << VideoWidget::bool2string(theme_.hasFlag(VideoWidget::Theme::FlagUnit)) << "</with-unit>" << std::endl;
		os << "<unit-font-family>" << theme_.unitFontFamily() << "</unit-font-family>" << std::endl;
		os << "<unit-font-style>" << VideoWidget::fontstyle2string(theme_.unitFontStyle()) << "</unit-font-style>" << std::endl;
		os << "<unit-font-weight>" << VideoWidget::fontweight2string(theme_.unitFontWeight()) << "</unit-font-weight>" << std::endl;
		os << "<unit-horizontal-align>" << VideoWidget::align2string(theme_.unitHorizontalAlign()) << "</unit-horizontal-align>" << std::endl;
		os << "<unit-vertical-align>" << VideoWidget::align2string(theme_.unitVerticalAlign()) << "</unit-vertical-align>" << std::endl;
		os << "<unit-color>" << VideoWidget::Theme::color2hex(theme_.unitColor()) << "</unit-color>" << std::endl;
		os << "<unit-shadow-opacity>" << theme_.unitShadowOpacity() << "</unit-shadow-opacity>" << std::endl;
		os << "<unit-shadow-distance>" << theme_.unitShadowDistance() << "</unit-shadow-distance>" << std::endl;
		os << "<unit-border-width>" << theme_.unitBorderWidth() << "</unit-border-width>" << std::endl;
		os << "<unit-border-color>" << VideoWidget::Theme::color2hex(theme_.unitBorderColor()) << "</unit-border-color>" << std::endl;
		os << "<unit-font-size>" << theme_.unitFontSize() << "</unit-font-size>" << std::endl;
		os << "<unit-distance>" << theme_.unitDistance() << "</unit-distance>" << std::endl;
	}
}

