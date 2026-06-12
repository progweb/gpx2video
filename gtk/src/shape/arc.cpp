#include <glibmm/i18n.h>

#include "../log_i.h"
#include "arc.h"


GPX2VideoArcShapeSettingsBox::GPX2VideoArcShapeSettingsBox(BaseObjectType *cobject,
	const Glib::RefPtr<Gtk::Builder> &ref_builder, std::string resource_file, GPX2VideoWidget *widget) 
	: GPX2VideoShapeBaseSettingsBox(cobject, ref_builder, "GPX2VideoArcShapeSettingsBox", resource_file, widget) {
	log_call();

	loading_ = false;

	// Populate models
	load_models();

	// Binding
	bind_content();
}


void GPX2VideoArcShapeSettingsBox::load_models(void) {
	log_call();

	gauge_cap_model_ = Gtk::ListStore::create(model_);

	{
		auto iter = gauge_cap_model_->append();
		auto row = *iter;
		row[model_.m_id] = VideoWidget::Theme::GaugeCapSquare;
		row[model_.m_name] = _("Square");

		row = *(gauge_cap_model_->append());
		row[model_.m_id] = VideoWidget::Theme::GaugeCapRound;
		row[model_.m_name] = _("Round");
	}

	needle_type_model_ = Gtk::ListStore::create(model_);

	{
		auto iter = needle_type_model_->append();
		auto row = *iter;
		row[model_.m_id] = VideoWidget::Theme::NeedleTypeThin;
		row[model_.m_name] = _("Thin");

		row = *(needle_type_model_->append());
		row[model_.m_id] = VideoWidget::Theme::NeedleTypeLight;
		row[model_.m_name] = _("Light");

		row = *(needle_type_model_->append());
		row[model_.m_id] = VideoWidget::Theme::NeedleTypeBasic;
		row[model_.m_name] = _("Basic");

		row = *(needle_type_model_->append());
		row[model_.m_id] = VideoWidget::Theme::NeedleTypeDesign;
		row[model_.m_name] = _("Design");
	}
}


void GPX2VideoArcShapeSettingsBox::bind_content(void) {
	log_call();

	Gtk::Switch *sw;
	Gtk::ComboBox *combobox;
	Gtk::SpinButton *spinbutton;
	Gtk::ColorButton *colorbutton;

	// Gauge enable
	sw = ref_builder_->get_widget<Gtk::Switch>("gauge_enable_switch");
	if (!sw)
		throw std::runtime_error("No \"gauge_enable_switch\" object in " + resource_file_);
	sw->signal_state_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoArcShapeSettingsBox::on_widget_switch_changed), sw, 
					[this](const bool &state) {
						log_notice("Widget %s: gauge status changed to '%s'",
							   widget_->name().c_str(), state ? "enabled" : "disabled");

						if (state)
							widget_->theme().addFlag(VideoWidget::Theme::FlagGauge);
						else
							widget_->theme().removeFlag(VideoWidget::Theme::FlagGauge);

						// Broadcast widget change
						widget_->dispatchEvent();
					} 
			), false);

	// Gauge angle
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("gauge_angle_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"gauge_angle_spinbutton\" object in " + resource_file_);
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoArcShapeSettingsBox::on_widget_spin_changed), spinbutton, 
					[this](const int &value) {
						log_notice("Widget %s: gauge angle changed to '%d'",
							   widget_->name().c_str(), value);

						widget_->theme().setGaugeAngle(value);

						// Broadcast widget change
						widget_->dispatchEvent();
					}
			));

	// Gauge rotation
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("gauge_rotation_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"gauge_rotation_spinbutton\" object in " + resource_file_);
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoArcShapeSettingsBox::on_widget_spin_changed), spinbutton, 
					[this](const int &value) {
						log_notice("Widget %s: gauge rotation changed to '%d'",
							   widget_->name().c_str(), value);

						widget_->theme().setGaugeRotation(value);

						// Broadcast widget change
						widget_->dispatchEvent();
					}
			));

	// Gauge flip
	sw = ref_builder_->get_widget<Gtk::Switch>("gauge_flip_switch");
	if (!sw)
		throw std::runtime_error("No \"gauge_flip_switch\" object in " + resource_file_);
	sw->signal_state_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoArcShapeSettingsBox::on_widget_switch_changed), sw, 
					[this](const bool &state) {
						log_notice("Widget %s: gauge flip changed to '%s'",
							   widget_->name().c_str(), state ? "enabled" : "disabled");

						widget_->theme().setGaugeFlip(state);

						// Broadcast widget change
						widget_->dispatchEvent();
					} 
			), false);

	// Gauge width
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("gauge_width_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"gauge_width_spinbutton\" object in " + resource_file_);
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoArcShapeSettingsBox::on_widget_spin_changed), spinbutton, 
					[this](const int &value) {
						log_notice("Widget %s: gauge width changed to '%d'",
							   widget_->name().c_str(), value);

						widget_->theme().setGaugeWidth(value);

						// Broadcast widget change
						widget_->dispatchEvent();
					}
			));

	// Gauge cap
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("gauge_cap_combobox");
	if (!combobox)
		throw std::runtime_error("No \"gauge_cap_combobox\" object in " + resource_file_);
	combobox->pack_start(model_.m_name);
	combobox->signal_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoArcShapeSettingsBox::on_widget_combobox_changed), combobox, 
					[this](const Gtk::TreeModel::const_iterator &iter) {
						int value = iter->get_value(model_.m_id);

						log_notice("Widget %s: gauge cap changed to '%s'", 
								widget_->name().c_str(), iter->get_value(model_.m_name).c_str());

						widget_->theme().setGaugeCap((VideoWidget::Theme::GaugeCap) value);

						// Broadcast widget change
						widget_->dispatchEvent();
					}
			));

	// Gauge primary color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("gauge_primary_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"gauge_primary_color_button\" object in " + resource_file_);
	colorbutton->signal_color_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoArcShapeSettingsBox::on_widget_color_changed), colorbutton, 
					[this](const std::string &color) {
						log_notice("Widget %s: gauge primary color changed to '%s'", 
								widget_->name().c_str(), color.c_str());

						widget_->theme().setGaugePrimaryColor(color);

						// Broadcast widget change
						widget_->dispatchEvent();
					}
			));

	// Gauge secondary color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("gauge_secondary_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"gauge_secondary_color_button\" object in " + resource_file_);
	colorbutton->signal_color_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoArcShapeSettingsBox::on_widget_color_changed), colorbutton, 
					[this](const std::string &color) {
						log_notice("Widget %s: gauge secondary color changed to '%s'", 
								widget_->name().c_str(), color.c_str());

						widget_->theme().setGaugeSecondaryColor(color);

						// Broadcast widget change
						widget_->dispatchEvent();
					}
			));

	// Gauge border width
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("gauge_border_width_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"gauge_border_width_spinbutton\" object in " + resource_file_);
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoArcShapeSettingsBox::on_widget_spin_changed), spinbutton, 
					[this](const int &value) {
						log_notice("Widget %s: gauge border changed to '%d'",
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
				sigc::mem_fun(*this, &GPX2VideoArcShapeSettingsBox::on_widget_color_changed), colorbutton, 
					[this](const std::string &color) {
						log_notice("Widget %s: gauge border color changed to '%s'", 
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
				sigc::mem_fun(*this, &GPX2VideoArcShapeSettingsBox::on_widget_color_changed), colorbutton, 
					[this](const std::string &color) {
						log_notice("Widget %s: gauge background color changed to '%s'", 
								widget_->name().c_str(), color.c_str());

						widget_->theme().setGaugeBackgroundColor(color);

						// Broadcast widget change
						widget_->dispatchEvent();
					}
			));

	// Tick enable
	sw = ref_builder_->get_widget<Gtk::Switch>("tick_enable_switch");
	if (!sw)
		throw std::runtime_error("No \"tick_enable_switch\" object in " + resource_file_);
	sw->signal_state_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoArcShapeSettingsBox::on_widget_switch_changed), sw, 
					[this](const bool &state) {
						log_notice("Widget %s: tick status changed to '%s'",
							   widget_->name().c_str(), state ? "enabled" : "disabled");

						if (state)
							widget_->theme().addFlag(VideoWidget::Theme::FlagTick);
						else
							widget_->theme().removeFlag(VideoWidget::Theme::FlagTick);

						// Broadcast widget change
						widget_->dispatchEvent();
					} 
			), false);

	// Tick size
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("tick_size_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"tick_size_spinbutton\" object in " + resource_file_);
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoArcShapeSettingsBox::on_widget_spin_changed), spinbutton, 
					[this](const int &value) {
						log_notice("Widget %s: tick size changed to '%d'",
							   widget_->name().c_str(), value);

						widget_->theme().setTickSize(value);

						// Broadcast widget change
						widget_->dispatchEvent();
					}
			));

	// Tick color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("tick_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"tick_color_button\" object in " + resource_file_);
	colorbutton->signal_color_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoArcShapeSettingsBox::on_widget_color_changed), colorbutton, 
					[this](const std::string &color) {
						log_notice("Widget %s: tick color changed to '%s'", 
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
				sigc::mem_fun(*this, &GPX2VideoArcShapeSettingsBox::on_widget_switch_changed), sw, 
					[this](const bool &state) {
						log_notice("Widget %s: tick label status changed to '%s'",
							   widget_->name().c_str(), state ? "enabled" : "disabled");

						if (state)
							widget_->theme().addFlag(VideoWidget::Theme::FlagTickLabel);
						else
							widget_->theme().removeFlag(VideoWidget::Theme::FlagTickLabel);

						// Broadcast widget change
						widget_->dispatchEvent();
					} 
			), false);

	// Tick label distance
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("tick_label_distance_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"tick_label_distance_spinbutton\" object in " + resource_file_);
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoArcShapeSettingsBox::on_widget_spin_changed), spinbutton, 
					[this](const int &value) {
						log_notice("Widget %s: tick label distance changed to '%d'",
							   widget_->name().c_str(), value);

						widget_->theme().setTickLabelDistance(value);

						// Broadcast widget change
						widget_->dispatchEvent();
					}
			));

	// Tick label font size
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("tick_label_font_size_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"tick_label_font_size_spinbutton\" object in " + resource_file_);
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoArcShapeSettingsBox::on_widget_spin_changed), spinbutton, 
					[this](const int &value) {
						log_notice("Widget %s: tick label font size changed to '%d'",
							   widget_->name().c_str(), value);

						widget_->theme().setTickLabelFontSize(value);

						// Broadcast widget change
						widget_->dispatchEvent();
					}
			));

	// Tick label color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("tick_label_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"tick_label_color_button\" object in " + resource_file_);
	colorbutton->signal_color_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoArcShapeSettingsBox::on_widget_color_changed), colorbutton, 
					[this](const std::string &color) {
						log_notice("Widget %s: tick label color changed to '%s'", 
								widget_->name().c_str(), color.c_str());

						widget_->theme().setTickLabelColor(color);

						// Broadcast widget change
						widget_->dispatchEvent();
					}
			));

	// Tick label border color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("tick_label_border_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"tick_label_border_color_button\" object in " + resource_file_);
	colorbutton->signal_color_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoArcShapeSettingsBox::on_widget_color_changed), colorbutton, 
					[this](const std::string &color) {
						log_notice("Widget %s: tick label border color changed to '%s'", 
								widget_->name().c_str(), color.c_str());

						widget_->theme().setTickLabelBorderColor(color);

						// Broadcast widget change
						widget_->dispatchEvent();
					}
			));

	// Needle enable
	sw = ref_builder_->get_widget<Gtk::Switch>("needle_enable_switch");
	if (!sw)
		throw std::runtime_error("No \"needle_enable_switch\" object in " + resource_file_);
	sw->signal_state_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoArcShapeSettingsBox::on_widget_switch_changed), sw, 
					[this](const bool &state) {
						log_notice("Widget %s: needle status changed to '%s'",
							   widget_->name().c_str(), state ? "enabled" : "disabled");

						if (state)
							widget_->theme().addFlag(VideoWidget::Theme::FlagNeedle);
						else
							widget_->theme().removeFlag(VideoWidget::Theme::FlagNeedle);

						// Broadcast widget change
						widget_->dispatchEvent();
					} 
			), false);

	// Needle type
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("needle_type_combobox");
	if (!combobox)
		throw std::runtime_error("No \"needle_type_combobox\" object in " + resource_file_);
	combobox->pack_start(model_.m_name);
	combobox->signal_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoArcShapeSettingsBox::on_widget_combobox_changed), combobox, 
					[this](const Gtk::TreeModel::const_iterator &iter) {
						int value = iter->get_value(model_.m_id);

						log_notice("Widget %s: needle type changed to '%s'", 
								widget_->name().c_str(), iter->get_value(model_.m_name).c_str());

						widget_->theme().setNeedleType((VideoWidget::Theme::NeedleType) value);

						// Broadcast widget change
						widget_->dispatchEvent();
					}
			));

	// Needle primary color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("needle_primary_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"needle_primary_color_button\" object in " + resource_file_);
	colorbutton->signal_color_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoArcShapeSettingsBox::on_widget_color_changed), colorbutton, 
					[this](const std::string &color) {
						log_notice("Widget %s: needle primary color changed to '%s'", 
								widget_->name().c_str(), color.c_str());

						widget_->theme().setNeedlePrimaryColor(color);

						// Broadcast widget change
						widget_->dispatchEvent();
					}
			));

	// Needle secondary color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("needle_secondary_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"needle_secondary_color_button\" object in " + resource_file_);
	colorbutton->signal_color_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoArcShapeSettingsBox::on_widget_color_changed), colorbutton, 
					[this](const std::string &color) {
						log_notice("Widget %s: needle secondary color changed to '%s'", 
								widget_->name().c_str(), color.c_str());

						widget_->theme().setNeedleSecondaryColor(color);

						// Broadcast widget change
						widget_->dispatchEvent();
					}
			));
}


void GPX2VideoArcShapeSettingsBox::update_content(void) {
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

	// Widget gauge enable switch
	sw = ref_builder_->get_widget<Gtk::Switch>("gauge_enable_switch");
	if (!sw)
		throw std::runtime_error("No \"gauge_enable_switch\" object in " + resource_file_);

	sw->set_active(widget_->theme().hasFlag(VideoWidget::Theme::FlagGauge));

	// Widget gauge angle button
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("gauge_angle_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"gauge_angle_spinbutton\" object in " + resource_file_);

	spinbutton->set_value(widget_->theme().gaugeAngle());

	// Widget gauge rotation button
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("gauge_rotation_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"gauge_rotation_spinbutton\" object in " + resource_file_);

	spinbutton->set_value(widget_->theme().gaugeRotation());

	// Widget gauge flip switch
	sw = ref_builder_->get_widget<Gtk::Switch>("gauge_flip_switch");
	if (!sw)
		throw std::runtime_error("No \"gauge_flip_switch\" object in " + resource_file_);

	sw->set_active(widget_->theme().gaugeFlip());

	// Widget gauge width button
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("gauge_width_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"gauge_width_spinbutton\" object in " + resource_file_);

	spinbutton->set_value(widget_->theme().gaugeWidth());

	// Widget gauge cap combobox
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("gauge_cap_combobox");
	if (!combobox)
		throw std::runtime_error("No \"gauge_cap_combobox\" object in " + resource_file_);

	combobox->set_model(gauge_cap_model_);

	if (find_in_listtore(gauge_cap_model_, widget_->theme().gaugeCap(), iter))
		combobox->set_active(iter);

	// Widget gauge primary color button
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("gauge_primary_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"gauge_primary_color_button\" object in " + resource_file_);

	color = widget_->theme().gaugePrimaryColor();

	rgba.set_rgba(color[0], color[1], color[2], color[3]);

	colorbutton->set_rgba(rgba);

	// Widget gauge secondary color button
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("gauge_secondary_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"gauge_secondary_color_button\" object in " + resource_file_);

	color = widget_->theme().gaugeSecondaryColor();

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

	// Widget tick enable switch
	sw = ref_builder_->get_widget<Gtk::Switch>("tick_enable_switch");
	if (!sw)
		throw std::runtime_error("No \"tick_enable_switch\" object in " + resource_file_);

	sw->set_active(widget_->theme().hasFlag(VideoWidget::Theme::FlagTick));

	// Widget tick size button
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("tick_size_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"tick_size_spinbutton\" object in " + resource_file_);

	spinbutton->set_value(widget_->theme().tickSize());

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

	// Widget tick label distance button
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("tick_label_distance_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"tick_label_distance_spinbutton\" object in " + resource_file_);

	spinbutton->set_value(widget_->theme().tickLabelDistance());

	// Widget tick label font size button
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("tick_label_font_size_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"tick_label_font_size_spinbutton\" object in " + resource_file_);

	spinbutton->set_value(widget_->theme().tickLabelFontSize());

	// Widget tick label color button
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("tick_label_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"tick_label_color_button\" object in " + resource_file_);

	color = widget_->theme().tickLabelColor();

	rgba.set_rgba(color[0], color[1], color[2], color[3]);

	colorbutton->set_rgba(rgba);

	// Widget tick label border color button
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("tick_label_border_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"tick_label_border_color_button\" object in " + resource_file_);

	color = widget_->theme().tickLabelBorderColor();

	rgba.set_rgba(color[0], color[1], color[2], color[3]);

	colorbutton->set_rgba(rgba);

	// Widget needle enable switch
	sw = ref_builder_->get_widget<Gtk::Switch>("needle_enable_switch");
	if (!sw)
		throw std::runtime_error("No \"needle_enable_switch\" object in " + resource_file_);

	sw->set_active(widget_->theme().hasFlag(VideoWidget::Theme::FlagNeedle));

	// Widget needle type combobox
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("needle_type_combobox");
	if (!combobox)
		throw std::runtime_error("No \"needle_type_combobox\" object in " + resource_file_);

	combobox->set_model(needle_type_model_);

	if (find_in_listtore(needle_type_model_, widget_->theme().needleType(), iter))
		combobox->set_active(iter);

	// Widget needle primary color button
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("needle_primary_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"needle_primary_color_button\" object in " + resource_file_);

	color = widget_->theme().needlePrimaryColor();

	rgba.set_rgba(color[0], color[1], color[2], color[3]);

	colorbutton->set_rgba(rgba);

	// Widget needle secondary color button
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("needle_secondary_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"needle_secondary_color_button\" object in " + resource_file_);

	color = widget_->theme().needleSecondaryColor();

	rgba.set_rgba(color[0], color[1], color[2], color[3]);

	colorbutton->set_rgba(rgba);

	// Unmask value changed
	loading_ = false;

	// Apply limit
	update_boundaries();
}


void GPX2VideoArcShapeSettingsBox::update_boundaries(void) {
	log_call();
}

