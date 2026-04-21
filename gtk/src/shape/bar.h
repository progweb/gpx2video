#ifndef __GPX2VIDEO__GTK__BARSHAPESETTINGSBOX_H__
#define __GPX2VIDEO__GTK__BARSHAPESETTINGSBOX_H__

#include <gtkmm/liststore.h>

#include "../videowidget.h"
#include "base.h"


class GPX2VideoBarShapeSettingsBox : public GPX2VideoShapeBaseSettingsBox {
public:
	GPX2VideoBarShapeSettingsBox()
		: GPX2VideoShapeBaseSettingsBox("GPX2VideoBarShapeSettingsBox")
		, widget_(NULL) {
	}

	GPX2VideoBarShapeSettingsBox(BaseObjectType *cobject,
		const Glib::RefPtr<Gtk::Builder> &ref_builder, std::string resource_file, GPX2VideoWidget *widget);

	static GPX2VideoBarShapeSettingsBox * create(GPX2VideoWidget *widget) {
		log_call();

		const std::string resource_file = "shape/bar_box.ui";

		// Load the Builder file and instantiate its widgets.
		auto ref_builder = Gtk::Builder::create_from_resource("/com/progweb/gpx2video/ui/" + resource_file);

		auto box = Gtk::Builder::get_widget_derived<GPX2VideoBarShapeSettingsBox>(ref_builder, "settings_box", resource_file, widget);

		if (!box)
			throw std::runtime_error("No \"settings_box\" object in " + resource_file);

		return box;
	}

	virtual ~GPX2VideoBarShapeSettingsBox() {
		log_call();
	}

	void update_content(void);

protected:
	GPX2VideoWidget *widget_;

private:
	Glib::RefPtr<Gtk::ListStore> tick_align_model_;
};

#endif

