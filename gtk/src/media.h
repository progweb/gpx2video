#ifndef __GPX2VIDEO__GTK__MEDIA_H__
#define __GPX2VIDEO__GTK__MEDIA_H__

#include <string>

#include <glibmm/object.h>

#include <gtkmm.h>


class GPX2VideoMedia : public Glib::Object {
public:
	enum Media {
		MediaNone = -1,
		MediaAudio,
		MediaIcon,
		MediaImage,
		MediaUnknown
	};

	Media media_;
	std::string filename_;

	static Glib::RefPtr<GPX2VideoMedia> create(Media media, const Glib::ustring &filename) {
		return Glib::make_refptr_for_instance<GPX2VideoMedia>(new GPX2VideoMedia(media, filename));
	}

	Media media(void) {
		return media_;
	}

	const std::string& filename(void) const {
		return filename_;
	}

	static std::string media2string(Media media);

protected:
	GPX2VideoMedia(Media media, const std::string &filename)
		: Glib::Object()
		, media_(media)
		, filename_(filename) {
	}

};


class GPX2VideoMediaListStore : public Gio::ListStore<GPX2VideoMedia> {
public:
	static Glib::RefPtr<GPX2VideoMediaListStore> create(void) {
		return Glib::make_refptr_for_instance<GPX2VideoMediaListStore>(new GPX2VideoMediaListStore());
	}

	void append(GPX2VideoMedia::Media media, const std::string &filename) {
		Gio::ListStore<GPX2VideoMedia>::append(GPX2VideoMedia::create(media, filename));
	}

protected:
	GPX2VideoMediaListStore()
		: Gio::ListStore<GPX2VideoMedia>() {
	}
};


#endif

