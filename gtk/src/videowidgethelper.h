#ifndef __GPX2VIDEO__GTK__VIDEOWIDGETHELPER_H__
#define __GPX2VIDEO__GTK__VIDEOWIDGETHELPER_H__

#include <gtkmm/builder.h>
#include <gtkmm/switch.h>
#include <gtkmm/combobox.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/colorbutton.h>

#include "log.h"


class GPX2VideoWidgetHelper {
public:
	GPX2VideoWidgetHelper() 
		: ref_builder_(NULL) {
	}

	virtual ~GPX2VideoWidgetHelper() {
	}

protected:
	Glib::RefPtr<Gtk::Builder> ref_builder_;

	std::string resource_file_;

	bool loading_;

	Glib::RefPtr<Gtk::Builder> create_from_resource(std::string resource_file) {
		resource_file_ = resource_file;

		return Gtk::Builder::create_from_resource("/com/progweb/gpx2video/ui/" + resource_file);
	}

	void unload_content(void) {
		ref_builder_ = NULL;
	}

	void on_widget_spin_changed(Gtk::SpinButton *button, std::function<void(const int&)> set);
	void on_widget_color_changed(Gtk::ColorButton *button, std::function<void(const std::string&)> set);
	void on_widget_combobox_changed(Gtk::ComboBox *combobox, std::function<void(const Gtk::TreeModel::const_iterator&)> set);
	bool on_widget_switch_changed(bool state, Gtk::Switch *sw, std::function<void(const bool&)> set);
};

#endif

