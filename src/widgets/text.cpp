#include "text.h"


TextWidget::TextWidget(GPXApplication &app)
	: VideoWidget(app, VideoWidget::WidgetText) 
	, TextShape(theme())
	, bg_buf_(NULL)
	, fg_buf_(NULL) {

#define ADD_SHAPE(shape) \
	shapes_supported_.push_back((VideoWidget::ListItem) { \
		shape, VideoWidget::getFriendlyName(shape), VideoWidget::shape2string(shape) \
	})

	ADD_SHAPE(VideoWidget::ShapeText);
}


bool TextWidget::updated(const TelemetryData &data) const {
	(void) data;

	if (is_initialized_) {
		return false;
	}

	return true;
}


void TextWidget::initialize(cairo_t *cr) {
	if (is_initialized_)
		return;

	int x, y;
	int width, height;

	ShapeBase::Font font;

	setPadding(
		theme().border() + theme().padding(VideoWidget::Theme::PaddingLeft),
		theme().border() + theme().padding(VideoWidget::Theme::PaddingRight),
   		theme().border() + theme().padding(VideoWidget::Theme::PaddingTop),
   		theme().border() + theme().padding(VideoWidget::Theme::PaddingBottom));

	// Label height
	if (theme().hasFlag(VideoWidget::Theme::FlagLabel)) {
		std::string s = ((VideoWidget *) this)->label();

		font = (ShapeBase::Font) {
			.size = theme().labelFontSize(),
			.border = theme().labelBorderWidth(),
			.shadow_opacity = theme().labelShadowOpacity(),
			.shadow_distance = theme().labelShadowDistance(),
			.family = theme().labelFontFamily(),
			.align = theme().labelHorizontalAlign(),
			.style = theme().labelFontStyle(),
			.weight = theme().labelFontWeight(),
		};

		extents(cr, font, s.c_str(), x, y, width, height);

		setLabelExtents(x, y, width, height);
	}

	// Value height
	if (theme().hasFlag(VideoWidget::Theme::FlagValue)) {
		std::string txt = VideoWidget::value();

		font = (ShapeBase::Font) {
			.size = theme().valueFontSize(),
			.border = theme().valueBorderWidth(),
			.shadow_opacity = theme().valueShadowOpacity(),
			.shadow_distance = theme().valueShadowDistance(),
			.family = theme().valueFontFamily(),
			.align = theme().valueHorizontalAlign(),
			.style = theme().valueFontStyle(),
			.weight = theme().valueFontWeight(),
		};

		extents(cr, font, txt.c_str(), x, y, width, height);
		
		setValueExtents(x, y, width, height);
	}

//	int n_families;
//
//	PangoFontMap *fontmap;
//	PangoFontFamily **families;
//
//	fontmap = pango_cairo_font_map_get_default();
//
//	pango_font_map_list_families(fontmap, &families, &n_families);
//
//	for (int i=0; i<n_families; i++) {
//		const char *familiy_name = pango_font_family_get_name(families[i]);
//
//		printf("Font family name: %s\n", familiy_name);
//
//		int n_faces;
//
//		PangoFontFace **faces;
//
//		pango_font_family_list_faces(families[i], &faces, &n_faces);
//
//		for (int j=0; j<n_faces; j++) {
//			printf("  Face: %s\n", pango_font_face_get_face_name(faces[j]));
//		}
//
//		g_free(faces);
//	}
//
//	g_free(families);
//
//	PangoLanguage *lang = pango_language_get_default();
//
//	const char *lang_str = pango_language_to_string(lang);
//
//	printf("LANGUAGE: %s\n", lang_str);

	is_initialized_ = true;
}


void TextWidget::draw(cairo_t *cr, const TelemetryData &data) {
	ShapeBase::Font font;

	(void) data;

	// Initialize
	initialize(cr);

	// Draw background
	background(cr);

	// Draw label
	if (theme().hasFlag(VideoWidget::Theme::FlagLabel)) {
		std::string s = ((VideoWidget *) this)->label();

		font = (ShapeBase::Font) {
			.size = theme().labelFontSize(),
			.border = theme().labelBorderWidth(),
			.shadow_opacity = theme().labelShadowOpacity(),
			.shadow_distance = theme().labelShadowDistance(),
			.family = theme().labelFontFamily(),
			.align = theme().labelHorizontalAlign(),
			.style = theme().labelFontStyle(),
			.weight = theme().labelFontWeight(),
		};

		TextShape::label(cr, font, theme().labelColor(), theme().labelBorderColor(), s.c_str());
	}

	// Draw value
	if (theme().hasFlag(VideoWidget::Theme::FlagValue)) {
		font = (ShapeBase::Font) {
			.size = theme().valueFontSize(),
			.border = theme().valueBorderWidth(),
			.shadow_opacity = theme().valueShadowOpacity(),
			.shadow_distance = theme().valueShadowDistance(),
			.family = theme().valueFontFamily(),
			.align = theme().valueHorizontalAlign(),
			.style = theme().valueFontStyle(),
			.weight = theme().valueFontWeight(),
		};

		TextShape::value(cr, font, theme().valueColor(), theme().valueBorderColor(), VideoWidget::value().c_str());
	}

//	double fix;
//	int x, y;
//
//	int fontsize;
//
//	PangoLayout *layout;
//	PangoContext *context;
//
//	PangoFont *font;
//	PangoFontMap *fontmap;
//	PangoFontMetrics *metrics;
//	PangoFontDescription *desc;
//
//	int border = theme().labelBorderWidth();
//	const float *fill = theme().labelColor(); //{ 1.0, 1.0, 1.0, 1.0 };
//	const float *outline = theme().labelBorderColor(); //{ 0.0, 0.0, 1.0, 1.0 };
//
//	fontsize = theme().labelFontSize();
//
//	x = theme().padding(VideoWidget::Theme::PaddingLeft);
//	y = theme().padding(VideoWidget::Theme::PaddingTop);
//
//	// Pango layout
//	layout = pango_cairo_create_layout(cr);
//	context = pango_cairo_create_context(cr);
//
//	//desc = pango_font_description_from_string("Sans Bold 27");
//	desc = pango_font_description_new();
//	pango_font_description_set_family(desc, theme().labelFontFamily().c_str());
//	pango_font_description_set_style(desc, (PangoStyle) theme().labelFontStyle());
//	pango_font_description_set_variant(desc, PANGO_VARIANT_NORMAL);
//	pango_font_description_set_weight(desc, (PangoWeight) theme().labelFontWeight());
//	pango_font_description_set_stretch(desc, PANGO_STRETCH_NORMAL);
//	pango_font_description_set_size(desc, fontsize * PANGO_SCALE);
//	pango_font_description_set_absolute_size(desc, fontsize * PANGO_SCALE);
//
//	fontmap = pango_cairo_font_map_get_default();
//	font = pango_font_map_load_font(fontmap, context, desc);
//	metrics = pango_font_get_metrics(font, NULL);
//
//	printf("size: %d / %f \n", fontsize, fontsize * PANGO_SCALE);
//	printf("HEIGHT: %d / %f px\n", pango_font_metrics_get_height(metrics), pango_font_metrics_get_height(metrics) / (double) PANGO_SCALE);
//	printf("ASCENT: %.2f px\n", pango_font_metrics_get_ascent(metrics) / (double) PANGO_SCALE);
//	printf("DESCENT: %.2f px\n", pango_font_metrics_get_descent(metrics) / (double) PANGO_SCALE);
//
//	fix = (fontsize > 0) ? ((double) pango_font_metrics_get_height(metrics) / (double) PANGO_SCALE) / (double) fontsize : 0;
//
//	if (fix > 0)
//		pango_font_description_set_size(desc, fontsize * PANGO_SCALE / fix);
//
//	printf("APPLY FIX: %f\n", fix);
//
//	pango_font_metrics_unref(metrics);
//	g_object_unref(font);
//
//	font = pango_font_map_load_font(fontmap, context, desc);
//	metrics = pango_font_get_metrics(font, NULL);
//
//	printf("HEIGHT: %d / %f px\n", pango_font_metrics_get_height(metrics), pango_font_metrics_get_height(metrics) / (double) PANGO_SCALE);
//	printf("ASCENT: %.2f px\n", pango_font_metrics_get_ascent(metrics) / (double) PANGO_SCALE);
//	printf("DESCENT: %.2f px\n", pango_font_metrics_get_descent(metrics) / (double) PANGO_SCALE);
//
//	pango_layout_set_line_spacing(layout, 0);
//	pango_layout_set_font_description(layout, desc);
//	pango_layout_set_text(layout, "\"L'TEXTpyËÂ\"", -1);
//	pango_layout_set_text(layout, this->text().c_str(), -1);
//
//
//	// shadow effect
//	cairo_save(cr);
//	cairo_move_to(cr, x + theme().labelShadowDistance(), y + theme().labelShadowDistance());
//	pango_cairo_layout_path(cr, layout);
//	cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, theme().labelShadowOpacity());
//	cairo_fill(cr);
//	cairo_restore(cr);
//
//	// main text
//	cairo_save(cr);
//	cairo_move_to(cr, x, y);
//	pango_cairo_layout_path(cr, layout);
//	cairo_set_source_rgba(cr, fill[0], fill[1], fill[2], fill[3]);
//	cairo_fill_preserve(cr);
//
//	cairo_set_line_width(cr, border);
//	cairo_set_source_rgba(cr, outline[0], outline[1], outline[2], outline[3]);
//	cairo_stroke(cr);
//	cairo_restore(cr);
//
//	pango_cairo_show_layout(cr, layout);
//
//
//	pango_font_metrics_unref(metrics);
//	pango_font_description_free(desc);
//
//	g_object_unref(context);
//	g_object_unref(layout);
//	g_object_unref(font);
}


void TextWidget::clear(void) {
	is_initialized_ = false;

	setPadding(0, 0, 0, 0);
	setLabelExtents(0, 0, 0, 0);
	setValueExtents(0, 0, 0, 0);

	if (bg_buf_)
		delete bg_buf_;

	if (fg_buf_)
		delete fg_buf_;

	bg_buf_ = NULL;
	fg_buf_ = NULL;
}

