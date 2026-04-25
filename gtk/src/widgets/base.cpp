#include "../log.h"
#include "base.h"


bool GPX2VideoWidgetBaseSettingsBox::find_in_listtore(const Glib::RefPtr<Gtk::ListStore> &store, const int &value, Gtk::TreeModel::iterator &result) {
	log_call();

	for (auto iter = store->children().begin(); iter != store->children().end(); iter++) {
		if (iter->get_value(model_.m_id) != value)
			continue;

		result = iter;

		return true;
	}

	return false;
}


void GPX2VideoWidgetBaseSettingsBox::on_widget_spin_int_changed(Gtk::SpinButton *button, std::function<void(const int&)> set) {
	log_call();

	int value;

	if (loading_)
		return;

	value = button->get_value_as_int();

	// Set value
	set(value);
}


void GPX2VideoWidgetBaseSettingsBox::on_widget_spin_double_changed(Gtk::SpinButton *button, std::function<void(const double&)> set) {
	log_call();

	double value;

	if (loading_)
		return;

	value = button->get_value();

	// Set value
	set(value);
}


void GPX2VideoWidgetBaseSettingsBox::on_widget_color_changed(Gtk::ColorButton *button, std::function<void(const std::string&)> set) {
	log_call();

	Gdk::RGBA rgba;

	Glib::ustring color;

	if (loading_)
		return;

	rgba = button->get_rgba();

	// Convert to hexa string color
	color = Glib::ustring::sprintf("#%02X%02X%02X%02X",
			(unsigned char) std::round(rgba.get_red() * 255),
			(unsigned char) std::round(rgba.get_green() * 255),
			(unsigned char) std::round(rgba.get_blue() * 255),
			(unsigned char) std::round(rgba.get_alpha() * 255)
	);

	// Set color
	set(color);
}


void GPX2VideoWidgetBaseSettingsBox::on_widget_combobox_changed(Gtk::ComboBox *combobox, std::function<void(const Gtk::TreeModel::const_iterator&)> set) {
	log_call();

	if (loading_)
		return;

	// Set combobox
	set(combobox->get_active());
}


bool GPX2VideoWidgetBaseSettingsBox::on_widget_switch_changed(bool state, Gtk::Switch *sw, std::function<void(const bool&)> set) {
	log_call();

	if (loading_)
		return false;

	// Text enable
	sw->set_state(state);

	// Set state
	set(state);

	return true;
}

