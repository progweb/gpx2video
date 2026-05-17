#include <iostream>
#include <exception>

#include <gdkmm/general.h>

#include <gtkmm.h>
#include <gtkmm/label.h>
#include <gtkmm/image.h>

#include "log_i.h"
#include "../src/widgets.h"
#include "append.h"



GPX2VideoAppend::GPX2VideoAppend()
	: Glib::ObjectBase("GPX2VideoAppend")
	, renderer_(NULL) {
	log_call();
}


GPX2VideoAppend::GPX2VideoAppend(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder>& ref_builder, GPX2VideoRenderer *renderer) 
	: Glib::ObjectBase("GPX2VideoAppend")
	, Gtk::Dialog(cobject)
	, ref_builder_(ref_builder)
	, renderer_(renderer) {
	log_call();

	// Populate widgets list
	build();

	// Connect widget list
	auto list = ref_builder_->get_widget<Gtk::ListBox>("widgets_listbox");
	if (!list)
		throw std::runtime_error("No \"widgets_listbox\" object in append.ui");

	list->signal_row_selected().connect(sigc::mem_fun(*this, &GPX2VideoAppend::on_selected));


	// OK button object
	ok_button_ = ref_builder_->get_widget<Gtk::Button>("ok_button");
	if (!ok_button_)
		throw std::runtime_error("No \"ok_button\" object in append.ui");

	// Cancel button object
	cancel_button_ = ref_builder_->get_widget<Gtk::Button>("cancel_button");
	if (!cancel_button_)
		throw std::runtime_error("No \"cancel_button\" object in append.ui");

	// Connect signals
	ok_button_->signal_clicked().connect(sigc::mem_fun(*this, &GPX2VideoAppend::on_ok_clicked));
	cancel_button_->signal_clicked().connect(sigc::mem_fun(*this, &GPX2VideoAppend::on_cancel_clicked));
}


GPX2VideoAppend::~GPX2VideoAppend() {
	log_call();
}


GPX2VideoAppend * GPX2VideoAppend::create(Gtk::Window &parent, GPX2VideoRenderer *renderer) {
	log_call();

	// Create a dummy instance before the call to Gtk::Builder::create_from_resource
	// This creation registers GPX2VideoAppend's class in the GType system.
	static_cast<void>(GPX2VideoAppend());

	// Load the Gtk::Builder file and instantiate its widgets.
	auto ref_builder = Gtk::Builder::create_from_resource("/com/progweb/gpx2video/ui/append.ui");

	auto dialog = Gtk::Builder::get_widget_derived<GPX2VideoAppend>(ref_builder, "append_dialog", renderer);
	if (!dialog)
		throw std::runtime_error("No \"append_dialog\" object in append.ui");

	dialog->set_transient_for(parent);

	return dialog;
}


void GPX2VideoAppend::build(void) {
	log_call();

	widget_model_ = Gio::ListStore<GPX2VideoAppend::Widget>::create();

	for (int i=0; i != VideoWidget::WidgetUnknown; i++) {
		VideoWidget::Widget type = (VideoWidget::Widget) i;

		widget_model_->append(Widget::create(
				type,
				VideoWidget::getIconFilename(type),
				VideoWidget::getFriendlyName(type)
			)
		);
	}

	// Widgets list
	auto list = ref_builder_->get_widget<Gtk::ListBox>("widgets_listbox");
	if (!list)
		throw std::runtime_error("No \"widgets_listbox\" object in append.ui");


	list->bind_model(widget_model_, sigc::mem_fun(*this, &GPX2VideoAppend::create_row));
}


Gtk::Widget * GPX2VideoAppend::create_row(const Glib::RefPtr<Glib::ObjectBase> &obj) {
	Glib::RefPtr<const GPX2VideoAppend::Widget> widget = std::dynamic_pointer_cast<const GPX2VideoAppend::Widget>(obj);
	if (!widget)
		return NULL;

	// Build & append widget item
	auto box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 0);
	auto icon = Gtk::make_managed<Gtk::Image>(widget->icon());
	auto label = Gtk::make_managed<Gtk::Label>(widget->label());

	icon->set_halign(Gtk::Align::START);
	icon->set_valign(Gtk::Align::CENTER);
	icon->set_margin(8);
	icon->set_icon_size(Gtk::IconSize::LARGE);

	label->set_halign(Gtk::Align::START);
	label->set_hexpand(true);
	label->set_valign(Gtk::Align::CENTER);
	label->set_xalign(0.0);
	label->set_yalign(1.0);

	box->set_hexpand(true);
	box->append(*icon);
	box->append(*label);

	return box;
}


void GPX2VideoAppend::on_selected(Gtk::ListBoxRow *row) {
	log_call();

	int index;

	Glib::RefPtr<const GPX2VideoAppend::Widget> widget;

	if (row == NULL)
		goto abort;

	// Get selected row
	index = row->get_index();

	// Get item
	widget = widget_model_->get_item(index);

	log_info("New widget '%s' selected", VideoWidget::widget2string(widget->type()).c_str());

	// Enable button
	ok_button_->set_sensitive(true);

abort:
	return;
}


void GPX2VideoAppend::on_ok_clicked(void) {
	log_call();

	int index;

	Glib::RefPtr<const GPX2VideoAppend::Widget> widget;

	// Widgets list
	auto list = ref_builder_->get_widget<Gtk::ListBox>("widgets_listbox");
	if (!list)
		throw std::runtime_error("No \"widgets_listbox\" object in append.ui");

	// Get row selected
	auto row = list->get_selected_row();

	// Get selected row
	index = row->get_index();

	// Get item
	widget = widget_model_->get_item(index);

	// Append widget to layout
	log_info("Request to create & append new widget '%s'", 
			VideoWidget::widget2string(widget->type()).c_str());

	renderer_->append(widget->type());

	close();
}


void GPX2VideoAppend::on_cancel_clicked(void) {
	log_call();

	close();
}

