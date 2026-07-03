#ifndef __GPX2VIDEO__GTK__WIDGETSTACKPAGE_H__
#define __GPX2VIDEO__GTK__WIDGETSTACKPAGE_H__

#include <glibmm/i18n.h>

#include <gtkmm/builder.h>
#include <gtkmm/label.h>
#include <gtkmm/button.h>
#include <gtkmm/listbox.h>
#include <gtkmm/scrolledwindow.h>

#include "renderer.h"
#include "videowidget.h"


class GPX2VideoWidgetStackPage : public Gtk::ScrolledWindow {
public:
	GPX2VideoWidgetStackPage(BaseObjectType *cobject,
			const Glib::RefPtr<Gtk::Builder> &ref_builder, std::string resource_file, Gtk::Window &parent);

	virtual ~GPX2VideoWidgetStackPage() {
	}

	static void init(GPX2VideoWidgetStackPage *klass);

	static GPX2VideoWidgetStackPage * create(Gtk::Window &parent);

	std::string name(void) {
		return "widget_page";
	}

	std::string title(void) {
		return _("Widgets");
	}

	std::string icon_name(void) {
		return "power-profile-performance-symbolic";
	}

	void set_renderer(GPX2VideoRenderer *renderer);

	void append(GPX2VideoWidget *widget);
	void remove(GPX2VideoWidget *widget);
	void purge(void);

	void set_widget_selected(GPX2VideoWidget *widget);

	void on_append_clicked(void);

//	Glib::SignalProxy<void()> signal_widget_selected() {
//		return Glib::SignalProxy<void()>(this, &signal_info_);
//	}

	// signal accessor:
	using type_signal_widget_selected = sigc::signal<void(GPX2VideoWidget *)>;
	type_signal_widget_selected signal_widget_selected();

	using type_signal_widget_remove_clicked = sigc::signal<void(GPX2VideoWidget *)>;
	type_signal_widget_remove_clicked signal_widget_remove_clicked();

	using type_signal_widget_visible_changed = sigc::signal<void(GPX2VideoWidget *)>;
	type_signal_widget_visible_changed signal_widget_visible_changed();

protected:
	Glib::RefPtr<Gtk::Builder> ref_builder_;

	const std::string resource_file_;

	type_signal_widget_selected m_signal_widget_selected;
	type_signal_widget_remove_clicked m_signal_widget_remove_clicked;
	type_signal_widget_visible_changed m_signal_widget_visible_changed;

private:
	Gtk::Window &parent_window_;

	GPX2VideoRenderer *renderer_;

	void on_selected(Gtk::ListBoxRow *row);

	void on_remove_clicked(GPX2VideoWidget *widget);
	void on_visible_clicked(GPX2VideoWidget *widget, Gtk::Button *button);
};

#endif

