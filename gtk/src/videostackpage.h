#ifndef __GPX2VIDEO__GTK__VIDEOSTACKPAGE_H__
#define __GPX2VIDEO__GTK__VIDEOSTACKPAGE_H__

#include <gtkmm/builder.h>
#include <gtkmm/box.h>

#include "../../src/media.h"


class GPX2VideoVideoStackPage : public Gtk::Box {
public:
	GPX2VideoVideoStackPage(BaseObjectType *cobject,
			const Glib::RefPtr<Gtk::Builder> &ref_builder, std::string resource_file);

	virtual ~GPX2VideoVideoStackPage() {
	}

	static GPX2VideoVideoStackPage * create(void);

	std::string name(void) {
		return "video_page";
	}

	std::string title(void) {
		return "Video";
	}

	std::string icon_name(void) {
		return "camera-video-symbolic";
	}

	void set_media(MediaContainer *media);

protected:
	Glib::RefPtr<Gtk::Builder> ref_builder_;

	const std::string resource_file_;
};

#endif

