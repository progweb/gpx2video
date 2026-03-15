#ifndef __GPX2VIDEO__GTK__BASESETTINGSBOX_H__
#define __GPX2VIDEO__GTK__BASESETTINGSBOX_H__

#include <gtkmm/builder.h>
#include <gtkmm/box.h>
#include <gtkmm/switch.h>
#include <gtkmm/combobox.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/colorbutton.h>

#include "../log.h"


class GPX2VideoBaseSettingsBox : public Gtk::Box {
public:
	GPX2VideoBaseSettingsBox(const std::string &name)
		: Glib::ObjectBase(name)
		, ref_builder_(NULL) {
		loading_ = false;
	}

	GPX2VideoBaseSettingsBox(BaseObjectType *cobject,
			const Glib::RefPtr<Gtk::Builder> &ref_builder, std::string name, std::string resource_file)
		: Glib::ObjectBase(name)
		, Gtk::Box(cobject)
		, ref_builder_(ref_builder) 
		, resource_file_(resource_file) {
		loading_ = false;
	}

	virtual ~GPX2VideoBaseSettingsBox() {
		ref_builder_ = NULL;
	}

	void release(void) {
		ref_builder_ = NULL;
	}

	virtual void update_content(void) = 0;

protected:
	Glib::RefPtr<Gtk::Builder> ref_builder_;

	const std::string resource_file_;

	bool loading_;

	void on_widget_spin_changed(Gtk::SpinButton *button, std::function<void(const int&)> set);
	void on_widget_color_changed(Gtk::ColorButton *button, std::function<void(const std::string&)> set);
	void on_widget_combobox_changed(Gtk::ComboBox *combobox, std::function<void(const Gtk::TreeModel::const_iterator&)> set);
	bool on_widget_switch_changed(bool state, Gtk::Switch *sw, std::function<void(const bool&)> set);
};

#endif

