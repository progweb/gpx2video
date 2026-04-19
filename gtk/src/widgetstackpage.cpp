#include "log.h"
#include "widgetstackpage.h"


GPX2VideoWidgetStackPage::GPX2VideoWidgetStackPage(BaseObjectType *cobject,
		const Glib::RefPtr<Gtk::Builder> &ref_builder, std::string resource_file)
	: Glib::ObjectBase("GPX2VideoWidgetStackPage")
	, Gtk::ScrolledWindow(cobject)
	, ref_builder_(ref_builder) 
	, resource_file_(resource_file) {

	// Connect widget list
	auto list = ref_builder_->get_widget<Gtk::ListBox>("widgets_listbox");

	if (!list)
		throw std::runtime_error("No \"widgets_listbox\" object in window.ui");

	list->signal_row_selected().connect(sigc::mem_fun(*this, &GPX2VideoWidgetStackPage::on_widget_selected));
}


GPX2VideoWidgetStackPage * GPX2VideoWidgetStackPage::create(void) {
	log_call();

	const std::string resource_file = "widget_stackpage.ui";

	// Load the Builder file and instantiate its widgets.
	auto ref_builder = Gtk::Builder::create_from_resource("/com/progweb/gpx2video/ui/" + resource_file);

	auto stackpage = Gtk::Builder::get_widget_derived<GPX2VideoWidgetStackPage>(ref_builder, "widget_stackpage", resource_file);

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
				sigc::mem_fun(*this, &GPX2VideoWidgetStackPage::on_widget_remove_clicked), widget), true);

	box.set_hexpand(true);
	box.append(label);
	box.append(*button);

	list->append(box);
}


void GPX2VideoWidgetStackPage::remove(GPX2VideoWidget *widget) {
	log_call();

	// Search widget item index
	int index = -1;

	for (GPX2VideoWidget *item : renderer_->widgets()) {
		if (item == widget)
			break;

		index++;
	}

	if (index == -1)
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


/**
 * Select a widget from the list
 *
 * User has clicked on a widget inside the list
 *
 * Called from GTK main thread
 */
void GPX2VideoWidgetStackPage::on_widget_selected(Gtk::ListBoxRow *row) {
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
void GPX2VideoWidgetStackPage::on_widget_append_clicked(void) {
	log_call();

	log_info("Append new widget");

	log_warn("NOT YET IMPLEMENTED");
}


/**
 * Remove widget
 *
 * User removes a widget.
 *
 * Called from GTK main thread
 */
void GPX2VideoWidgetStackPage::on_widget_remove_clicked(GPX2VideoWidget *widget) {
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

