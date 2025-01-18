#ifndef __GPX2VIDEO__GTK__ABOUT_H__
#define __GPX2VIDEO__GTK__ABOUT_H__

#include <gtkmm/builder.h>
#include <gtkmm/aboutdialog.h>


class GPX2VideoAbout : public Gtk::AboutDialog {
public:
	GPX2VideoAbout();
	GPX2VideoAbout(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder>& ref_builder);
	virtual ~GPX2VideoAbout();

	static GPX2VideoAbout * create(Gtk::Window &parent);

protected:
	Glib::RefPtr<Gtk::Builder> ref_builder_;

private:
};

#endif

