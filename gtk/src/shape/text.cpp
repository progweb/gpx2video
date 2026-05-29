#include "../log_i.h"
#include "text.h"


GPX2VideoTextShapeSettingsBox::GPX2VideoTextShapeSettingsBox(BaseObjectType *cobject,
	const Glib::RefPtr<Gtk::Builder> &ref_builder, std::string resource_file, GPX2VideoWidget *widget) 
	: GPX2VideoShapeBaseSettingsBox(cobject, ref_builder, "GPX2VideoTextShapeSettingsBox", resource_file, widget) {
	log_call();

	Gtk::Switch *sw;
//	Gtk::ComboBox *combobox;
//	Gtk::SpinButton *spinbutton;
	Gtk::ColorButton *colorbutton;

	loading_ = false;

	// Populate models
	//-----------------

//	tick_align_model_ = Gtk::ListStore::create(model_);
//
//	{
//		auto iter = tick_align_model_->append();
//		auto row = *iter;
//		row[model_.m_id] = VideoWidget::Theme::AlignLeft;
//		row[model_.m_name] = "Left";
//
//		row = *(tick_align_model_->append());
//		row[model_.m_id] = VideoWidget::Theme::AlignCenter;
//		row[model_.m_name] = "Center";
//
//		row = *(tick_align_model_->append());
//		row[model_.m_id] = VideoWidget::Theme::AlignRight;
//		row[model_.m_name] = "Right";
//	}

	// Icon enable
	sw = ref_builder_->get_widget<Gtk::Switch>("icon_enable_switch");
	if (!sw)
		throw std::runtime_error("No \"icon_enable_switch\" object in " + resource_file_);
	sw->signal_state_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoTextShapeSettingsBox::on_widget_switch_changed), sw, 
					[this](const bool &state) {
						if (state)
							widget_->theme().addFlag(VideoWidget::Theme::FlagIcon);
						else
							widget_->theme().removeFlag(VideoWidget::Theme::FlagIcon);

						// Broadcast widget change
						widget_->dispatchEvent();
					} 
			), false);

	// Icon color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("icon_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"icon_color_button\" object in " + resource_file_);
	colorbutton->signal_color_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoTextShapeSettingsBox::on_widget_color_changed), colorbutton, 
					[this](const std::string &color) {
						log_notice("Widget %s: icon color changed to '%s'", 
								widget_->name().c_str(), color.c_str());

						widget_->theme().setIconColor(color);

						// Broadcast widget change
						widget_->dispatchEvent();
					}
			));
}


void GPX2VideoTextShapeSettingsBox::update_content(void) {
	log_call();

	Gdk::RGBA rgba;

	const float *color;

	Gtk::Switch *sw;
//	Gtk::ComboBox *combobox;
//	Gtk::SpinButton *spinbutton;
	Gtk::ColorButton *colorbutton;

//	Gtk::TreeModel::iterator iter;

	// Mask value changed
	loading_ = true;

	// Widget icon enable switch
	sw = ref_builder_->get_widget<Gtk::Switch>("icon_enable_switch");
	if (!sw)
		throw std::runtime_error("No \"icon_enable_switch\" object in " + resource_file_);

	sw->set_active(widget_->theme().hasFlag(VideoWidget::Theme::FlagIcon));

	// Widget icon color button
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("icon_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"icon_color_button\" object in " + resource_file_);

	color = widget_->theme().iconColor();

	rgba.set_rgba(color[0], color[1], color[2], color[3]);

	colorbutton->set_rgba(rgba);

	// Unmask value changed
	loading_ = false;
}

