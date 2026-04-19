#ifndef __GPX2VIDEO__GTK__WIDGETFRAME_H__
#define __GPX2VIDEO__GTK__WIDGETFRAME_H__

#include <gtkmm/builder.h>
#include <glibmm/dispatcher.h>
#include <gtkmm/box.h>
#include <gtkmm/frame.h>
#include <gtkmm/switch.h>
#include <gtkmm/fontbutton.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/colorbutton.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/liststore.h>

#include <pangomm/fontdescription.h>

#include "renderer.h"
#include "videowidget.h"
#include "shape/base.h"
#include "widgets/base.h"


class GPX2VideoWidgetFrame : public Gtk::Frame {
public:
	GPX2VideoWidgetFrame();
	GPX2VideoWidgetFrame(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &ref_builder);
	virtual ~GPX2VideoWidgetFrame();

	void set_renderer(GPX2VideoRenderer *renderer);

	void set_visible(bool visible);

	void update(void) {
		update_content();
	}

	GPX2VideoWidget * widget_selected(void);
	void set_widget_selected(GPX2VideoWidget *widget);

	Glib::Dispatcher& signal_widget_changed(void) {
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

	Model model_;
	Glib::RefPtr<Gtk::ListStore> shape_model_;
	Glib::RefPtr<Gtk::ListStore> position_model_;
	Glib::RefPtr<Gtk::ListStore> orientation_model_;
	Glib::RefPtr<Gtk::ListStore> font_style_model_;
	Glib::RefPtr<Gtk::ListStore> text_align_model_;
	Glib::RefPtr<Gtk::ListStore> label_font_weight_model_;
	Glib::RefPtr<Gtk::ListStore> value_font_weight_model_;

	Glib::RefPtr<Gtk::ListStore> duplicate_liststore(const Glib::RefPtr<Gtk::ListStore> &source, class Model &columns);
	bool find_in_listtore(const Glib::RefPtr<Gtk::ListStore> &store, const int &value, Gtk::TreeModel::iterator &result);

	void build_shape_settings(void);
	void build_widget_settings(void);

	void bind_content(void);

	void update_content(void);
	void update_boundaries(void);

	void update_shape_content(void);
	void update_widget_content(void);

	void update_font_weight_model(const Glib::RefPtr<Gtk::ListStore> &store, const std::string &value);

	void on_widget_margin_value_changed(const VideoWidget::Margin &margin);
	void on_widget_padding_value_changed(const VideoWidget::Theme::Padding &padding);

	void on_widget_font_changed(Gtk::FontButton *button, std::function<void(const Pango::FontDescription&)> set);
	void on_widget_spin_changed(Gtk::SpinButton *button, std::function<void(const int&)> set);
	void on_widget_color_changed(Gtk::ColorButton *button, std::function<void(const std::string&)> set);
	void on_widget_entry_changed(Gtk::Entry *entry, std::function<void(const Glib::ustring&)> set);
	void on_widget_combobox_changed(Gtk::ComboBox *combobox, std::function<void(const Gtk::TreeModel::const_iterator&)> set);
	bool on_widget_switch_changed(bool state, Gtk::Switch *sw, std::function<void(const bool&)> set);

	void on_widget_changed(void);

private:
	GPX2VideoRenderer *renderer_;

	GPX2VideoWidget *widget_selected_;

	GPX2VideoShapeBaseSettingsBox *shape_child_box_;
	GPX2VideoWidgetBaseSettingsBox *widget_child_box_;

	sigc::connection sigc_connection_;

	bool loading_;
};

#endif

