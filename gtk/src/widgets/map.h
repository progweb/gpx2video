#ifndef __GPX2VIDEO__GTK__MAPWIDGETSETTINGSBOX_H__
#define __GPX2VIDEO__GTK__MAPWIDGETSETTINGSBOX_H__

#include <gtkmm/menubutton.h>

#include "../../src/map.h"
#include "../videowidget.h"
#include "../media.h"
#include "base.h"


class GPX2VideoMapWidgetSettingsBox : public GPX2VideoWidgetBaseSettingsBox {
public:
	class Icon : public Glib::Object {
	public:
		TrackSettings::Icon id_;
		std::string filename_;

		Icon(TrackSettings::Icon id, const std::string &filename)
			: Glib::Object()
			, id_(id)
			, filename_(filename) {
		}

		static Glib::RefPtr<Icon> create(TrackSettings::Icon id, const std::string &filename) {
			return Glib::make_refptr_for_instance<Icon>(new Icon(id, filename));
		}

		TrackSettings::Icon icon(void) const {
			return id_;
		}

		const std::string& filename(void) const {
			return filename_;
		}
	};

	GPX2VideoMapWidgetSettingsBox()
		: GPX2VideoWidgetBaseSettingsBox("GPX2VideoMapWidgetSettingsBox")
		, media_model_(NULL) {
	}

	GPX2VideoMapWidgetSettingsBox(BaseObjectType *cobject,
		const Glib::RefPtr<Gtk::Builder> &ref_builder, std::string resource_file,
		GPX2VideoWidget *widget, const Glib::RefPtr<GPX2VideoMediaListStore> &media_model);

	static GPX2VideoMapWidgetSettingsBox * create(GPX2VideoWidget *widget, const Glib::RefPtr<GPX2VideoMediaListStore> &media_model) {
		log_call();

		const std::string resource_file = "widgets/map_box.ui";

		// Load the Builder file and instantiate its widgets.
		auto ref_builder = Gtk::Builder::create_from_resource("/com/progweb/gpx2video/ui/" + resource_file);

		auto box = Gtk::Builder::get_widget_derived<GPX2VideoMapWidgetSettingsBox>(ref_builder, "settings_box", resource_file, 
				widget, media_model);

		if (!box)
			throw std::runtime_error("No \"settings_box\" object in " + resource_file);

		return box;
	}

	virtual ~GPX2VideoMapWidgetSettingsBox() {
		log_call();
	}

	void load_models(void);
	void bind_content(void);
	void update_content(void);
	void update_boundaries(void);

protected:
	Glib::RefPtr<GPX2VideoMediaListStore> media_model_;

	Glib::RefPtr<Gtk::SingleSelection> icon_model_;
	Glib::RefPtr<Gtk::SignalListItemFactory> icon_factory_;

	Glib::RefPtr<Gtk::ListStore> view_model_;
	Glib::RefPtr<Gtk::ListStore> source_model_;

	void create_popover(Gtk::MenuButton *menubutton, TrackSettings::Icon type);

	void on_icon_ok_clicked(TrackSettings::Icon type);
	void on_icon_usedefault_toggled(TrackSettings::Icon type);
	void on_icon_zoomin_clicked(TrackSettings::Icon type);
	void on_icon_zoomout_clicked(TrackSettings::Icon type);

	void on_model_changed(guint position, guint removed, guint added);

private:
	int icon_pixel_size_;
	int icon_pixel_minsize_;
	int icon_pixel_maxsize_;

	sigc::connection icon_checkbutton_connection_;
	sigc::connection icon_zoomin_connection_;
	sigc::connection icon_zoomout_connection_;
	sigc::connection icon_ok_connection_;
};

#endif

