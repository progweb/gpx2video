#ifndef __GPX2VIDEO__GTK__TEXTSHAPESETTINGSBOX_H__
#define __GPX2VIDEO__GTK__TEXTSHAPESETTINGSBOX_H__

#include <gtkmm.h>
#include <gtkmm/singleselection.h>

#include "../videowidget.h"
#include "base.h"


class GPX2VideoTextShapeSettingsBox : public GPX2VideoShapeBaseSettingsBox {
public:
	class Icon : public Glib::Object {
	public:
		VideoWidget::Theme::Icon id_;
		std::string filename_;

		Icon(VideoWidget::Theme::Icon id, const std::string &filename)
			: Glib::Object()
			, id_(id)
			, filename_(filename) {
		}

		static Glib::RefPtr<Icon> create(VideoWidget::Theme::Icon id, const std::string &filename) {
			return Glib::make_refptr_for_instance<Icon>(new Icon(id, filename));
		}

		VideoWidget::Theme::Icon icon(void) const {
			return id_;
		}

		const std::string& filename(void) const {
			return filename_;
		}
	};

	GPX2VideoTextShapeSettingsBox()
		: GPX2VideoShapeBaseSettingsBox("GPX2VideoTextShapeSettingsBox") {
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

	void bind_content(void);
	void update_content(void);

protected:
	Glib::RefPtr<Gtk::SingleSelection> icon_model_;

	void on_icon_clicked(void);
	void on_icon_toggled(void);
};

#endif

