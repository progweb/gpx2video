#ifndef __GPX2VIDEO__GTK__LAPWIDGETSETTINGSBOX_H__
#define __GPX2VIDEO__GTK__LAPWIDGETSETTINGSBOX_H__

#include "../videowidget.h"
#include "base.h"


class GPX2VideoLapWidgetSettingsBox : public GPX2VideoWidgetBaseSettingsBox {
public:
	GPX2VideoLapWidgetSettingsBox()
		: GPX2VideoWidgetBaseSettingsBox("GPX2VideoLapWidgetSettingsBox") {
	}

	GPX2VideoLapWidgetSettingsBox(BaseObjectType *cobject,
		const Glib::RefPtr<Gtk::Builder> &ref_builder, std::string resource_file, GPX2VideoWidget *widget);

	static GPX2VideoLapWidgetSettingsBox * create(GPX2VideoWidget *widget) {
		log_call();

		const std::string resource_file = "widgets/lap_box.ui";

		// Load the Builder file and instantiate its widgets.
		auto ref_builder = Gtk::Builder::create_from_resource("/com/progweb/gpx2video/ui/" + resource_file);

		auto box = Gtk::Builder::get_widget_derived<GPX2VideoLapWidgetSettingsBox>(ref_builder, "settings_box", resource_file, widget);

		if (!box)
			throw std::runtime_error("No \"settings_box\" object in " + resource_file);

		return box;
	}

	virtual ~GPX2VideoLapWidgetSettingsBox() {
		log_call();
	}

	void load_models(void);
	void bind_content(void);
	void update_content(void);
	void update_boundaries(void);

	void set_default(void);

protected:
};

#endif

