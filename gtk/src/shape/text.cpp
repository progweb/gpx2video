#include "../log.h"
#include "text.h"


GPX2VideoTextSettingsBox::GPX2VideoTextSettingsBox(BaseObjectType *cobject,
	const Glib::RefPtr<Gtk::Builder> &ref_builder, std::string resource_file, GPX2VideoWidget *widget) 
	: GPX2VideoBaseSettingsBox(cobject, ref_builder, "GPX2VideoTextSettingsBox", resource_file) 
	, widget_(widget) {
	log_call();

	Gtk::Switch *sw;
//	Gtk::ComboBox *combobox;
//	Gtk::SpinButton *spinbutton;
//	Gtk::ColorButton *colorbutton;

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
				sigc::mem_fun(*this, &GPX2VideoTextSettingsBox::on_widget_switch_changed), sw, 
					[this](const bool &state) {
						if (state)
							widget_->theme().addFlag(VideoWidget::Theme::FlagIcon);
						else
							widget_->theme().removeFlag(VideoWidget::Theme::FlagIcon);

						// Broadcast widget change
						widget_->dispatchEvent();
					} 
			), false);
}


void GPX2VideoTextSettingsBox::update_content(void) {
	log_call();

//	Gdk::RGBA rgba;
//
//	const float *color;

	Gtk::Switch *sw;
//	Gtk::ComboBox *combobox;
//	Gtk::SpinButton *spinbutton;
//	Gtk::ColorButton *colorbutton;
//
//	Gtk::TreeModel::iterator iter;

	// Mask value changed
	loading_ = true;

	// Widget icon enable switch
	sw = ref_builder_->get_widget<Gtk::Switch>("icon_enable_switch");
	if (!sw)
		throw std::runtime_error("No \"icon_enable_switch\" object in " + resource_file_);

	sw->set_active(widget_->theme().hasFlag(VideoWidget::Theme::FlagIcon));

	// Unmask value changed
	loading_ = false;
}

