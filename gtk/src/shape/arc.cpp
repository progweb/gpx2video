#include "../log.h"
#include "arc.h"


GPX2VideoArcSettingsBox::GPX2VideoArcSettingsBox(BaseObjectType *cobject,
	const Glib::RefPtr<Gtk::Builder> &ref_builder, std::string resource_file, GPX2VideoWidget *widget) 
	: GPX2VideoBaseSettingsBox(cobject, ref_builder, "GPX2VideoArcSettingsBox", resource_file) 
	, widget_(widget) {
	log_call();

	Gtk::Switch *sw;
//		Gtk::ComboBox *combobox;
	Gtk::SpinButton *spinbutton;
	Gtk::ColorButton *colorbutton;

	loading_ = false;

	// Tick enable
	sw = ref_builder_->get_widget<Gtk::Switch>("tick_enable_switch");
	if (!sw)
		throw std::runtime_error("No \"tick_enable_switch\" object in " + resource_file_);
	sw->signal_state_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoArcSettingsBox::on_widget_switch_changed), sw, 
					[this](const bool &state) {
						if (state)
							widget_->theme().addFlag(VideoWidget::Theme::FlagTick);
						else
							widget_->theme().removeFlag(VideoWidget::Theme::FlagTick);

						// Broadcast widget change
						widget_->dispatchEvent();
					} 
			), false);

	// Tick color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("tick_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"tick_color_button\" object in " + resource_file_);
	colorbutton->signal_color_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoArcSettingsBox::on_widget_color_changed), colorbutton, 
					[this](const std::string &color) {
						log_info("Widget '%s' tick color changed to '%s'", 
								widget_->name().c_str(), color.c_str());

						widget_->theme().setTickColor(color);

						// Broadcast widget change
						widget_->dispatchEvent();
					}
			));

	// Tick label enable
	sw = ref_builder_->get_widget<Gtk::Switch>("tick_label_enable_switch");
	if (!sw)
		throw std::runtime_error("No \"tick_label_enable_switch\" object in " + resource_file_);
	sw->signal_state_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoArcSettingsBox::on_widget_switch_changed), sw, 
					[this](const bool &state) {
						if (state)
							widget_->theme().addFlag(VideoWidget::Theme::FlagTickLabel);
						else
							widget_->theme().removeFlag(VideoWidget::Theme::FlagTickLabel);

						// Broadcast widget change
						widget_->dispatchEvent();
					} 
			), false);

	// Tick label color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("tick_label_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"tick_label_color_button\" object in " + resource_file_);
	colorbutton->signal_color_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoArcSettingsBox::on_widget_color_changed), colorbutton, 
					[this](const std::string &color) {
						log_info("Widget '%s' tick label color changed to '%s'", 
								widget_->name().c_str(), color.c_str());

						widget_->theme().setTickLabelColor(color);

						// Broadcast widget change
						widget_->dispatchEvent();
					}
			));

//		// Tick alignment
//		combobox = ref_builder_->get_widget<Gtk::ComboBox>("tick_align_combobox");
//		if (!combobox)
//			throw std::runtime_error("No \"tick_align_combobox\" object in " + resource_file_);
//		combobox->pack_start(model_.m_name);
//		combobox->signal_changed().connect(sigc::mem_fun(*this, &GPX2VideoElevationWidget::on_widget_tick_align_value_changed));

	// Needle enable
	sw = ref_builder_->get_widget<Gtk::Switch>("needle_enable_switch");
	if (!sw)
		throw std::runtime_error("No \"needle_enable_switch\" object in " + resource_file_);
	sw->signal_state_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoArcSettingsBox::on_widget_switch_changed), sw, 
					[this](const bool &state) {
						if (state)
							widget_->theme().addFlag(VideoWidget::Theme::FlagNeedle);
						else
							widget_->theme().removeFlag(VideoWidget::Theme::FlagNeedle);

						// Broadcast widget change
						widget_->dispatchEvent();
					} 
			), false);

	// Needle color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("needle_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"needle_color_button\" object in " + resource_file_);
	colorbutton->signal_color_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoArcSettingsBox::on_widget_color_changed), colorbutton, 
					[this](const std::string &color) {
						log_info("Widget '%s' neelde color changed to '%s'", 
								widget_->name().c_str(), color.c_str());

						widget_->theme().setNeedleColor(color);

						// Broadcast widget change
						widget_->dispatchEvent();
					}
			));

	// Gauge enable
	sw = ref_builder_->get_widget<Gtk::Switch>("gauge_enable_switch");
	if (!sw)
		throw std::runtime_error("No \"gauge_enable_switch\" object in " + resource_file_);
	sw->signal_state_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoArcSettingsBox::on_widget_switch_changed), sw, 
					[this](const bool &state) {
						if (state)
							widget_->theme().addFlag(VideoWidget::Theme::FlagGauge);
						else
							widget_->theme().removeFlag(VideoWidget::Theme::FlagGauge);

						// Broadcast widget change
						widget_->dispatchEvent();
					} 
			), false);

	// Gauge color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("gauge_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"gauge_color_button\" object in " + resource_file_);
	colorbutton->signal_color_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoArcSettingsBox::on_widget_color_changed), colorbutton, 
					[this](const std::string &color) {
						log_info("Widget '%s' gauge color changed to '%s'", 
								widget_->name().c_str(), color.c_str());

						widget_->theme().setGaugeColor(0, color);

						// Broadcast widget change
						widget_->dispatchEvent();
					}
			));

	// Gauge border width
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("gauge_border_width_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"gauge_border_width_spinbutton\" object in " + resource_file_);
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoArcSettingsBox::on_widget_spin_changed), spinbutton, 
					[this](const int &value) {
						log_info("Widget %s: gauge border changed to '%d'",
							   widget_->name().c_str(), value);

						widget_->theme().setGaugeBorder(value);

						// Broadcast widget change
						widget_->dispatchEvent();
					}
			));

	// Gauge border color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("gauge_border_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"gauge_border_color_button\" object in " + resource_file_);
	colorbutton->signal_color_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoArcSettingsBox::on_widget_color_changed), colorbutton, 
					[this](const std::string &color) {
						log_info("Widget '%s' gauge border color changed to '%s'", 
								widget_->name().c_str(), color.c_str());

						widget_->theme().setGaugeBorderColor(color);

						// Broadcast widget change
						widget_->dispatchEvent();
					}
			));

	// Gauge background color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("gauge_background_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"gauge_background_color_button\" object in " + resource_file_);
	colorbutton->signal_color_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoArcSettingsBox::on_widget_color_changed), colorbutton, 
					[this](const std::string &color) {
						log_info("Widget '%s' gauge background color changed to '%s'", 
								widget_->name().c_str(), color.c_str());

						widget_->theme().setGaugeBackgroundColor(color);

						// Broadcast widget change
						widget_->dispatchEvent();
					}
			));
}


void GPX2VideoArcSettingsBox::update_content(void) {
	log_call();

	Gdk::RGBA rgba;

	const float *color;

	Gtk::Switch *sw;
//		Gtk::ComboBox *combobox;
	Gtk::SpinButton *spinbutton;
	Gtk::ColorButton *colorbutton;

//		Gtk::TreeModel::iterator iter;

	// Mask value changed
	loading_ = true;

	// Widget tick enable switch
	sw = ref_builder_->get_widget<Gtk::Switch>("tick_enable_switch");
	if (!sw)
		throw std::runtime_error("No \"tick_enable_switch\" object in " + resource_file_);

	sw->set_active(widget_->theme().hasFlag(VideoWidget::Theme::FlagTick));

	// Widget tick color button
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("tick_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"tick_color_button\" object in " + resource_file_);

	color = widget_->theme().tickColor();

	rgba.set_rgba(color[0], color[1], color[2], color[3]);

	colorbutton->set_rgba(rgba);

	// Widget tick label enable switch
	sw = ref_builder_->get_widget<Gtk::Switch>("tick_label_enable_switch");
	if (!sw)
		throw std::runtime_error("No \"tick_label_enable_switch\" object in " + resource_file_);

	sw->set_active(widget_->theme().hasFlag(VideoWidget::Theme::FlagTickLabel));

	// Widget tick label color button
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("tick_label_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"tick_label_color_button\" object in " + resource_file_);

	color = widget_->theme().tickLabelColor();

	rgba.set_rgba(color[0], color[1], color[2], color[3]);

	colorbutton->set_rgba(rgba);

//		// Widget tick alignment
//		combobox = ref_builder_->get_widget<Gtk::ComboBox>("tick_align_combobox");
//		if (!combobox)
//			throw std::runtime_error("No \"tick_align_combobox\" object in " + resource_file_);
//
//		combobox->set_model(tick_align_model_);
//
//		if (find_in_listtore(tick_align_model_, widget_->theme().tickAlign(), iter))
//			combobox->set_active(iter);

	// Widget needle enable switch
	sw = ref_builder_->get_widget<Gtk::Switch>("needle_enable_switch");
	if (!sw)
		throw std::runtime_error("No \"needle_enable_switch\" object in " + resource_file_);

	sw->set_active(widget_->theme().hasFlag(VideoWidget::Theme::FlagNeedle));

	// Widget needle color button
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("needle_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"needle_color_button\" object in " + resource_file_);

	color = widget_->theme().needleColor();

	rgba.set_rgba(color[0], color[1], color[2], color[3]);

	colorbutton->set_rgba(rgba);

	// Widget gauge enable switch
	sw = ref_builder_->get_widget<Gtk::Switch>("gauge_enable_switch");
	if (!sw)
		throw std::runtime_error("No \"gauge_enable_switch\" object in " + resource_file_);

	sw->set_active(widget_->theme().hasFlag(VideoWidget::Theme::FlagGauge));

	// Widget gauge color button
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("gauge_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"gauge_color_button\" object in " + resource_file_);

	color = widget_->theme().gaugeColor(0);

	rgba.set_rgba(color[0], color[1], color[2], color[3]);

	colorbutton->set_rgba(rgba);

	// Widget gauge border width button
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("gauge_border_width_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"gauge_border_width_spinbutton\" object in " + resource_file_);

	spinbutton->set_value(widget_->theme().gaugeBorder());

	// Widget gauge border color button
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("gauge_border_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"gauge_border_color_button\" object in " + resource_file_);

	color = widget_->theme().gaugeBorderColor();

	rgba.set_rgba(color[0], color[1], color[2], color[3]);

	colorbutton->set_rgba(rgba);

	// Widget gauge background color button
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("gauge_background_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"gauge_background_color_button\" object in " + resource_file_);

	color = widget_->theme().gaugeBackgroundColor();

	rgba.set_rgba(color[0], color[1], color[2], color[3]);

	colorbutton->set_rgba(rgba);

	// Unmask value changed
	loading_ = false;
}

