#ifndef __GPX2VIDEO__GTK__IMAGEWIDGETSETTINGSBOX_H__
#define __GPX2VIDEO__GTK__IMAGEWIDGETSETTINGSBOX_H__

#include <gtkmm/menubutton.h>

#include "../videowidget.h"
#include "../media.h"
#include "base.h"


class GPX2VideoImageWidgetSettingsBox : public GPX2VideoWidgetBaseSettingsBox {
public:
	class Image : public Glib::Object {
	public:
		std::string filename_;

		Image(const std::string &filename)
			: Glib::Object()
			, filename_(filename) {
		}

		static Glib::RefPtr<Image> create(const std::string &filename) {
			return Glib::make_refptr_for_instance<Image>(new Image(filename));
		}

		const std::string& filename(void) const {
			return filename_;
		}
	};

	GPX2VideoImageWidgetSettingsBox()
		: GPX2VideoWidgetBaseSettingsBox("GPX2VideoImageWidgetSettingsBox")
		, media_model_(NULL) {
	}

	GPX2VideoImageWidgetSettingsBox(BaseObjectType *cobject,
		const Glib::RefPtr<Gtk::Builder> &ref_builder, std::string resource_file,
		GPX2VideoWidget *widget, const Glib::RefPtr<GPX2VideoMediaListStore> &media_model);

	static GPX2VideoImageWidgetSettingsBox * create(GPX2VideoWidget *widget, const Glib::RefPtr<GPX2VideoMediaListStore> &media_model) {
		log_call();

		const std::string resource_file = "widgets/image_box.ui";

		// Load the Builder file and instantiate its widgets.
		auto ref_builder = Gtk::Builder::create_from_resource("/com/progweb/gpx2video/ui/" + resource_file);

		auto box = Gtk::Builder::get_widget_derived<GPX2VideoImageWidgetSettingsBox>(ref_builder, "settings_box", resource_file,
			   widget, media_model);

		if (!box)
			throw std::runtime_error("No \"settings_box\" object in " + resource_file);

		return box;
	}

	virtual ~GPX2VideoImageWidgetSettingsBox() {
		log_call();
	}

	void load_models(void);
	void bind_content(void);
	void update_content(void);
	void update_boundaries(void);

protected:
	Glib::RefPtr<GPX2VideoMediaListStore> media_model_;

	Glib::RefPtr<Gtk::SingleSelection> image_model_;
	Glib::RefPtr<Gtk::SignalListItemFactory> image_factory_;

	Glib::RefPtr<Gtk::ListStore> zoom_model_;

	void create_popover(Gtk::MenuButton *menubutton);

	void on_image_ok_clicked(void);
	void on_image_zoomin_clicked(void);
	void on_image_zoomout_clicked(void);

	void on_model_changed(guint position, guint removed, guint added);

private:
	int image_pixel_size_;
	int image_pixel_step_;
	int image_pixel_minsize_;
	int image_pixel_maxsize_;

	sigc::connection image_zoomin_connection_;
	sigc::connection image_zoomout_connection_;
	sigc::connection image_ok_connection_;
};

#endif

