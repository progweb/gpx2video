#include <gtkmm/spinbutton.h>
#include <gtkmm/colorbutton.h>

#include "log.h"
#include "widgetframe.h"



GPX2VideoWidgetFrame::GPX2VideoWidgetFrame()
	: Glib::ObjectBase("GPX2VideoWidgetFrame")
	, ref_builder_(NULL) 
	, dispatcher_()
	, renderer_(NULL)
	, widget_selected_(NULL) {
	log_call();
}


GPX2VideoWidgetFrame::GPX2VideoWidgetFrame(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &ref_builder) 
	: Glib::ObjectBase("GPX2VideoWidgetFrame")
	, Gtk::Frame(cobject)
	, ref_builder_(ref_builder) 
	, dispatcher_()
	, renderer_(NULL)
	, widget_selected_(NULL) {
	log_call();

	Gtk::SpinButton *spinbutton;
	Gtk::ColorButton *colorbutton;

	// Connect widgets button
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("background_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"background_color_button\" object in video_frame.ui");
	colorbutton->signal_color_set().connect(sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_background_color_set));

	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("border_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"border_color_button\" object in video_frame.ui");
	colorbutton->signal_color_set().connect(sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_border_color_set));

	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("border_width_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"border_width_spinbutton\" object in widget_frame.ui");
	spinbutton->signal_value_changed().connect(sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_border_value_changed));

	// Update ui
	update_content();
}


GPX2VideoWidgetFrame::~GPX2VideoWidgetFrame() {
	log_call();
}


void GPX2VideoWidgetFrame::set_renderer(GPX2VideoRenderer *renderer) {
	log_call();

	renderer_ = renderer;
}


void GPX2VideoWidgetFrame::set_visible(bool visible) {
	log_call();

	Gtk::Frame::set_visible(visible && (widget_selected_ != NULL));
}


void GPX2VideoWidgetFrame::set_widget_selected(GPX2VideoWidget *widget) {
	log_call();

	// Save selected widget
	widget_selected_ = widget;

	// Update ui content
	update_content();
}


void GPX2VideoWidgetFrame::update_content(void) {
	log_call();

	Gdk::RGBA rgba;

	const float *color;

	Gtk::SpinButton *spinbutton;
	Gtk::ColorButton *colorbutton;

	// Frame is visible only as widget is selected
	set_visible((widget_selected_ != NULL));

	// No widget
	if (widget_selected_ == NULL)
		return;

	// Widget background color button
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("background_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"background_color_button\" object in widget_frame.ui");

	color = widget_selected_->widget()->backgroundColor();

	rgba.set_rgba(color[0], color[1], color[2], color[3]);

	colorbutton->set_rgba(rgba);

	// Widget border color button
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("border_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"border_color_button\" object in widget_frame.ui");

	color = widget_selected_->widget()->borderColor();

	rgba.set_rgba(color[0], color[1], color[2], color[3]);

	colorbutton->set_rgba(rgba);

	// Widget border width
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("border_width_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"border_width_spinbutton\" object in widget_frame.ui");

	spinbutton->set_value(widget_selected_->widget()->border());
}


void GPX2VideoWidgetFrame::on_widget_background_color_set(void) {
	log_call();

	Gdk::RGBA rgba;

	Glib::ustring color;

	// Widget color button
	auto button = ref_builder_->get_widget<Gtk::ColorButton>("background_color_button");
	if (!button)
		throw std::runtime_error("No \"background_color_button\" object in widget_frame.ui");

	rgba = button->get_rgba();

	// Convert to hexa string color
	color = Glib::ustring::sprintf("#%02X%02X%02X%02X",
			(unsigned char) std::round(rgba.get_red() * 255),
			(unsigned char) std::round(rgba.get_green() * 255),
			(unsigned char) std::round(rgba.get_blue() * 255),
			(unsigned char) std::round(rgba.get_alpha() * 255)
	);

	log_info("Background color changed to '%s'", color.c_str());

	widget_selected_->widget()->setBackgroundColor(color);
	renderer_->refresh(widget_selected_);

	// Refresh video preview
	dispatcher_.emit();
}


void GPX2VideoWidgetFrame::on_widget_border_color_set(void) {
	log_call();

	Gdk::RGBA rgba;

	Glib::ustring color;

	// Widget color button
	auto button = ref_builder_->get_widget<Gtk::ColorButton>("border_color_button");
	if (!button)
		throw std::runtime_error("No \"border_color_button\" object in widget_frame.ui");

	rgba = button->get_rgba();

	// Convert to hexa string color
	color = Glib::ustring::sprintf("#%02X%02X%02X%02X",
			(unsigned char) std::round(rgba.get_red() * 255),
			(unsigned char) std::round(rgba.get_green() * 255),
			(unsigned char) std::round(rgba.get_blue() * 255),
			(unsigned char) std::round(rgba.get_alpha() * 255)
	);

	log_info("Border color changed to '%s'", color.c_str());

	widget_selected_->widget()->setBorderColor(color);
	renderer_->refresh(widget_selected_);

	// Refresh video preview
	dispatcher_.emit();
}


void GPX2VideoWidgetFrame::on_widget_border_value_changed(void) {
	log_call();

	int border;

	// Widget border width
	auto spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("border_width_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"border_width_spinbutton\" object in widget_frame.ui");

	border = spinbutton->get_value_as_int();

	log_info("Border widget changed to '%d'", border);

	widget_selected_->widget()->setBorder(border);
	renderer_->refresh(widget_selected_);

	// Refresh video preview
	dispatcher_.emit();
}

