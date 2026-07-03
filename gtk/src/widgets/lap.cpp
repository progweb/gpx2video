#include <glibmm/i18n.h>

#include "../log_i.h"
//#include "../../src/videowidget.h"
#include "../../../src/widgets/lap.h"
#include "lap.h"


GPX2VideoLapWidgetSettingsBox::GPX2VideoLapWidgetSettingsBox(BaseObjectType *cobject,
	const Glib::RefPtr<Gtk::Builder> &ref_builder, std::string resource_file, GPX2VideoWidget *widget) 
	: GPX2VideoWidgetBaseSettingsBox(cobject, ref_builder, "GPX2VideoLapWidgetSettingsBox", resource_file, widget) { 
	log_call();

	// Populate models
	load_models();

	// Binding
	bind_content();

	// Update ui
	update_content();
	update_boundaries();
}


void GPX2VideoLapWidgetSettingsBox::load_models(void) {
	log_call();
}


void GPX2VideoLapWidgetSettingsBox::bind_content(void) {
	log_call();

	Gtk::SpinButton *spinbutton;

	// Connect widgets button
	//------------------------

	// Lap
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("lap_total_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"lap_total_spinbutton\" object in " + resource_file_);

	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoLapWidgetSettingsBox::on_widget_spin_int_changed), spinbutton, 
					[this](const int &value) {
						log_notice("Widget %s: lap total changed to '%d'",
							   widget_->name().c_str(), value);

						((LapWidget *) widget_->widget())->setTargetLap(value);

						// Broadcast widget change
						widget_->dispatchEvent(true);
					}
			));
}


void GPX2VideoLapWidgetSettingsBox::update_content(void) {
	log_call();

	Gtk::SpinButton *spinbutton;

	// Mask value changed
	loading_ = true;

	// Lap
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("lap_total_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"lap_total_spinbutton\" object in " + resource_file_);

	spinbutton->set_value(((LapWidget *) widget_->widget())->targetLap());

	// Unmask value changed
	loading_ = false;
}


void GPX2VideoLapWidgetSettingsBox::set_default(void) {
	log_call();
}


void GPX2VideoLapWidgetSettingsBox::update_boundaries(void) {
	log_call();
}

