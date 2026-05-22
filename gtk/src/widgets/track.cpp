#include <glibmm/i18n.h>

#include "../log_i.h"
#include "../../src/track.h"
#include "track.h"


GPX2VideoTrackWidgetSettingsBox::GPX2VideoTrackWidgetSettingsBox(BaseObjectType *cobject,
	const Glib::RefPtr<Gtk::Builder> &ref_builder, std::string resource_file, GPX2VideoWidget *widget) 
	: GPX2VideoWidgetBaseSettingsBox(cobject, ref_builder, "GPX2VideoTrackWidgetSettingsBox", resource_file, widget) { 
	log_call();

	// Populate models
	//-----------------

	view_model_ = Gtk::ListStore::create(model_);

	{
		auto iter = view_model_->append();
		auto row = *iter;
		row[model_.m_id] = TrackSettings::ViewDefault;
		row[model_.m_name] = _("Default");
		row[model_.m_enable] = true;

		row = *(view_model_->append());
		row[model_.m_id] = TrackSettings::ViewLockCenter;
		row[model_.m_name] = _("Center");
		row[model_.m_enable] = true;

		row = *(view_model_->append());
		row[model_.m_id] = TrackSettings::ViewZoomFit;
		row[model_.m_name] = _("Zoom fit");
		row[model_.m_enable] = true;
	}

	// Binding
	bind_content();

	// Update ui
	update_content();
	update_boundaries();
}


void GPX2VideoTrackWidgetSettingsBox::bind_content(void) {
	log_call();

	Gtk::Switch *sw;
	Gtk::ComboBox *combobox;
	Gtk::SpinButton *spinbutton;
	Gtk::ColorButton *colorbutton;

	auto renderer = Gtk::make_managed<Gtk::CellRendererText>();

	// Connect widgets button
	//------------------------

	// View
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("view_combobox");
	if (!combobox)
		throw std::runtime_error("No \"view_combobox\" object in" + resource_file_);

	combobox->pack_start(*renderer, true);
	combobox->add_attribute(renderer->property_text(), model_.m_name);
	combobox->add_attribute(renderer->property_sensitive(), model_.m_enable);

	combobox->signal_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoTrackWidgetSettingsBox::on_widget_combobox_changed), combobox, 
					[this](const Gtk::TreeModel::const_iterator &iter) {
						int value = iter->get_value(model_.m_id);

						log_notice("Widget %s: view changed to '%s'", 
								widget_->widget()->name().c_str(), iter->get_value(model_.m_name).c_str());

						((Track *) widget_->widget())->settings().setView((TrackSettings::View) value);

						// Update
						update_boundaries();

						// Broadcast widget change
						widget_->dispatchEvent(true);
					}
			));

	// Factor
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("factor_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"factor_spinbutton\" object in " + resource_file_);
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoTrackWidgetSettingsBox::on_widget_spin_double_changed), spinbutton, 
					[this](const double &value) {
						log_notice("Widget %s: factor changed to '%.1f'",
							   widget_->name().c_str(), value);

						((Track *) widget_->widget())->settings().setDivider(value);

						// Broadcast widget change
						widget_->dispatchEvent(true);
					}
			));

	// Path thick
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("path_thick_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"path_thick_spinbutton\" object in " + resource_file_);

	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoTrackWidgetSettingsBox::on_widget_spin_double_changed), spinbutton, 
					[this](const double &value) {
						log_notice("Widget %s: path thick changed to '%.1f'",
							   widget_->name().c_str(), value);

						((Track *) widget_->widget())->settings().setPathThick(value);

						// Broadcast widget change
						widget_->dispatchEvent(true);
					}
			));

	// Path border
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("path_border_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"path_border_spinbutton\" object in " + resource_file_);

	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoTrackWidgetSettingsBox::on_widget_spin_double_changed), spinbutton, 
					[this](const double &value) {
						log_notice("Widget %s: path thick changed to '%.1f'",
							   widget_->name().c_str(), value);

						((Track *) widget_->widget())->settings().setPathBorder(value);

						// Broadcast widget change
						widget_->dispatchEvent(true);
					}
			));

	// Path border color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("path_border_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"path_border_color_button\" object in " + resource_file_);

	colorbutton->signal_color_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoTrackWidgetSettingsBox::on_widget_color_changed), colorbutton, 
					[this](const std::string &color) {
						log_notice("Widget %s: path border color changed to '%s'", 
								widget_->name().c_str(), color.c_str());

						((Track *) widget_->widget())->settings().setPathBorderColor(color);

						// Broadcast widget change
						widget_->dispatchEvent(true);
					}
			));

	// Path primary color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("path_primary_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"path_primary_color_button\" object in " + resource_file_);

	colorbutton->signal_color_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoTrackWidgetSettingsBox::on_widget_color_changed), colorbutton, 
					[this](const std::string &color) {
						log_notice("Widget %s: path primary color changed to '%s'", 
								widget_->name().c_str(), color.c_str());

						((Track *) widget_->widget())->settings().setPathPrimaryColor(color);

						// Broadcast widget change
						widget_->dispatchEvent(true);
					}
			));

	// Path secondary color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("path_secondary_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"path_secondary_color_button\" object in " + resource_file_);

	colorbutton->signal_color_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoTrackWidgetSettingsBox::on_widget_color_changed), colorbutton, 
					[this](const std::string &color) {
						log_notice("Widget %s: path secondary color changed to '%s'", 
								widget_->name().c_str(), color.c_str());

						((Track *) widget_->widget())->settings().setPathSecondaryColor(color);

						// Broadcast widget change
						widget_->dispatchEvent(true);
					}
			));

	// Marker enable
	sw = ref_builder_->get_widget<Gtk::Switch>("marker_enable_switch");
	if (!sw)
		throw std::runtime_error("No \"marker_enable_switch\" object in " + resource_file_);
	sw->signal_state_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoTrackWidgetSettingsBox::on_widget_switch_changed), sw, 
					[this](const bool &state) {
						log_notice("Widget %s: marker status changed to '%s'",
							   widget_->widget()->name().c_str(), state ? "enabled" : "disabled");

						if (state)
							widget_->widget()->theme().addFlag(VideoWidget::Theme::FlagIcon);
						else
							widget_->widget()->theme().removeFlag(VideoWidget::Theme::FlagIcon);

						// Broadcast widget change
						widget_->dispatchEvent(false);
					} 
			), false);

	// Marker size
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("marker_size_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"factor_spinbutton\" object in " + resource_file_);
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoTrackWidgetSettingsBox::on_widget_spin_double_changed), spinbutton, 
					[this](const double &value) {
						log_notice("Widget %s: factor changed to '%.1f'",
							   widget_->name().c_str(), value);

						((Track *) widget_->widget())->settings().setMarkerSize(value);

						// Broadcast widget change
						widget_->dispatchEvent(false);
					}
			));
}


void GPX2VideoTrackWidgetSettingsBox::update_content(void) {
	log_call();

	Gdk::RGBA rgba;

	const float *color;

	Gtk::Switch *sw;
	Gtk::ComboBox *combobox;
	Gtk::SpinButton *spinbutton;
	Gtk::ColorButton *colorbutton;

	Gtk::TreeModel::iterator iter;

	// Mask value changed
	loading_ = true;

	// view
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("view_combobox");
	if (!combobox)
		throw std::runtime_error("No \"view_combobox\" object in " + resource_file_);

	combobox->set_model(view_model_);

	if (find_in_listtore(view_model_, ((Track *) widget_->widget())->settings().view(), iter))
		combobox->set_active(iter);

	// factor
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("factor_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"factor_spinbutton\" object in " + resource_file_);

	spinbutton->set_value(((Track *) widget_->widget())->settings().divider());

	// Path thick
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("path_thick_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"path_thick_spinbutton\" object in " + resource_file_);

	spinbutton->set_value(((Track *) widget_->widget())->settings().pathThick());

	// Path border
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("path_border_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"path_border_spinbutton\" object in " + resource_file_);

	spinbutton->set_value(((Track *) widget_->widget())->settings().pathBorder());

	// Path border color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("path_border_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"path_border_color_button\" object in " + resource_file_);

	color = ((Track *) widget_->widget())->settings().pathBorderColor();

	rgba.set_rgba(color[0], color[1], color[2], color[3]);

	colorbutton->set_rgba(rgba);

	// Path primary color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("path_primary_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"path_primary_color_button\" object in " + resource_file_);

	color = ((Track *) widget_->widget())->settings().pathPrimaryColor();

	rgba.set_rgba(color[0], color[1], color[2], color[3]);

	colorbutton->set_rgba(rgba);

	// Path secondary color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("path_secondary_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"path_secondary_color_button\" object in " + resource_file_);

	color = ((Track *) widget_->widget())->settings().pathSecondaryColor();

	rgba.set_rgba(color[0], color[1], color[2], color[3]);

	colorbutton->set_rgba(rgba);

	// Marker enable
	sw = ref_builder_->get_widget<Gtk::Switch>("marker_enable_switch");
	if (!sw)
		throw std::runtime_error("No \"marker_enable_switch\" object in " + resource_file_);

	sw->set_active(widget_->widget()->theme().hasFlag(VideoWidget::Theme::FlagIcon));

	// Marker size
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("marker_size_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"factor_spinbutton\" object in " + resource_file_);

	spinbutton->set_value(((Track *) widget_->widget())->settings().markerSize());

	// Unmask value changed
	loading_ = false;
}


void GPX2VideoTrackWidgetSettingsBox::update_boundaries(void) {
	log_call();

	Gtk::SpinButton *spinbutton;

	// Factor
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("factor_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"factor_spinbutton\" object in " + resource_file_);

	spinbutton->set_value(((Track *) widget_->widget())->settings().divider());
	spinbutton->set_sensitive(((Track *) widget_->widget())->settings().view() != TrackSettings::ViewZoomFit);
}

