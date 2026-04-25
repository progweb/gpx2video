#include "../log.h"
#include "../../src/map.h"
#include "map.h"


GPX2VideoMapWidgetSettingsBox::GPX2VideoMapWidgetSettingsBox(BaseObjectType *cobject,
	const Glib::RefPtr<Gtk::Builder> &ref_builder, std::string resource_file, GPX2VideoWidget *widget) 
	: GPX2VideoWidgetBaseSettingsBox(cobject, ref_builder, "GPX2VideoMapWidgetSettingsBox", resource_file) 
	, widget_(widget) {
	log_call();

	// Populate models
	//-----------------

	source_model_ = Gtk::ListStore::create(model_);

	for (int i=MapSettings::SourceNull; i != MapSettings::SourceCount; i++) {
		std::string name = MapSettings::getFriendlyName((MapSettings::Source) i);
		std::string uri = MapSettings::getRepoURI((MapSettings::Source) i);

		if (i == MapSettings::SourceNull)
			continue;

		if (uri == "")
			continue;

		auto row = *(source_model_->append());
		row[model_.m_id] = i;
		row[model_.m_name] = name;
		row[model_.m_enable] = true;
	}

	// Binding
	bind_content();

	// Update ui
	update_content();
	update_boundaries();
}


void GPX2VideoMapWidgetSettingsBox::bind_content(void) {
	log_call();

	Gtk::ComboBox *combobox;
	Gtk::SpinButton *spinbutton;
	Gtk::ColorButton *colorbutton;

	auto renderer = Gtk::make_managed<Gtk::CellRendererText>();

	// Connect widgets button
	//------------------------

	// Source
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("source_combobox");
	if (!combobox)
		throw std::runtime_error("No \"source_combobox\" object in" + resource_file_);

	combobox->pack_start(*renderer, true);
	combobox->add_attribute(renderer->property_text(), model_.m_name);
	combobox->add_attribute(renderer->property_sensitive(), model_.m_enable);

	combobox->signal_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoMapWidgetSettingsBox::on_widget_combobox_changed), combobox, 
					[this](const Gtk::TreeModel::const_iterator &iter) {
						int value = iter->get_value(model_.m_id);

						log_info("Widget %s: source changed to '%s'", 
								widget_->widget()->name().c_str(), iter->get_value(model_.m_name).c_str());

						((Map *) widget_->widget())->settings().setSource((MapSettings::Source) value);

						// Update
						update_boundaries();

						// Broadcast widget change
						widget_->dispatchEvent(true);
					}
			));
	// Zoom
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("zoom_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"zoom_spinbutton\" object in " + resource_file_);
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoMapWidgetSettingsBox::on_widget_spin_int_changed), spinbutton, 
					[this](const int &value) {
						log_info("Widget %s: zoom changed to '%d'",
							   widget_->name().c_str(), value);

						((Map *) widget_->widget())->settings().setZoom(value);

						// Broadcast widget change
						widget_->dispatchEvent(true);
					}
			));

	// Factor
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("factor_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"factor_spinbutton\" object in " + resource_file_);
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoMapWidgetSettingsBox::on_widget_spin_double_changed), spinbutton, 
					[this](const double &value) {
						log_info("Widget %s: factor changed to '%.1f'",
							   widget_->name().c_str(), value);

						((Map *) widget_->widget())->settings().setDivider(value);

						// Broadcast widget change
						widget_->dispatchEvent(true);
					}
			));

	// Path thick
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("path_thick_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"path_thick_spinbutton\" object in " + resource_file_);

	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoMapWidgetSettingsBox::on_widget_spin_double_changed), spinbutton, 
					[this](const double &value) {
						log_info("Widget %s: path thick changed to '%.1f'",
							   widget_->name().c_str(), value);

						((Map *) widget_->widget())->settings().setPathThick(value);

						// Broadcast widget change
						widget_->dispatchEvent(true);
					}
			));

	// Path border
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("path_border_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"path_border_spinbutton\" object in " + resource_file_);

	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoMapWidgetSettingsBox::on_widget_spin_double_changed), spinbutton, 
					[this](const double &value) {
						log_info("Widget %s: path thick changed to '%.1f'",
							   widget_->name().c_str(), value);

						((Map *) widget_->widget())->settings().setPathBorder(value);

						// Broadcast widget change
						widget_->dispatchEvent(true);
					}
			));

	// Path border color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("path_border_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"path_border_color_button\" object in " + resource_file_);

	colorbutton->signal_color_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoMapWidgetSettingsBox::on_widget_color_changed), colorbutton, 
					[this](const std::string &color) {
						log_info("Widget %s: path border color changed to '%s'", 
								widget_->name().c_str(), color.c_str());

						((Map *) widget_->widget())->settings().setPathBorderColor(color);

						// Broadcast widget change
						widget_->dispatchEvent(true);
					}
			));

	// Path primary color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("path_primary_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"path_primary_color_button\" object in " + resource_file_);

	colorbutton->signal_color_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoMapWidgetSettingsBox::on_widget_color_changed), colorbutton, 
					[this](const std::string &color) {
						log_info("Widget %s: path primary color changed to '%s'", 
								widget_->name().c_str(), color.c_str());

						((Map *) widget_->widget())->settings().setPathPrimaryColor(color);

						// Broadcast widget change
						widget_->dispatchEvent(true);
					}
			));

	// Path secondary color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("path_secondary_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"path_secondary_color_button\" object in " + resource_file_);

	colorbutton->signal_color_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoMapWidgetSettingsBox::on_widget_color_changed), colorbutton, 
					[this](const std::string &color) {
						log_info("Widget %s: path secondary color changed to '%s'", 
								widget_->name().c_str(), color.c_str());

						((Map *) widget_->widget())->settings().setPathSecondaryColor(color);

						// Broadcast widget change
						widget_->dispatchEvent(true);
					}
			));
}


void GPX2VideoMapWidgetSettingsBox::update_content(void) {
	log_call();

	Gdk::RGBA rgba;

	const float *color;

	Gtk::ComboBox *combobox;
	Gtk::SpinButton *spinbutton;
	Gtk::ColorButton *colorbutton;

	Gtk::TreeModel::iterator iter;

	// Mask value changed
	loading_ = true;

	// source
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("source_combobox");
	if (!combobox)
		throw std::runtime_error("No \"source_combobox\" object in " + resource_file_);

	combobox->set_model(source_model_);

	if (find_in_listtore(source_model_, ((Map *) widget_->widget())->settings().source(), iter))
		combobox->set_active(iter);

	// zoom
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("zoom_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"zoom_spinbutton\" object in " + resource_file_);

	spinbutton->set_value(((Map *) widget_->widget())->settings().zoom());

	// factor
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("factor_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"factor_spinbutton\" object in " + resource_file_);

	spinbutton->set_value(((Map *) widget_->widget())->settings().divider());

	// Path thick
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("path_thick_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"path_thick_spinbutton\" object in " + resource_file_);

	spinbutton->set_value(((Map *) widget_->widget())->settings().pathThick());

	// Path border
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("path_border_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"path_border_spinbutton\" object in " + resource_file_);

	spinbutton->set_value(((Map *) widget_->widget())->settings().pathBorder());

	// Path border color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("path_border_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"path_border_color_button\" object in " + resource_file_);

	color = ((Map *) widget_->widget())->settings().pathBorderColor();

	rgba.set_rgba(color[0], color[1], color[2], color[3]);

	colorbutton->set_rgba(rgba);

	// Path primary color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("path_primary_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"path_primary_color_button\" object in " + resource_file_);

	color = ((Map *) widget_->widget())->settings().pathPrimaryColor();

	rgba.set_rgba(color[0], color[1], color[2], color[3]);

	colorbutton->set_rgba(rgba);

	// Path secondary color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("path_secondary_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"path_secondary_color_button\" object in " + resource_file_);

	color = ((Map *) widget_->widget())->settings().pathSecondaryColor();

	rgba.set_rgba(color[0], color[1], color[2], color[3]);

	colorbutton->set_rgba(rgba);

	// Unmask value changed

	loading_ = false;
}


void GPX2VideoMapWidgetSettingsBox::update_boundaries(void) {
	log_call();

	Gtk::SpinButton *spinbutton;

	MapSettings::Source source = ((Map *) widget_->widget())->settings().source();

	// Update UI range for source
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("zoom_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"zoom_spinbutton\" object in " + resource_file_);

	spinbutton->set_range(
			MapSettings::getMinZoom(source),
			MapSettings::getMaxZoom(source));
}

