#ifndef __GPX2VIDEO__GTK__TEXTSHAPESETTINGSBOX_H__
#define __GPX2VIDEO__GTK__TEXTSHAPESETTINGSBOX_H__

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

private:
	Glib::RefPtr<Gtk::ListStore> tick_align_model_;
};

#endif

