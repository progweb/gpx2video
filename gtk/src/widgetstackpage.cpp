#include <gtkmm/adjustment.h>

#include "log.h"
#include "append.h"
#include "widgetstackpage.h"


GPX2VideoWidgetStackPage::GPX2VideoWidgetStackPage(BaseObjectType *cobject,
		const Glib::RefPtr<Gtk::Builder> &ref_builder, std::string resource_file, Gtk::Window &parent)
	: Glib::ObjectBase("GPX2VideoWidgetStackPage")
	, Gtk::ScrolledWindow(cobject)
	, ref_builder_(ref_builder) 
	, resource_file_(resource_file) 
	, parent_window_(parent) {

	// Connect widget list
	auto list = ref_builder_->get_widget<Gtk::ListBox>("widgets_listbox");

	if (!list)
		throw std::runtime_error("No \"widgets_listbox\" object in " + resource_file_);

	list->signal_row_selected().connect(sigc::mem_fun(*this, &GPX2VideoWidgetStackPage::on_selected));
}


GPX2VideoWidgetStackPage * GPX2VideoWidgetStackPage::create(Gtk::Window &parent) {
	log_call();

	const std::string resource_file = "widget_stackpage.ui";

	// Load the Builder file and instantiate its widgets.
	auto ref_builder = Gtk::Builder::create_from_resource("/com/progweb/gpx2video/ui/" + resource_file);

	auto stackpage = Gtk::Builder::get_widget_derived<GPX2VideoWidgetStackPage>(ref_builder, "widget_stackpage", resource_file, parent);

	if (!stackpage)
		throw std::runtime_error("No \"widget_stackpage\" object in " + resource_file);

	return stackpage;
}


void GPX2VideoWidgetStackPage::set_renderer(GPX2VideoRenderer *renderer) {
	log_call();

	renderer_ = renderer;
}


void GPX2VideoWidgetStackPage::append(GPX2VideoWidget *widget) {
	log_call();

	// Widgets list
	auto list = ref_builder_->get_widget<Gtk::ListBox>("widgets_listbox");
	if (!list)
		throw std::runtime_error("No \"widgets_listbox\" object in window.ui");

	// Build & append widget item
	auto box = Gtk::Box(Gtk::Orientation::HORIZONTAL, 0);
	auto label = Gtk::Label(widget->widget()->name());
	auto button = Gtk::make_managed<Gtk::Button>(); //(new Gtk::Button());

	label.set_halign(Gtk::Align::START);
	label.set_hexpand(true);
	label.set_valign(Gtk::Align::CENTER);
	label.set_xalign(0.0);
	label.set_yalign(1.0);

	button->set_halign(Gtk::Align::END);
	button->set_valign(Gtk::Align::CENTER);
	button->set_icon_name("user-trash-symbolic");

	button->signal_clicked().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetStackPage::on_remove_clicked), widget), true);

	box.set_hexpand(true);
	box.append(label);
	box.append(*button);

	list->append(box);
}


void GPX2VideoWidgetStackPage::remove(GPX2VideoWidget *widget) {
	log_call();

	size_t index = 0;

	// Search widget item index
	for (GPX2VideoWidget *item : renderer_->widgets()) {
		if (item == widget)
			break;

		index++;
	}

	// Widget not found!
	if (index >= renderer_->widgets().size())
		return;

	// Widgets list
	auto list = ref_builder_->get_widget<Gtk::ListBox>("widgets_listbox");
	if (!list)
		throw std::runtime_error("No \"widgets_listbox\" object in window.ui");

	// Remove item
	auto row = list->get_row_at_index(index);
	list->remove(*row);
}


void GPX2VideoWidgetStackPage::purge(void) {
	log_call();

	// Widgets list
	auto list = ref_builder_->get_widget<Gtk::ListBox>("widgets_listbox");
	if (!list)
		throw std::runtime_error("No \"widgets_listbox\" object in window.ui");

	// Clear widgets list
	for (auto child = list->get_first_child(); child;) {
		auto next = child->get_next_sibling();
		list->remove(*child);
		child = next;
	}
}


void GPX2VideoWidgetStackPage::set_widget_selected(GPX2VideoWidget *widget) {
	log_call();

	size_t index = 0;

	// Widgets list
	auto list = ref_builder_->get_widget<Gtk::ListBox>("widgets_listbox");
	if (!list)
		throw std::runtime_error("No \"widgets_listbox\" object in window.ui");

	if (widget) {
		// Search widget item index
		for (GPX2VideoWidget *item : renderer_->widgets()) {
			if (item == widget)
				break;

			index++;
		}

		// Select item if found
		if (index < renderer_->widgets().size()) {
			auto adjustment = this->get_vadjustment();

			auto row = list->get_row_at_index(index);
			list->select_row(*row);

			// Scroll to row
			if (adjustment) {
				int y = row->get_allocation().get_y();
				int height = row->get_allocation().get_height();

				double value = y - (adjustment->get_page_size() - height) / 2.0;
				adjustment->set_value(std::max(0.0, value));
			}
		}
		else
			list->unselect_row();
	}
	else
		list->unselect_row();
}


/**
 * Select a widget from the list
 *
 * User has clicked on a widget inside the list
 *
 * Called from GTK main thread
 */
void GPX2VideoWidgetStackPage::on_selected(Gtk::ListBoxRow *row) {
	int i = 0;

	int index;

	if (row == NULL)
		goto abort;

	// Get selected row
	index = row->get_index();

	// Widget item
	for (GPX2VideoWidget *item : renderer_->widgets()) {
		if (i++ != index)
			continue;

		log_info("Widget '%s' selected", item->name().c_str());

		m_signal_widget_selected.emit(item);
		
		return;
	}

abort:
	log_info("None selected widget");

	m_signal_widget_selected.emit(NULL);
}


/**
 * Append new widget
 *
 * User appends a new widget.
 *
 * Called from GTK main thread
 */
void GPX2VideoWidgetStackPage::on_append_clicked(void) {
	log_call();

	log_info("Append new widget");

	try {
		auto append_dialog = GPX2VideoAppend::create(parent_window_, renderer_);

		append_dialog->present();

		// Delete the dialog when it is hidden.
		append_dialog->signal_hide().connect([append_dialog]() { 
			delete append_dialog; 
		});
	}
	catch (const Glib::Error& ex) {
		std::cerr << "GPX2VideoWidgetStackPage::on_append_clicked(): " << ex.what() << std::endl;
	}
	catch (const std::exception& ex) {
		std::cerr << "GPX2VideoWidgetStackPage::on_append_clicked(): " << ex.what() << std::endl;
	}
}


/**
 * Remove widget
 *
 * User removes a widget.
 *
 * Called from GTK main thread
 */
void GPX2VideoWidgetStackPage::on_remove_clicked(GPX2VideoWidget *widget) {
	log_call();

	log_info("Remove widget '%s'", widget->widget()->name().c_str());

	m_signal_widget_remove_clicked.emit(widget);
}


GPX2VideoWidgetStackPage::type_signal_widget_selected GPX2VideoWidgetStackPage::signal_widget_selected() {
	return m_signal_widget_selected;
}


GPX2VideoWidgetStackPage::type_signal_widget_remove_clicked GPX2VideoWidgetStackPage::signal_widget_remove_clicked() {
	return m_signal_widget_remove_clicked;
}


//const Glib::SignalProxyInfo GPX2VideoWidgetStackPage::signal_info_ = {
//	"my-signal", 
//	(GCallback) nullptr,
//	(GCallback) nullptr
//};
//
//
//void GPX2VideoWidgetStackPage::init(GPX2VideoWidgetStackPage *klass) {
//	g_signal_new(
//		"my-signal",
//		G_TYPE_FROM_CLASS(klass),
//		G_SIGNAL_RUN_FIRST,
//		0,
//		nullptr,
//		nullptr,
//		nullptr,
//		G_TYPE_NONE,
//		0 //1
//	);
//}

