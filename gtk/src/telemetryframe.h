#ifndef __GPX2VIDEO__GTK__TELEMETRYFRAME_H__
#define __GPX2VIDEO__GTK__TELEMETRYFRAME_H__

#include <glibmm/dispatcher.h>

#include <gtkmm/builder.h>
#include <gtkmm/frame.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/liststore.h>

#include "../../src/telemetrymedia.h"


class GPX2VideoTelemetryFrame : public Gtk::Frame {
public:
	GPX2VideoTelemetryFrame();
	GPX2VideoTelemetryFrame(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &ref_builder);
	virtual ~GPX2VideoTelemetryFrame();

	void set_telemetry(TelemetrySource *source);

	void set_visible(bool visible);

	void update(void) {
		update_content();
	}

	Glib::Dispatcher& signal_telemetry_changed(void) {
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
	Glib::RefPtr<Gtk::ListStore> prediction_method_model_;
	Glib::RefPtr<Gtk::ListStore> grade_smooth_method_model_;
	Glib::RefPtr<Gtk::ListStore> elevation_smooth_method_model_;
	Glib::RefPtr<Gtk::ListStore> speed_smooth_method_model_;
	Glib::RefPtr<Gtk::ListStore> acceleration_smooth_method_model_;

	Glib::RefPtr<Gtk::ListStore> duplicate_liststore(const Glib::RefPtr<Gtk::ListStore> &source, class Model &columns);
	bool find_in_listtore(const Glib::RefPtr<Gtk::ListStore> &store, const int &value, Gtk::TreeModel::iterator &result);

	void bind_content(void);

	void update_content(void);

	void on_telemetry_spin_changed(Gtk::SpinButton *button, std::function<void(const int&)> set);
	void on_telemetry_combobox_changed(Gtk::ComboBox *combobox, std::function<void(const Gtk::TreeModel::const_iterator&)> set);
	void on_telemetry_checkbutton_toggled(Gtk::CheckButton *button, std::function<void(const int&)> set);

private:
	TelemetrySource *source_;

	bool loading_;
	bool is_visible_;
};

#endif
