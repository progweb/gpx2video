#ifndef __GPX2VIDEO__GTK__WIDGETBASESETTINGSBOX_H__
#define __GPX2VIDEO__GTK__WIDGETBASESETTINGSBOX_H__

#include <gtkmm/builder.h>
#include <gtkmm/box.h>
#include <gtkmm/combobox.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/liststore.h>

#include "../log.h"


class GPX2VideoWidgetBaseSettingsBox : public Gtk::Box {
public:
	GPX2VideoWidgetBaseSettingsBox(const std::string &name)
		: Glib::ObjectBase(name)
		, ref_builder_(NULL) {
		loading_ = false;
	}

	GPX2VideoWidgetBaseSettingsBox(BaseObjectType *cobject,
			const Glib::RefPtr<Gtk::Builder> &ref_builder, std::string name, std::string resource_file)
		: Glib::ObjectBase(name)
		, Gtk::Box(cobject)
		, ref_builder_(ref_builder) 
		, resource_file_(resource_file) {
		loading_ = false;
	}

	virtual ~GPX2VideoWidgetBaseSettingsBox() {
		ref_builder_ = NULL;
	}

	void release(void) {
		ref_builder_ = NULL;
	}

	virtual void update_content(void) = 0;

protected:
	class Model : public Gtk::TreeModel::ColumnRecord {
	public:
		Model() { 
			add(m_id); 
			add(m_name);
			add(m_enable);
		}

		Gtk::TreeModelColumn<int> m_id;
		Gtk::TreeModelColumn<Glib::ustring> m_name;
		Gtk::TreeModelColumn<bool> m_enable;
	};

	Glib::RefPtr<Gtk::Builder> ref_builder_;

	const std::string resource_file_;

	bool loading_;

	Model model_;

	bool find_in_listtore(const Glib::RefPtr<Gtk::ListStore> &store, const int &value, Gtk::TreeModel::iterator &result);

	void on_widget_spin_int_changed(Gtk::SpinButton *button, std::function<void(const int&)> set);
	void on_widget_spin_double_changed(Gtk::SpinButton *button, std::function<void(const double&)> set);
	void on_widget_combobox_changed(Gtk::ComboBox *combobox, std::function<void(const Gtk::TreeModel::const_iterator&)> set);
};

#endif

