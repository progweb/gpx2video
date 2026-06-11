#include <glibmm/i18n.h>

#include "../log_i.h"
#include "../../src/videowidget.h"
#include "text.h"


GPX2VideoTextWidgetSettingsBox::GPX2VideoTextWidgetSettingsBox(BaseObjectType *cobject,
	const Glib::RefPtr<Gtk::Builder> &ref_builder, std::string resource_file, GPX2VideoWidget *widget) 
	: GPX2VideoWidgetBaseSettingsBox(cobject, ref_builder, "GPX2VideoTextWidgetSettingsBox", resource_file, widget) { 
	log_call();

	// Populate models
	load_models();

	// Binding
	bind_content();

	// Update ui
	update_content();
	update_boundaries();
}


void GPX2VideoTextWidgetSettingsBox::load_models(void) {
	log_call();
}


void GPX2VideoTextWidgetSettingsBox::bind_content(void) {
	log_call();

	Gtk::TextView *textview;

	Glib::RefPtr<Gtk::TextBuffer> buffer;

	// Connect widgets button
	//------------------------

	// Text
	textview = ref_builder_->get_widget<Gtk::TextView>("text_textview");
	if (!textview)
		throw std::runtime_error("No \"text_textview\" object in " + resource_file_);

	buffer = Gtk::TextBuffer::create();
	textview->set_buffer(buffer);
	textview->set_size_request(-1, 50);

	buffer->signal_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoTextWidgetSettingsBox::on_widget_textview_changed), textview,
					[this](Glib::RefPtr<Gtk::TextBuffer> buffer) {
						std::string string = buffer->get_text();

						log_notice("Widget %s: text content changed", widget_->name().c_str());
						log_notice("%s", string.c_str());

						widget_->widget()->setValue(string);

						// Broadcast widget change
						widget_->dispatchEvent(false);
					}
			));
}


void GPX2VideoTextWidgetSettingsBox::update_content(void) {
	log_call();

	Gtk::TextView *textview;

	Glib::RefPtr<Gtk::TextBuffer> buffer;

	// Mask value changed
	loading_ = true;

	// text
	textview = ref_builder_->get_widget<Gtk::TextView>("text_textview");
	if (!textview)
		throw std::runtime_error("No \"text_textview\" object in " + resource_file_);

	buffer = textview->get_buffer();
	buffer->set_text(widget_->widget()->value());

	// Unmask value changed
	loading_ = false;
}


void GPX2VideoTextWidgetSettingsBox::update_boundaries(void) {
	log_call();
}

