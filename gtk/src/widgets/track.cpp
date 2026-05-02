#include "../log.h"
#include "../../src/track.h"
#include "track.h"


GPX2VideoTrackWidgetSettingsBox::GPX2VideoTrackWidgetSettingsBox(BaseObjectType *cobject,
	const Glib::RefPtr<Gtk::Builder> &ref_builder, std::string resource_file, GPX2VideoWidget *widget) 
	: GPX2VideoWidgetBaseSettingsBox(cobject, ref_builder, "GPX2VideoTrackWidgetSettingsBox", resource_file) 
	, widget_(widget) {
	log_call();

	// Populate models
	//-----------------

	// Binding
	bind_content();

	// Update ui
	update_content();
	update_boundaries();
}


void GPX2VideoTrackWidgetSettingsBox::bind_content(void) {
	log_call();

	Gtk::Switch *sw;
	Gtk::SpinButton *spinbutton;
	Gtk::ColorButton *colorbutton;

	// Connect widgets button
	//------------------------

	// Zoomfit enable
	sw = ref_builder_->get_widget<Gtk::Switch>("zoomfit_switch");
	if (!sw)
		throw std::runtime_error("No \"zoomfit_switch\" object in " + resource_file_);
	sw->signal_state_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoTrackWidgetSettingsBox::on_widget_switch_changed), sw, 
					[this](const bool &state) {
						log_info("Widget %s: zoomfit changed to '%s'",
							   widget_->name().c_str(), std::to_string(state).c_str());

						((Track *) widget_->widget())->settings().setZoomfit(state);

						// Update
						update_boundaries();

						// Broadcast widget change
						widget_->dispatchEvent(true);
					} 
			), false);

	// Factor
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("factor_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"factor_spinbutton\" object in " + resource_file_);
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoTrackWidgetSettingsBox::on_widget_spin_double_changed), spinbutton, 
					[this](const double &value) {
						log_info("Widget %s: factor changed to '%.1f'",
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
						log_info("Widget %s: path thick changed to '%.1f'",
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
						log_info("Widget %s: path thick changed to '%.1f'",
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
						log_info("Widget %s: path border color changed to '%s'", 
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
						log_info("Widget %s: path primary color changed to '%s'", 
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
						log_info("Widget %s: path secondary color changed to '%s'", 
								widget_->name().c_str(), color.c_str());

						((Track *) widget_->widget())->settings().setPathSecondaryColor(color);

						// Broadcast widget change
						widget_->dispatchEvent(true);
					}
			));
}


void GPX2VideoTrackWidgetSettingsBox::update_content(void) {
	log_call();

	Gdk::RGBA rgba;

	const float *color;

	Gtk::Switch *sw;
	Gtk::SpinButton *spinbutton;
	Gtk::ColorButton *colorbutton;

	Gtk::TreeModel::iterator iter;

	// Mask value changed
	loading_ = true;

	// zoomfit
	sw = ref_builder_->get_widget<Gtk::Switch>("zoomfit_switch");
	if (!sw)
		throw std::runtime_error("No \"zoomfit_switch\" object in " + resource_file_);

	sw->set_active(((Track *) widget_->widget())->settings().zoomfit());

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
	spinbutton->set_sensitive(((Track *) widget_->widget())->settings().zoomfit() == false);
}

