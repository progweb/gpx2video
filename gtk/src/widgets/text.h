#ifndef __GPX2VIDEO__GTK__TEXTWIDGETSETTINGSBOX_H__
#define __GPX2VIDEO__GTK__TEXTWIDGETSETTINGSBOX_H__

#include "../videowidget.h"
#include "base.h"


class GPX2VideoTextWidgetSettingsBox : public GPX2VideoWidgetBaseSettingsBox {
public:
	GPX2VideoTextWidgetSettingsBox()
		: GPX2VideoWidgetBaseSettingsBox("GPX2VideoTextWidgetSettingsBox") {
	}

	GPX2VideoTextWidgetSettingsBox(BaseObjectType *cobject,
		const Glib::RefPtr<Gtk::Builder> &ref_builder, std::string resource_file, GPX2VideoWidget *widget);

	static GPX2VideoTextWidgetSettingsBox * create(GPX2VideoWidget *widget) {
		log_call();

		const std::string resource_file = "widgets/text_box.ui";

		// Load the Builder file and instantiate its widgets.
		auto ref_builder = Gtk::Builder::create_from_resource("/com/progweb/gpx2video/ui/" + resource_file);

		auto box = Gtk::Builder::get_widget_derived<GPX2VideoTextWidgetSettingsBox>(ref_builder, "settings_box", resource_file, widget);

		if (!box)
			throw std::runtime_error("No \"settings_box\" object in " + resource_file);

		return box;
	}

	virtual ~GPX2VideoTextWidgetSettingsBox() {
		log_call();
	}

	void bind_content(void);
	void update_content(void);
	void update_boundaries(void);

protected:
	Glib::RefPtr<Gtk::ListStore> view_model_;
};

#endif

