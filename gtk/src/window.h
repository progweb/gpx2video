#ifndef __GPX2VIDEO__WINDOW_H__
#define __GPX2VIDEO__WINDOW_H__

#include <gtkmm/application.h>
#include <gtkmm/applicationwindow.h>
#include <gtkmm/builder.h>
//#include <gtkmm/stack.h>
#include <gtkmm/menubutton.h>

#include "area.h"


class GPX2VideoApplicationWindow : public Gtk::ApplicationWindow {
public:
	GPX2VideoApplicationWindow(BaseObjectType *cobject,
			const Glib::RefPtr<Gtk::Builder> &ref_builder);
	~GPX2VideoApplicationWindow();

	static GPX2VideoApplicationWindow * create(void);

	void open_file_view(const Glib::RefPtr<Gio::File> &file);

protected:
	Glib::RefPtr<Gtk::Builder> ref_builder_;

	Glib::ustring media_file_;

//    Gtk::Stack *stack_ = NULL;
    Gtk::MenuButton *gears_ = NULL;

	GPX2VideoArea *video_area_ = NULL;

private:
};

#endif

