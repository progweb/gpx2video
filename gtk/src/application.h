#ifndef __GPX2VIDEO__APPLICATION_H__
#define __GPX2VIDEO__APPLICATION_H__

#include <gtkmm/application.h>
#include <gtkmm/applicationwindow.h>

#include "window.h"


class GPX2VideoApplication : public Gtk::Application {
public:
	static Glib::RefPtr<GPX2VideoApplication> create(void);

protected:
	GPX2VideoApplication();

protected:
	Glib::ustring media_file_;

	int on_handle_local_options(const Glib::RefPtr<Glib::VariantDict>& options) override;
	void on_startup(void);
	void on_activate(void) override;
	void on_open(const Gio::Application::type_vec_files& files,
			const Glib::ustring& hint) override;

	void on_action_about(void);
	void on_action_preferences(void);
	void on_action_quit(void);

private:
	GPX2VideoApplicationWindow * create_application_window(void);
};

#endif

