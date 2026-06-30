#ifndef __GPX2VIDEO__GTK__TRACKWIDGETSETTINGSBOX_H__
#define __GPX2VIDEO__GTK__TRACKWIDGETSETTINGSBOX_H__

#include <gtkmm/menubutton.h>

#include <librsvg/rsvg.h>
#include <cairomm/cairomm.h>

#include "../../src/track.h"
#include "../videowidget.h"
#include "../media.h"
#include "base.h"


class GPX2VideoTrackWidgetSettingsBox : public GPX2VideoWidgetBaseSettingsBox {
public:
	class Icon : public Glib::Object {
	public:
		Icon(TrackSettings::Icon id, const std::string &filename)
			: Glib::Object()
			, pixbuf_(NULL) 
			, id_(id)
			, filename_(filename) {
			load_and_crop_svg();
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

		const Glib::RefPtr<Gdk::Pixbuf>& pixbuf(void) const {
			return pixbuf_;
		}   

	protected:
		Glib::RefPtr<Gdk::Pixbuf> pixbuf_;

		TrackSettings::Icon id_;
		std::string filename_;

		void load_and_crop_svg(void);
	};

	GPX2VideoTrackWidgetSettingsBox()
		: GPX2VideoWidgetBaseSettingsBox("GPX2VideoTrackWidgetSettingsBox")
		, media_model_(NULL) {
	}

	GPX2VideoTrackWidgetSettingsBox(BaseObjectType *cobject,
		const Glib::RefPtr<Gtk::Builder> &ref_builder, std::string resource_file, 
		GPX2VideoWidget *widget, const Glib::RefPtr<GPX2VideoMediaListStore> &media_model);

	static GPX2VideoTrackWidgetSettingsBox * create(GPX2VideoWidget *widget, const Glib::RefPtr<GPX2VideoMediaListStore> &media_model) {
		log_call();

		const std::string resource_file = "widgets/track_box.ui";

		// Load the Builder file and instantiate its widgets.
		auto ref_builder = Gtk::Builder::create_from_resource("/com/progweb/gpx2video/ui/" + resource_file);

		auto box = Gtk::Builder::get_widget_derived<GPX2VideoTrackWidgetSettingsBox>(ref_builder, "settings_box", resource_file, 
				widget, media_model);

		if (!box)
			throw std::runtime_error("No \"settings_box\" object in " + resource_file);

		return box;
	}

	virtual ~GPX2VideoTrackWidgetSettingsBox() {
		log_call();
	}

	void load_models(void);
	void bind_content(void);
	void update_content(void);
	void update_boundaries(void);

	void set_default(void);

protected:
	Glib::RefPtr<GPX2VideoMediaListStore> media_model_;

	Glib::RefPtr<Gtk::SingleSelection> icon_model_;
	Glib::RefPtr<Gtk::SignalListItemFactory> icon_factory_;

	Glib::RefPtr<Gtk::ListStore> view_model_;
	Glib::RefPtr<Gtk::ListStore> follow_model_;

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

	Glib::RefPtr<GPX2VideoTrackWidgetSettingsBox::Icon> find_icon(const std::string &filename);
};

#endif

