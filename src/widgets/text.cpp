#include "text.h"

void TextWidget::initialize(void) {
}


void TextWidget::draw(cairo_t *cr, const TelemetryData &data) {
//	double fix;
	int x, y;

	int fontsize;

	PangoLayout *layout;
//	PangoContext *context;

//	PangoFont *font;
//	PangoFontMap *fontmap;
//	PangoFontMetrics *metrics;
	PangoFontDescription *desc;

	int border = theme().labelBorder();
	const float *fill = theme().labelColor(); //{ 1.0, 1.0, 1.0, 1.0 };
	const float *outline = theme().labelBorderColor(); //{ 0.0, 0.0, 1.0, 1.0 };

	fontsize = theme().labelFontSize();

	x = theme().padding(VideoWidget::Theme::PaddingLeft);
	y = theme().padding(VideoWidget::Theme::PaddingTop);

	// Pango layout
	layout = pango_cairo_create_layout(cr);
//	context = pango_cairo_create_context(cr);

	//desc = pango_font_description_from_string("Sans Bold 27");
	desc = pango_font_description_new();
	pango_font_description_set_family(desc, "Sans");
	pango_font_description_set_style(desc, (PangoStyle) theme().labelFontStyle());
	pango_font_description_set_variant(desc, PANGO_VARIANT_NORMAL);
	pango_font_description_set_weight(desc, (PangoWeight) theme().labelFontWeight());
	pango_font_description_set_stretch(desc, PANGO_STRETCH_NORMAL);
//	pango_font_description_set_size(desc, fontsize * PANGO_SCALE);
	pango_font_description_set_absolute_size(desc, fontsize * PANGO_SCALE);

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

	pango_layout_set_line_spacing(layout, 0);
	pango_layout_set_font_description(layout, desc);
//	pango_layout_set_text(layout, "\"L'TEXTpyËÂ\"", -1);
	pango_layout_set_text(layout, this->text().c_str(), -1);


	// shadow effect
	cairo_save(cr);
	cairo_move_to(cr, x + theme().labelShadowDistance(), y + theme().labelShadowDistance());
	pango_cairo_layout_path(cr, layout);
	cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, theme().labelShadowOpacity());
	cairo_fill(cr);
	cairo_restore(cr);

	// main text
	cairo_save(cr);
	cairo_move_to(cr, x, y);
	pango_cairo_layout_path(cr, layout);
	cairo_set_source_rgba(cr, fill[0], fill[1], fill[2], fill[3]);
	cairo_fill_preserve(cr);

	cairo_set_line_width(cr, border);
	cairo_set_source_rgba(cr, outline[0], outline[1], outline[2], outline[3]);
	cairo_stroke(cr);
	cairo_restore(cr);

//	pango_cairo_show_layout(cr, layout);


//	pango_font_metrics_unref(metrics);
	pango_font_description_free(desc);

//	g_object_unref(context);
	g_object_unref(layout);
//	g_object_unref(font);
}

