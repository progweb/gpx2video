#ifndef __GPX2VIDEO__GTK__SETTINGSFRAME_H__
#define __GPX2VIDEO__GTK__SETTINGSFRAME_H__

#include <glibmm/dispatcher.h>

#include <gtkmm/builder.h>
#include <gtkmm/frame.h>
#include <gtkmm/fontbutton.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/colorbutton.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/liststore.h>

#include <pangomm/fontdescription.h>

#include "settings.h"


class GPX2VideoSettingsFrame : public Gtk::Frame {
public:
	GPX2VideoSettingsFrame();
	GPX2VideoSettingsFrame(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &ref_builder);
	virtual ~GPX2VideoSettingsFrame();

	void set_visible(bool visible);

	void set_section(GPX2VideoSettings::Section section);

	void update(void) {
		update_content();
	}

	Glib::Dispatcher& signal_widget_load_default_settings_requested(void) {
		return dispatcher_;
	}

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

	Glib::Dispatcher dispatcher_;

	GPX2VideoSettings::Section settings_section_;

	Model model_;
	Glib::RefPtr<Gtk::ListStore> font_style_model_;
	Glib::RefPtr<Gtk::ListStore> text_horizontal_align_model_;
	Glib::RefPtr<Gtk::ListStore> text_vertical_align_model_;
	Glib::RefPtr<Gtk::ListStore> label_font_weight_model_;
	Glib::RefPtr<Gtk::ListStore> value_font_weight_model_;
	Glib::RefPtr<Gtk::ListStore> unit_font_weight_model_;
	
	Glib::RefPtr<Gtk::ListStore> duplicate_liststore(const Glib::RefPtr<Gtk::ListStore> &source, class Model &columns);
	bool find_in_listtore(const Glib::RefPtr<Gtk::ListStore> &store, const int &value, Gtk::TreeModel::iterator &result);

	void load_models(void);

	void bind_content(void);

	void update_content(void);
	void update_boundaries(void);

	void update_font_weight_model(const Glib::RefPtr<Gtk::ListStore> &store, const std::string &value);

	void on_font_changed(Gtk::FontButton *button, std::function<void(const Pango::FontDescription&)> set);
	void on_spin_int_changed(Gtk::SpinButton *button, std::function<void(const int&)> set);
	void on_spin_double_changed(Gtk::SpinButton *button, std::function<void(const double&)> set);
	void on_color_changed(Gtk::ColorButton *button, std::function<void(const std::string&)> set);
	void on_combobox_changed(Gtk::ComboBox *combobox, std::function<void(const Gtk::TreeModel::const_iterator&)> set);

	void on_apply_clicked(void);

private:
	bool loading_;
	bool is_visible_;
};

#endif

