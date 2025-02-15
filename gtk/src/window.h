#ifndef __GPX2VIDEO__GTK__WINDOW_H__
#define __GPX2VIDEO__GTK__WINDOW_H__

#include <gtkmm/version.h>
#include <gtkmm/application.h>
#include <gtkmm/applicationwindow.h>
#include <gtkmm/builder.h>
//#include <gtkmm/stack.h>
#include <gtkmm/menubutton.h>
#include <gtkmm/button.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/scale.h>
#if GTKMM_CHECK_VERSION(4, 10, 0)
#include <gtkmm/filedialog.h>
#else
#include <gtkmm/filechooserdialog.h>
#endif

#include "../../src/application.h"
#include "../../src/renderer.h"
#include "../../src/telemetrysettings.h"
#include "renderer.h"
#include "area.h"


class GPX2VideoApplicationWindow : public Gtk::ApplicationWindow, public GPXApplication {
public:
	GPX2VideoApplicationWindow(BaseObjectType *cobject,
			const Glib::RefPtr<Gtk::Builder> &ref_builder);
	~GPX2VideoApplicationWindow();

	static GPX2VideoApplicationWindow * create(void);

	void open_media_file(const Glib::RefPtr<const Gio::File> &file);
	void open_layout_file(const Glib::RefPtr<const Gio::File> &file);
	void open_telemetry_file(const Glib::RefPtr<const Gio::File> &file);

protected:
	Glib::RefPtr<Gtk::Builder> ref_builder_;

	Glib::ustring media_file_;
	Glib::ustring layout_file_;

//    Gtk::Stack *stack_ = NULL;
    Gtk::MenuButton *gears_ = NULL;

	Gtk::Button *play_button_ = NULL;

	Gtk::Scale *progress_scale_ = NULL;

	GPX2VideoArea *video_area_ = NULL;

	void on_action_open(void);
	void on_action_append(void);

	void on_play_clicked(void);
	bool on_progress_change_value(Gtk::ScrollType type, double value, const Glib::RefPtr<Gtk::Adjustment> &adjustment);
	void on_progress_scale_pressed(guint n, double x, double y);
	void on_progress_scale_released(guint n, double x, double y);
	bool on_key_pressed(guint, guint, Gdk::ModifierType);

#if GTKMM_CHECK_VERSION(4, 10, 0)
	void on_file_dialog_open_clicked(const Glib::RefPtr<Gio::AsyncResult> &result,
			const Glib::RefPtr<Gtk::FileDialog> &dialog);
#else
	void on_file_dialog_open_clicked(int response_id,
			Gtk::FileChooserDialog *dialog);
#endif

private:
	RendererSettings renderer_settings_;
	TelemetrySettings telemetry_settings_;

	MediaContainer *media_;

	GPX2VideoRenderer *renderer_;
};

#endif

