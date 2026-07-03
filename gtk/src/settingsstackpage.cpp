#include <glibmm/i18n.h>

#include <gtkmm/listbox.h>
#include <gtkmm/scrolledwindow.h>

#include "log_i.h"
#include "compat.h"
#include "settingsstackpage.h"


GPX2VideoSettingsStackPage::GPX2VideoSettingsStackPage(BaseObjectType *cobject,
		const Glib::RefPtr<Gtk::Builder> &ref_builder, std::string resource_file)
	: Glib::ObjectBase("GPX2VideoSettingsStackPage")
	, Gtk::Box(cobject)
	, ref_builder_(ref_builder) 
	, dispatcher_()
	, resource_file_(resource_file) {
	log_call();

	// Default settings section
	settings_section_ = GPX2VideoSettings::SectionNone;

	// Connect settings seection list
	auto list = ref_builder_->get_widget<Gtk::ListBox>("settings_section_listbox");

	if (!list)
		throw std::runtime_error("No \"settings_section_listbox\" object in " + resource_file_);

	list->signal_row_selected().connect(sigc::mem_fun(*this, &GPX2VideoSettingsStackPage::on_selected));
}


GPX2VideoSettingsStackPage * GPX2VideoSettingsStackPage::create(void) {
	log_call();

	const std::string resource_file = "settings_stackpage.ui";

	// Load the Builder file and instantiate its widgets.
	auto ref_builder = Gtk::Builder::create_from_resource("/com/progweb/gpx2video/ui/" + resource_file);

	auto stackpage = Gtk::Builder::get_widget_derived<GPX2VideoSettingsStackPage>(ref_builder, "settings_stackpage", resource_file);

	if (!stackpage)
		throw std::runtime_error("No \"settings_stackpage\" object in " + resource_file);

	return stackpage;
}


void GPX2VideoSettingsStackPage::on_selected(Gtk::ListBoxRow *row) {
	GPX2VideoSettings::Section section;

	if (row == NULL)
		goto abort;

	// Get selected row
	section = (GPX2VideoSettings::Section) row->get_index();

	log_info("Settings section '%s' selected", GPX2VideoSettings::section2string(section).c_str());

	// Save section
	settings_section_ = section;

	dispatcher_.emit();

	return;

abort:
	log_info("None selected settings section");
}

