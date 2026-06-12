#include <glibmm/i18n.h>

#include "../log_i.h"
#include "bar.h"


GPX2VideoBarShapeSettingsBox::GPX2VideoBarShapeSettingsBox(BaseObjectType *cobject,
	const Glib::RefPtr<Gtk::Builder> &ref_builder, std::string resource_file, GPX2VideoWidget *widget) 
	: GPX2VideoShapeBaseSettingsBox(cobject, ref_builder, "GPX2VideoBarShapeSettingsBox", resource_file, widget) {
	log_call();

	loading_ = false;

	// Populate models
	load_models();

	// Binding
	bind_content();
}


void GPX2VideoBarShapeSettingsBox::load_models(void) {
	log_call();

	gauge_orientation_model_ = Gtk::ListStore::create(model_);

	{
		auto iter = gauge_orientation_model_->append();
		auto row = *iter;
		row[model_.m_id] = VideoWidget::OrientationHorizontal;
		row[model_.m_name] = _("Horizontal");
		row[model_.m_enable] = true;

		row = *(gauge_orientation_model_->append());
		row[model_.m_id] = VideoWidget::OrientationVertical;
		row[model_.m_name] = _("Vertical");
		row[model_.m_enable] = true;
	}

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

	tick_align_model_ = Gtk::ListStore::create(model_);

	{
		auto iter = tick_align_model_->append();
		auto row = *iter;
		row[model_.m_id] = VideoWidget::Theme::AlignLeft;
		row[model_.m_name] = _("Left");

		row = *(tick_align_model_->append());
		row[model_.m_id] = VideoWidget::Theme::AlignCenter;
		row[model_.m_name] = _("Center");

		row = *(tick_align_model_->append());
		row[model_.m_id] = VideoWidget::Theme::AlignRight;
		row[model_.m_name] = _("Right");
	}
}


void GPX2VideoBarShapeSettingsBox::bind_content(void) {
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
				sigc::mem_fun(*this, &GPX2VideoBarShapeSettingsBox::on_widget_switch_changed), sw, 
					[this](const bool &state) {
						log_notice("Widget %s: gauge status changed to '%s'",
							   widget_->name().c_str(), state ? "enabled" : "disabled");

						if (state)
							widget_->theme().addFlag(VideoWidget::Theme::FlagGauge);
						else
							widget_->theme().removeFlag(VideoWidget::Theme::FlagGauge);

						// Broadcast widget change
						widget_->dispatchEvent(true);
					} 
			), false);

	// Gauge orientation
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("gauge_orientation_combobox");
	if (!combobox)
		throw std::runtime_error("No \"gauge_orientation_combobox\" object in " + resource_file_);
	combobox->pack_start(model_.m_name);
	combobox->signal_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoBarShapeSettingsBox::on_widget_combobox_changed), combobox, 
					[this](const Gtk::TreeModel::const_iterator &iter) {
						int value = iter->get_value(model_.m_id);

						int angle = ((VideoWidget::Orientation) value == VideoWidget::OrientationHorizontal) ? 90 : 0;

						log_notice("Widget %s: gauge orientation changed to '%s'", 
								widget_->name().c_str(), iter->get_value(model_.m_name).c_str());

						widget_->theme().setGaugeRotation(angle);

						// Broadcast widget change
						widget_->dispatchEvent(true);
					}
			));

	// Gauge flip
	sw = ref_builder_->get_widget<Gtk::Switch>("gauge_flip_switch");
	if (!sw)
		throw std::runtime_error("No \"gauge_flip_switch\" object in " + resource_file_);
	sw->signal_state_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoBarShapeSettingsBox::on_widget_switch_changed), sw, 
					[this](const bool &state) {
						log_notice("Widget %s: gauge flip changed to '%s'",
							   widget_->name().c_str(), state ? "enabled" : "disabled");

						widget_->theme().setGaugeFlip(state);

						// Broadcast widget change
						widget_->dispatchEvent(true);
					} 
			), false);

	// Gauge width
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("gauge_width_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"gauge_width_spinbutton\" object in " + resource_file_);
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoBarShapeSettingsBox::on_widget_spin_changed), spinbutton, 
					[this](const int &value) {
						log_notice("Widget %s: gauge width changed to '%d'",
							   widget_->name().c_str(), value);

						widget_->theme().setGaugeWidth(value);

						// Broadcast widget change
						widget_->dispatchEvent(true);
					}
			));

	// Gauge cap
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("gauge_cap_combobox");
	if (!combobox)
		throw std::runtime_error("No \"gauge_cap_combobox\" object in " + resource_file_);
	combobox->pack_start(model_.m_name);
	combobox->signal_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoBarShapeSettingsBox::on_widget_combobox_changed), combobox, 
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
				sigc::mem_fun(*this, &GPX2VideoBarShapeSettingsBox::on_widget_color_changed), colorbutton, 
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
				sigc::mem_fun(*this, &GPX2VideoBarShapeSettingsBox::on_widget_color_changed), colorbutton, 
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
				sigc::mem_fun(*this, &GPX2VideoBarShapeSettingsBox::on_widget_spin_changed), spinbutton, 
					[this](const int &value) {
						log_notice("Widget %s: gauge border changed to '%d'",
							   widget_->name().c_str(), value);

						widget_->theme().setGaugeBorder(value);

						// Broadcast widget change
						widget_->dispatchEvent(true);
					}
			));

	// Gauge border color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("gauge_border_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"gauge_border_color_button\" object in " + resource_file_);
	colorbutton->signal_color_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoBarShapeSettingsBox::on_widget_color_changed), colorbutton, 
					[this](const std::string &color) {
						log_notice("Widget '%s' gauge border color changed to '%s'", 
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
				sigc::mem_fun(*this, &GPX2VideoBarShapeSettingsBox::on_widget_color_changed), colorbutton, 
					[this](const std::string &color) {
						log_notice("Widget '%s' gauge background color changed to '%s'", 
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
				sigc::mem_fun(*this, &GPX2VideoBarShapeSettingsBox::on_widget_switch_changed), sw, 
					[this](const bool &state) {
						log_notice("Widget %s: tick status changed to '%s'",
							   widget_->name().c_str(), state ? "enabled" : "disabled");

						if (state)
							widget_->theme().addFlag(VideoWidget::Theme::FlagTick);
						else
							widget_->theme().removeFlag(VideoWidget::Theme::FlagTick);

						// Broadcast widget change
						widget_->dispatchEvent(true);
					} 
			), false);

	// Tick size
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("tick_size_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"tick_size_spinbutton\" object in " + resource_file_);
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoBarShapeSettingsBox::on_widget_spin_changed), spinbutton, 
					[this](const int &value) {
						log_notice("Widget %s: tick size changed to '%d'",
							   widget_->name().c_str(), value);

						widget_->theme().setTickSize(value);

						// Broadcast widget change
						widget_->dispatchEvent(true);
					}
			));

	// Tick color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("tick_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"tick_color_button\" object in " + resource_file_);
	colorbutton->signal_color_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoBarShapeSettingsBox::on_widget_color_changed), colorbutton, 
					[this](const std::string &color) {
						log_notice("Widget '%s' tick color changed to '%s'", 
								widget_->name().c_str(), color.c_str());

						widget_->theme().setTickColor(color);

						// Broadcast widget change
						widget_->dispatchEvent();
					}
			));

	// Tick alignment
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("tick_align_combobox");
	if (!combobox)
		throw std::runtime_error("No \"tick_align_combobox\" object in " + resource_file_);
	combobox->pack_start(model_.m_name);
	combobox->signal_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoBarShapeSettingsBox::on_widget_combobox_changed), combobox, 
					[this](const Gtk::TreeModel::const_iterator &iter) {
						int value = iter->get_value(model_.m_id);

						log_notice("Widget %s: tick align changed to '%s'", 
								widget_->name().c_str(), iter->get_value(model_.m_name).c_str());

						widget_->theme().setTickAlign((VideoWidget::Theme::Align) value);

						// Broadcast widget change
						widget_->dispatchEvent();
					}
			));

	// Tick label enable
	sw = ref_builder_->get_widget<Gtk::Switch>("tick_label_enable_switch");
	if (!sw)
		throw std::runtime_error("No \"tick_label_enable_switch\" object in " + resource_file_);
	sw->signal_state_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoBarShapeSettingsBox::on_widget_switch_changed), sw, 
					[this](const bool &state) {
						log_notice("Widget %s: tick label status changed to '%s'",
							   widget_->name().c_str(), state ? "enabled" : "disabled");

						if (state)
							widget_->theme().addFlag(VideoWidget::Theme::FlagTickLabel);
						else
							widget_->theme().removeFlag(VideoWidget::Theme::FlagTickLabel);

						// Broadcast widget change
						widget_->dispatchEvent(true);
					} 
			), false);

	// Tick label distance
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("tick_label_distance_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"tick_label_distance_spinbutton\" object in " + resource_file_);
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoBarShapeSettingsBox::on_widget_spin_changed), spinbutton, 
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
				sigc::mem_fun(*this, &GPX2VideoBarShapeSettingsBox::on_widget_spin_changed), spinbutton, 
					[this](const int &value) {
						log_notice("Widget %s: tick label font size changed to '%d'",
							   widget_->name().c_str(), value);

						widget_->theme().setTickLabelFontSize(value);

						// Broadcast widget change
						widget_->dispatchEvent(true);
					}
			));

	// Tick label color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("tick_label_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"tick_label_color_button\" object in " + resource_file_);
	colorbutton->signal_color_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoBarShapeSettingsBox::on_widget_color_changed), colorbutton, 
					[this](const std::string &color) {
						log_notice("Widget '%s' tick label color changed to '%s'", 
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
				sigc::mem_fun(*this, &GPX2VideoBarShapeSettingsBox::on_widget_color_changed), colorbutton, 
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
				sigc::mem_fun(*this, &GPX2VideoBarShapeSettingsBox::on_widget_switch_changed), sw, 
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

	// Needle distance
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("needle_distance_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"needle_distance_spinbutton\" object in " + resource_file_);
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoBarShapeSettingsBox::on_widget_spin_changed), spinbutton, 
					[this](const int &value) {
						log_notice("Widget %s: needle distance changed to '%d'",
							   widget_->name().c_str(), value);

						widget_->theme().setNeedleDistance(value);

						// Broadcast widget change
						widget_->dispatchEvent();
					}
			));

	// Needle border color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("needle_border_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"needle_border_color_button\" object in " + resource_file_);
	colorbutton->signal_color_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoBarShapeSettingsBox::on_widget_color_changed), colorbutton, 
					[this](const std::string &color) {
						log_notice("Widget %s: needle border color changed to '%s'", 
								widget_->name().c_str(), color.c_str());

						widget_->theme().setNeedleBorderColor(color);

						// Broadcast widget change
						widget_->dispatchEvent();
					}
			));

	// Needle background color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("needle_background_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"needle_background_color_button\" object in " + resource_file_);
	colorbutton->signal_color_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoBarShapeSettingsBox::on_widget_color_changed), colorbutton, 
					[this](const std::string &color) {
						log_notice("Widget %s: needle background color changed to '%s'", 
								widget_->name().c_str(), color.c_str());

						widget_->theme().setNeedleBackgroundColor(color);

						// Broadcast widget change
						widget_->dispatchEvent();
					}
			));
}



void GPX2VideoBarShapeSettingsBox::update_content(void) {
	log_call();

	Gdk::RGBA rgba;

	const float *color;

	VideoWidget::Orientation orientation;

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

	// Widget gauge orientation button
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("gauge_orientation_combobox");
	if (!combobox)
		throw std::runtime_error("No \"gauge_orientation_spinbutton\" object in " + resource_file_);

	orientation = (widget_->theme().gaugeRotation() != 0) ? VideoWidget::OrientationHorizontal : VideoWidget::OrientationVertical;

	combobox->set_model(gauge_orientation_model_);

	if (find_in_listtore(gauge_orientation_model_, orientation, iter))
		combobox->set_active(iter);

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

	// Widget tick alignment
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("tick_align_combobox");
	if (!combobox)
		throw std::runtime_error("No \"tick_align_combobox\" object in " + resource_file_);

	combobox->set_model(tick_align_model_);

	if (find_in_listtore(tick_align_model_, widget_->theme().tickAlign(), iter))
		combobox->set_active(iter);

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

	// Widget needle distance button
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("needle_distance_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"needle_distance_spinbutton\" object in " + resource_file_);

	spinbutton->set_value(widget_->theme().needleDistance());

	// Widget needle border color button
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("needle_border_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"needle_border_color_button\" object in " + resource_file_);

	color = widget_->theme().needleBorderColor();

	rgba.set_rgba(color[0], color[1], color[2], color[3]);

	colorbutton->set_rgba(rgba);

	// Widget needle background color button
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("needle_background_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"needle_background_color_button\" object in " + resource_file_);

	color = widget_->theme().needleBackgroundColor();

	rgba.set_rgba(color[0], color[1], color[2], color[3]);

	colorbutton->set_rgba(rgba);

	// Unmask value changed
	loading_ = false;

	// Apply limit
	update_boundaries();
}


void GPX2VideoBarShapeSettingsBox::update_boundaries(void) {
	log_call();
}

