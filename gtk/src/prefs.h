#ifndef __GPX2VIDEO__PREFS_H__
#define __GPX2VIDEO__PREFS_H__

#include <gtkmm/builder.h>
#include <gtkmm/dialog.h>


class GPX2VideoPrefs : public Gtk::Dialog {
public:
	GPX2VideoPrefs();
	GPX2VideoPrefs(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder>& ref_builder);
	virtual ~GPX2VideoPrefs();

	static GPX2VideoPrefs * create(Gtk::Window &parent);

protected:
	Glib::RefPtr<Gtk::Builder> ref_builder_;

private:
};

#endif

