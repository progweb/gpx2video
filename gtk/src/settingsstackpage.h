#ifndef __GPX2VIDEO__GTK__SETTINGSSTACKPAGE_H__
#define __GPX2VIDEO__GTK__SETTINGSSTACKPAGE_H__

#include <glibmm/i18n.h>
#include <glibmm/dispatcher.h>

#include <gtkmm/builder.h>
#include <gtkmm/box.h>

#include "settings.h"


class GPX2VideoSettingsStackPage : public Gtk::Box {
public:
	GPX2VideoSettingsStackPage(BaseObjectType *cobject,
			const Glib::RefPtr<Gtk::Builder> &ref_builder, std::string resource_file);

	virtual ~GPX2VideoSettingsStackPage() {
	}

	static GPX2VideoSettingsStackPage * create(void);

	std::string name(void) {
		return "settings_page";
	}

	std::string title(void) {
		return _("Settings");
	}

	std::string icon_name(void) {
		return "emblem-system-symbolic";
	}

	enum GPX2VideoSettings::Section get_section(void) const {
		return settings_section_;
	}

	Glib::Dispatcher& signal_section_changed(void) {
		return dispatcher_;
	}

protected:
	Glib::RefPtr<Gtk::Builder> ref_builder_;

	Glib::Dispatcher dispatcher_;

	const std::string resource_file_;

	GPX2VideoSettings::Section settings_section_;

private:
	void on_selected(Gtk::ListBoxRow *row);
};

#endif

