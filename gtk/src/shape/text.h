#ifndef __GPX2VIDEO__GTK__TEXTSHAPESETTINGSBOX_H__
#define __GPX2VIDEO__GTK__TEXTSHAPESETTINGSBOX_H__

#include <gtkmm/liststore.h>

#include "../videowidget.h"
#include "base.h"


class GPX2VideoTextShapeSettingsBox : public GPX2VideoShapeBaseSettingsBox {
public:
	GPX2VideoTextShapeSettingsBox()
		: GPX2VideoShapeBaseSettingsBox("GPX2VideoTextShapeSettingsBox")
		, widget_(NULL) {
	}

	GPX2VideoTextShapeSettingsBox(BaseObjectType *cobject,
		const Glib::RefPtr<Gtk::Builder> &ref_builder, std::string resource_file, GPX2VideoWidget *widget);

	static GPX2VideoTextShapeSettingsBox * create(GPX2VideoWidget *widget) {
		log_call();

		const std::string resource_file = "shape/text_box.ui";

		// Load the Builder file and instantiate its widgets.
		auto ref_builder = Gtk::Builder::create_from_resource("/com/progweb/gpx2video/ui/" + resource_file);

		auto box = Gtk::Builder::get_widget_derived<GPX2VideoTextShapeSettingsBox>(ref_builder, "settings_box", resource_file, widget);

		if (!box)
			throw std::runtime_error("No \"settings_box\" object in " + resource_file);

		return box;
	}

	virtual ~GPX2VideoTextShapeSettingsBox() {
		log_call();
	}

	void update_content(void);

protected:
	GPX2VideoWidget *widget_;

	bool find_in_listtore(const Glib::RefPtr<Gtk::ListStore> &store, const int &value, Gtk::TreeModel::iterator &result) {
		log_call();

		for (auto iter = store->children().begin(); iter != store->children().end(); iter++) {
			if (iter->get_value(model_.m_id) != value)
				continue;

			result = iter;

			return true;
		}

		return false;
	}

private:
	class Model : public Gtk::TreeModel::ColumnRecord {
	public:
		Model() { 
			add(m_id); 
			add(m_name);
		}

		Gtk::TreeModelColumn<int> m_id;
		Gtk::TreeModelColumn<Glib::ustring> m_name;
	};

	Model model_;
	Glib::RefPtr<Gtk::ListStore> tick_align_model_;
};

#endif

