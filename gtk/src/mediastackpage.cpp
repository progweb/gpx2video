#include <glibmm/datetime.h>
#include <glibmm/miscutils.h>
#include <glibmm/i18n.h>

#include <giomm/file.h>
#include <giomm/fileinfo.h>

#include <gtkmm/label.h>
#include <gtkmm/image.h>
#include <gtkmm/listbox.h>
#include <gtkmm/scrolledwindow.h>

#include "log_i.h"
#include "compat.h"
#include "mediastackpage.h"


GPX2VideoMediaStackPage::GPX2VideoMediaStackPage(BaseObjectType *cobject,
		const Glib::RefPtr<Gtk::Builder> &ref_builder, std::string resource_file)
	: Glib::ObjectBase("GPX2VideoMediaStackPage")
	, Gtk::Box(cobject)
	, ref_builder_(ref_builder) 
	, dispatcher_()
	, resource_file_(resource_file) {
	log_call();

	// Default media
	media_ = GPX2VideoMedia::MediaNone;

	// Connect media list
	auto list = ref_builder_->get_widget<Gtk::ListBox>("media_listbox");

	if (!list)
		throw std::runtime_error("No \"media_listbox\" object in " + resource_file_);

	list->signal_row_selected().connect(sigc::mem_fun(*this, &GPX2VideoMediaStackPage::on_selected));
}


GPX2VideoMediaStackPage * GPX2VideoMediaStackPage::create(void) {
	log_call();

	const std::string resource_file = "media_stackpage.ui";

	// Load the Builder file and instantiate its widgets.
	auto ref_builder = Gtk::Builder::create_from_resource("/com/progweb/gpx2video/ui/" + resource_file);

	auto stackpage = Gtk::Builder::get_widget_derived<GPX2VideoMediaStackPage>(ref_builder, "media_stackpage", resource_file);

	if (!stackpage)
		throw std::runtime_error("No \"media_stackpage\" object in " + resource_file);

	return stackpage;
}


/**
 * Select a media from the list
 *
 * User has clicked on a media inside the list
 *
 * Called from GTK main thread
 */
void GPX2VideoMediaStackPage::on_selected(Gtk::ListBoxRow *row) {
	GPX2VideoMedia::Media media;

	if (row == NULL)
		goto abort;

	// Get selected row
	media = (GPX2VideoMedia::Media) row->get_index();

	log_info("Media '%s' selected", GPX2VideoMedia::media2string(media).c_str());

	// Save media
	media_ = media;

	dispatcher_.emit();

	return;

abort:
	log_info("None selected media");
}

