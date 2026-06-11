#ifndef __GPX2VIDEO__GTK__MEDIASTACKPAGE_H__
#define __GPX2VIDEO__GTK__MEDIASTACKPAGE_H__

#include <glibmm/dispatcher.h>

#include <gtkmm/builder.h>
#include <gtkmm/box.h>

#include "media.h"


class GPX2VideoMediaStackPage : public Gtk::Box {
public:
	GPX2VideoMediaStackPage(BaseObjectType *cobject,
			const Glib::RefPtr<Gtk::Builder> &ref_builder, std::string resource_file);

	virtual ~GPX2VideoMediaStackPage() {
	}

	static GPX2VideoMediaStackPage * create(void);

	std::string name(void) {
		return "media_page";
	}

	std::string title(void) {
		return "Media";
	}

	std::string icon_name(void) {
		return "image-x-generic-symbolic";
	}

	enum GPX2VideoMedia::Media get_media(void) const {
		return media_;
	}

	Glib::Dispatcher& signal_media_changed(void) {
		return dispatcher_;
	}

protected:
	Glib::RefPtr<Gtk::Builder> ref_builder_;

	Glib::Dispatcher dispatcher_;

	const std::string resource_file_;

	GPX2VideoMedia::Media media_;

private:
	void on_selected(Gtk::ListBoxRow *row);
};

#endif

