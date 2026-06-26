#include <glibmm/i18n.h>

#include <gtkmm/image.h>
#include <gtkmm/gridview.h>
#include <gtkmm/menubutton.h>

#include "../log_i.h"
#include "bar.h"


GPX2VideoBarShapeSettingsBox::GPX2VideoBarShapeSettingsBox(BaseObjectType *cobject,
	const Glib::RefPtr<Gtk::Builder> &ref_builder, std::string resource_file, 
	GPX2VideoWidget *widget, const Glib::RefPtr<GPX2VideoMediaListStore> &media_model) 
	: GPX2VideoShapeBaseSettingsBox(cobject, ref_builder, "GPX2VideoBarShapeSettingsBox", resource_file, widget)
	, media_model_(media_model) 
	, icon_model_(NULL) {
	log_call();

	loading_ = false;

	// Default icon size
	icon_pixel_size_ = 40;
	icon_pixel_minsize_ = 16;
	icon_pixel_maxsize_ = 96;

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

	needle_type_model_ = Gtk::ListStore::create(model_);

	{
		auto iter = needle_type_model_->append();
		auto row = *iter;
		row[model_.m_id] = VideoWidget::Theme::NeedleTypeBasic;
		row[model_.m_name] = _("Basic");

		row = *(needle_type_model_->append());
		row[model_.m_id] = VideoWidget::Theme::NeedleTypeIcon;
		row[model_.m_name] = _("Icon");

		row = *(needle_type_model_->append());
		row[model_.m_id] = VideoWidget::Theme::NeedleTypeValue;
		row[model_.m_name] = _("Value");
	}

	auto model = Gio::ListStore<GPX2VideoBarShapeSettingsBox::Icon>::create();

	// Internal icon
	for (int i=0; i != VideoWidget::Theme::IconUnknown; i++) {
		if (i != VideoWidget::Theme::IconUserFile) {
			VideoWidget::Theme::Icon icon = (VideoWidget::Theme::Icon) i;

			std::string filename = widget_->widget()->getIconFilename(icon);

			model->append(Icon::create(
					icon,
					filename	
				)
			);
		}
	}

	// User icon
	guint n_items = media_model_->get_n_items();

	for (guint i=0; i < n_items; i++) {
		auto item = media_model_->get_item(i);
		if (!item || (item->media() != GPX2VideoMedia::MediaIcon))
			continue;

		model->append(Icon::create(
				VideoWidget::Theme::IconUserFile,
				item->filename()
			)
		);
	}

	// Wrap store in a SingleSelection model
	icon_model_ = Gtk::SingleSelection::create(model);

	// Create a factory to build and bind list icons
	icon_factory_ = Gtk::SignalListItemFactory::create();

	icon_factory_->signal_setup().connect([this] (const Glib::RefPtr<Gtk::ListItem>& list_item) {
		auto image = Gtk::make_managed<Gtk::Image>();

		image->set_pixel_size(icon_pixel_size_);

		list_item->set_child(*image);
	});

	icon_factory_->signal_bind().connect([this] (const Glib::RefPtr<Gtk::ListItem>& list_item) {
		auto image = dynamic_cast<Gtk::Image*>(list_item->get_child());
		auto item = std::dynamic_pointer_cast<GPX2VideoBarShapeSettingsBox::Icon>(list_item->get_item());

		if (image && item) {
			image->set_pixel_size(icon_pixel_size_);
			image->set(item->filename());
		}
	});
}


void GPX2VideoBarShapeSettingsBox::bind_content(void) {
	log_call();

	Gtk::Switch *sw;
	Gtk::ComboBox *combobox;
	Gtk::MenuButton *menubutton;
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

						log_notice("Widget %s: gauge orientation changed to '%s'", 
								widget_->name().c_str(), iter->get_value(model_.m_name).c_str());

						widget_->theme().setGaugeOrientation((VideoWidget::Orientation) value);

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
				sigc::mem_fun(*this, &GPX2VideoBarShapeSettingsBox::on_widget_spin_double_changed), spinbutton, 
					[this](const double &value) {
						log_notice("Widget %s: gauge width changed to '%.1f'",
							   widget_->name().c_str(), value);

						widget_->theme().setGaugeWidth(value);

						// Broadcast widget change
						widget_->dispatchEvent(true);
					}
			));

	// Gauge offset
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("gauge_offset_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"gauge_offset_spinbutton\" object in " + resource_file_);
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoBarShapeSettingsBox::on_widget_spin_double_changed), spinbutton, 
					[this](const double &value) {
						log_notice("Widget %s: gauge offset changed to '%.1f'",
							   widget_->name().c_str(), value);

						widget_->theme().setGaugeOffset(value);

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
						widget_->dispatchEvent(true);
					}
			));

	// Gauge border width
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("gauge_border_width_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"gauge_border_width_spinbutton\" object in " + resource_file_);
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoBarShapeSettingsBox::on_widget_spin_double_changed), spinbutton, 
					[this](const double &value) {
						log_notice("Widget %s: gauge border changed to '%.1f'",
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
				sigc::mem_fun(*this, &GPX2VideoBarShapeSettingsBox::on_widget_spin_double_changed), spinbutton, 
					[this](const double &value) {
						log_notice("Widget %s: tick size changed to '%.1f'",
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
				sigc::mem_fun(*this, &GPX2VideoBarShapeSettingsBox::on_widget_spin_double_changed), spinbutton, 
					[this](const double &value) {
						log_notice("Widget %s: tick label distance changed to '%.1f'",
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
				sigc::mem_fun(*this, &GPX2VideoBarShapeSettingsBox::on_widget_spin_double_changed), spinbutton, 
					[this](const double &value) {
						log_notice("Widget %s: tick label font size changed to '%.1f'",
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

	// Cursor enable
	sw = ref_builder_->get_widget<Gtk::Switch>("cursor_enable_switch");
	if (!sw)
		throw std::runtime_error("No \"cursor_enable_switch\" object in " + resource_file_);
	sw->signal_state_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoBarShapeSettingsBox::on_widget_switch_changed), sw, 
					[this](const bool &state) {
						log_notice("Widget %s: cursor status changed to '%s'",
							   widget_->widget()->name().c_str(), state ? "enabled" : "disabled");

						if (state)
							widget_->widget()->theme().addFlag(VideoWidget::Theme::FlagCursor);
						else
							widget_->widget()->theme().removeFlag(VideoWidget::Theme::FlagCursor);

						// Broadcast widget change
						widget_->dispatchEvent(false);
					} 
			), false);

	// Cursor width
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("cursor_width_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"cursor_width_spinbutton\" object in " + resource_file_);

	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoBarShapeSettingsBox::on_widget_spin_double_changed), spinbutton, 
					[this](const double &value) {
						log_notice("Widget %s: cursor width changed to '%.1f'",
							   widget_->name().c_str(), value);

						widget_->widget()->theme().setCursorWidth(value);

						// Broadcast widget change
						widget_->dispatchEvent(true);
					}
			));

	// Cursor color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("cursor_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"cursor_primary_color_button\" object in " + resource_file_);

	colorbutton->signal_color_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoBarShapeSettingsBox::on_widget_color_changed), colorbutton, 
					[this](const std::string &color) {
						log_notice("Widget %s: cursor primary color changed to '%s'", 
								widget_->name().c_str(), color.c_str());

						widget_->widget()->theme().setCursorColor(color);

						// Broadcast widget change
						widget_->dispatchEvent(true);
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

	// Needle type
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("needle_type_combobox");
	if (!combobox)
		throw std::runtime_error("No \"needle_type_combobox\" object in " + resource_file_);
	combobox->pack_start(model_.m_name);
	combobox->signal_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoBarShapeSettingsBox::on_widget_combobox_changed), combobox, 
					[this](const Gtk::TreeModel::const_iterator &iter) {
						int value = iter->get_value(model_.m_id);

						log_notice("Widget %s: needle type changed to '%s'", 
								widget_->name().c_str(), iter->get_value(model_.m_name).c_str());

						widget_->theme().setNeedleType((VideoWidget::Theme::NeedleType) value);

						// Broadcast widget change
						widget_->dispatchEvent();
					}
			));

	// Needle distance
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("needle_distance_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"needle_distance_spinbutton\" object in " + resource_file_);
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoBarShapeSettingsBox::on_widget_spin_double_changed), spinbutton, 
					[this](const double &value) {
						log_notice("Widget %s: needle distance changed to '%.1f'",
							   widget_->name().c_str(), value);

						widget_->theme().setNeedleDistance(value);

						// Broadcast widget change
						widget_->dispatchEvent();
					}
			));

	// Needle border
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("needle_border_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"needle_border_spinbutton\" object in " + resource_file_);
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoBarShapeSettingsBox::on_widget_spin_double_changed), spinbutton, 
					[this](const double &value) {
						log_notice("Widget %s: needle border changed to '%.1f'",
							   widget_->name().c_str(), value);

						widget_->theme().setNeedleBorder(value);

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

	// Icon menubutton
	menubutton = ref_builder_->get_widget<Gtk::MenuButton>("icon_menubutton");
	if (!menubutton)
		throw std::runtime_error("No \"icon_menubutton\" object in " + resource_file_);

	menubutton->set_create_popup_func(sigc::bind(
			sigc::mem_fun(*this, &GPX2VideoBarShapeSettingsBox::create_popover), menubutton));

	// Icon size
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("icon_size_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"icon_size_spinbutton\" object in " + resource_file_);
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoBarShapeSettingsBox::on_widget_spin_double_changed), spinbutton, 
					[this](const double &value) {
						log_notice("Widget %s: icon size changed to '%.1f'",
							   widget_->name().c_str(), value);

						widget_->theme().setIconSize(value);

						// Broadcast widget change
						widget_->dispatchEvent(true);
					}
			));

	// Icon color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("icon_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"icon_color_button\" object in " + resource_file_);
	colorbutton->signal_color_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoBarShapeSettingsBox::on_widget_color_changed), colorbutton, 
					[this](const std::string &color) {
						log_notice("Widget %s: icon color changed to '%s'", 
								widget_->name().c_str(), color.c_str());

						widget_->theme().setIconColor(color);

						// Broadcast widget change
						widget_->dispatchEvent();
					}
			));

	// Icon model
	media_model_->signal_items_changed().connect(sigc::mem_fun(*this, &GPX2VideoBarShapeSettingsBox::on_model_changed));
}



void GPX2VideoBarShapeSettingsBox::update_content(void) {
	log_call();

	Gdk::RGBA rgba;

	const float *color;

	Gtk::Switch *sw;
	Gtk::Image *image;
	Gtk::ComboBox *combobox;
	Gtk::SpinButton *spinbutton;
	Gtk::ColorButton *colorbutton;

	Gtk::TreeModel::iterator iter;

	// Mask value changed
	loading_ = true;

	// Gauge enable switch
	sw = ref_builder_->get_widget<Gtk::Switch>("gauge_enable_switch");
	if (!sw)
		throw std::runtime_error("No \"gauge_enable_switch\" object in " + resource_file_);

	sw->set_active(widget_->theme().hasFlag(VideoWidget::Theme::FlagGauge));

	// Gauge orientation button
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("gauge_orientation_combobox");
	if (!combobox)
		throw std::runtime_error("No \"gauge_orientation_spinbutton\" object in " + resource_file_);

	combobox->set_model(gauge_orientation_model_);

	if (find_in_listtore(gauge_orientation_model_, widget_->theme().gaugeOrientation(), iter))
		combobox->set_active(iter);

	// Gauge flip switch
	sw = ref_builder_->get_widget<Gtk::Switch>("gauge_flip_switch");
	if (!sw)
		throw std::runtime_error("No \"gauge_flip_switch\" object in " + resource_file_);

	sw->set_active(widget_->theme().gaugeFlip());

	// Gauge width button
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("gauge_width_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"gauge_width_spinbutton\" object in " + resource_file_);

	spinbutton->set_value(widget_->theme().gaugeWidth());

	// Gauge offset button
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("gauge_offset_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"gauge_offset_spinbutton\" object in " + resource_file_);

	spinbutton->set_value(widget_->theme().gaugeOffset());

	// Gauge cap combobox
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("gauge_cap_combobox");
	if (!combobox)
		throw std::runtime_error("No \"gauge_cap_combobox\" object in " + resource_file_);

	combobox->set_model(gauge_cap_model_);

	if (find_in_listtore(gauge_cap_model_, widget_->theme().gaugeCap(), iter))
		combobox->set_active(iter);

	// Gauge border width button
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("gauge_border_width_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"gauge_border_width_spinbutton\" object in " + resource_file_);

	spinbutton->set_value(widget_->theme().gaugeBorder());

	// Gauge border color button
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("gauge_border_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"gauge_border_color_button\" object in " + resource_file_);

	color = widget_->theme().gaugeBorderColor();

	rgba.set_rgba(color[0], color[1], color[2], color[3]);

	colorbutton->set_rgba(rgba);

	// Gauge background color button
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("gauge_background_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"gauge_background_color_button\" object in " + resource_file_);

	color = widget_->theme().gaugeBackgroundColor();

	rgba.set_rgba(color[0], color[1], color[2], color[3]);

	colorbutton->set_rgba(rgba);

	// Gauge primary color button
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("gauge_primary_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"gauge_primary_color_button\" object in " + resource_file_);

	color = widget_->theme().gaugePrimaryColor();

	rgba.set_rgba(color[0], color[1], color[2], color[3]);

	colorbutton->set_rgba(rgba);

	// Gauge secondary color button
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("gauge_secondary_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"gauge_secondary_color_button\" object in " + resource_file_);

	color = widget_->theme().gaugeSecondaryColor();

	rgba.set_rgba(color[0], color[1], color[2], color[3]);

	colorbutton->set_rgba(rgba);

	// Tick enable switch
	sw = ref_builder_->get_widget<Gtk::Switch>("tick_enable_switch");
	if (!sw)
		throw std::runtime_error("No \"tick_enable_switch\" object in " + resource_file_);

	sw->set_active(widget_->theme().hasFlag(VideoWidget::Theme::FlagTick));

	// Tick size button
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("tick_size_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"tick_size_spinbutton\" object in " + resource_file_);

	spinbutton->set_value(widget_->theme().tickSize());

	// Tick alignment
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("tick_align_combobox");
	if (!combobox)
		throw std::runtime_error("No \"tick_align_combobox\" object in " + resource_file_);

	combobox->set_model(tick_align_model_);

	if (find_in_listtore(tick_align_model_, widget_->theme().tickAlign(), iter))
		combobox->set_active(iter);

	// Tick color button
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("tick_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"tick_color_button\" object in " + resource_file_);

	color = widget_->theme().tickColor();

	rgba.set_rgba(color[0], color[1], color[2], color[3]);

	colorbutton->set_rgba(rgba);

	// Tick label enable switch
	sw = ref_builder_->get_widget<Gtk::Switch>("tick_label_enable_switch");
	if (!sw)
		throw std::runtime_error("No \"tick_label_enable_switch\" object in " + resource_file_);

	sw->set_active(widget_->theme().hasFlag(VideoWidget::Theme::FlagTickLabel));

	// Tick label distance button
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("tick_label_distance_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"tick_label_distance_spinbutton\" object in " + resource_file_);

	spinbutton->set_value(widget_->theme().tickLabelDistance());

	// Tick label font size button
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("tick_label_font_size_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"tick_label_font_size_spinbutton\" object in " + resource_file_);

	spinbutton->set_value(widget_->theme().tickLabelFontSize());

	// Tick label color button
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("tick_label_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"tick_label_color_button\" object in " + resource_file_);

	color = widget_->theme().tickLabelColor();

	rgba.set_rgba(color[0], color[1], color[2], color[3]);

	colorbutton->set_rgba(rgba);

	// Tick label border color button
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("tick_label_border_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"tick_label_border_color_button\" object in " + resource_file_);

	color = widget_->theme().tickLabelBorderColor();

	rgba.set_rgba(color[0], color[1], color[2], color[3]);

	colorbutton->set_rgba(rgba);

	// Cursor enable switch
	sw = ref_builder_->get_widget<Gtk::Switch>("cursor_enable_switch");
	if (!sw)
		throw std::runtime_error("No \"cursor_enable_switch\" object in " + resource_file_);

	sw->set_active(widget_->theme().hasFlag(VideoWidget::Theme::FlagCursor));

	// Curve width
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("cursor_width_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"cursor_width_spinbutton\" object in " + resource_file_);

	spinbutton->set_value(widget_->widget()->theme().cursorWidth());

	// Cursor color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("cursor_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"cursor_color_button\" object in " + resource_file_);

	color = widget_->widget()->theme().cursorColor();

	rgba.set_rgba(color[0], color[1], color[2], color[3]);

	colorbutton->set_rgba(rgba);

	// Needle enable switch
	sw = ref_builder_->get_widget<Gtk::Switch>("needle_enable_switch");
	if (!sw)
		throw std::runtime_error("No \"needle_enable_switch\" object in " + resource_file_);

	sw->set_active(widget_->theme().hasFlag(VideoWidget::Theme::FlagNeedle));

	// Needle type combobox
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("needle_type_combobox");
	if (!combobox)
		throw std::runtime_error("No \"needle_type_combobox\" object in " + resource_file_);

	combobox->set_model(needle_type_model_);

	if (find_in_listtore(needle_type_model_, widget_->theme().needleType(), iter))
		combobox->set_active(iter);

	// Needle distance button
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("needle_distance_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"needle_distance_spinbutton\" object in " + resource_file_);

	spinbutton->set_value(widget_->theme().needleDistance());

	// Needle border button
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("needle_border_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"needle_border_spinbutton\" object in " + resource_file_);

	spinbutton->set_value(widget_->theme().needleBorder());

	// Needle border color button
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("needle_border_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"needle_border_color_button\" object in " + resource_file_);

	color = widget_->theme().needleBorderColor();

	rgba.set_rgba(color[0], color[1], color[2], color[3]);

	colorbutton->set_rgba(rgba);

	// Needle background color button
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("needle_background_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"needle_background_color_button\" object in " + resource_file_);

	color = widget_->theme().needleBackgroundColor();

	rgba.set_rgba(color[0], color[1], color[2], color[3]);

	colorbutton->set_rgba(rgba);

	// Icon image
	image = ref_builder_->get_widget<Gtk::Image>("icon_image");
	if (!image)
		throw std::runtime_error("No \"icon_image\" object in " + resource_file_);

	image->set_pixel_size(24);
	image->set(widget_->widget()->getIconFilename(widget_->theme().icon()));

	// Icon size
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("icon_size_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"icon_size_spinbutton\" object in " + resource_file_);

	spinbutton->set_value(widget_->theme().iconSize());

	// Icon color button
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("icon_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"icon_color_button\" object in " + resource_file_);

	color = widget_->theme().iconColor();

	rgba.set_rgba(color[0], color[1], color[2], color[3]);

	colorbutton->set_rgba(rgba);

	// Unmask value changed
	loading_ = false;

	// Apply limit
	update_boundaries();
}


void GPX2VideoBarShapeSettingsBox::update_boundaries(void) {
	log_call();

	Gtk::Button *button;

	// Icon zoomin button
	button = ref_builder_->get_widget<Gtk::Button>("icon_zoomin_button");
	if (!button)
		throw std::runtime_error("No \"icon_zoomin_button\" object in " + resource_file_);

	button->set_sensitive(icon_pixel_size_ < icon_pixel_maxsize_);

	// Icon zoomout button
	button = ref_builder_->get_widget<Gtk::Button>("icon_zoomout_button");
	if (!button)
		throw std::runtime_error("No \"icon_zoomout_button\" object in " + resource_file_);

	button->set_sensitive(icon_pixel_size_ > icon_pixel_minsize_);
}


void GPX2VideoBarShapeSettingsBox::set_default(void) {
	log_call();

	double fontsize;
	double tickfontsize;
	double distance;

	if (widget_->theme().gaugeOrientation() == VideoWidget::OrientationVertical) {
		fontsize = 5.0;
		tickfontsize = 5.0;
		distance = 20.0;
	}
	else {
		fontsize = 10.0;
		tickfontsize = 13.0;
		distance = 15.0;
	}

	widget_->theme().setValueFontSize(fontsize);
	widget_->theme().setValueShadowOpacity(80);
	widget_->theme().setValueShadowDistance(5.0);
	widget_->theme().setValueHorizontalAlign(VideoWidget::Theme::AlignLeft);
	widget_->theme().setValueVerticalAlign(VideoWidget::Theme::AlignTop);
	widget_->theme().setValueBorderWidth(5.0);

	widget_->theme().setValueFontSize(fontsize);
	widget_->theme().setValueShadowOpacity(80);
	widget_->theme().setValueShadowDistance(5.0);
	widget_->theme().setValueHorizontalAlign(VideoWidget::Theme::AlignCenter);
	widget_->theme().setValueVerticalAlign(VideoWidget::Theme::AlignBottom);
	widget_->theme().setValueBorderWidth(5.0);

	widget_->theme().setUnitFontSize(fontsize);

	widget_->theme().setGaugeWidth(10.0);

	widget_->theme().setTickSize(10.0);
	widget_->theme().setTickLabelDistance(distance);
	widget_->theme().setTickLabelFontSize(tickfontsize);
}


void GPX2VideoBarShapeSettingsBox::create_popover(Gtk::MenuButton *menubutton) {
	log_call();

	int index;

	Gtk::Button *button;
	Gtk::Popover *popover;
	Gtk::GridView *gridview;
	Gtk::CheckButton *checkbutton;

	// Widget icon model
	index = widget_->theme().icon();
	if (index == VideoWidget::Theme::IconDefault)
		index = widget_->widget()->type();
	else if (index == VideoWidget::Theme::IconUserFile) {
		guint n_items;

		auto model = icon_model_->get_model();

		n_items = model->get_n_items();

		for (guint i=0; i < n_items; i++) {
			auto item = std::dynamic_pointer_cast<GPX2VideoBarShapeSettingsBox::Icon>(model->get_object(i));
			if (!item || item->icon() != VideoWidget::Theme::IconUserFile)
				continue;
			if (item->filename() != widget_->theme().iconFile())
				continue;
			index = i;
			break;
		}

		// Icon not found!
		if (index == VideoWidget::Theme::IconUserFile) {
			// Auto append
			media_model_->append(GPX2VideoMedia::MediaIcon, widget_->theme().iconFile());

			// Select
			index = n_items;
		}
	}

	icon_model_->set_selected(index);

	// Icon popover
	popover = ref_builder_->get_widget<Gtk::Popover>("icon_popover");
	if (!popover)
		throw std::runtime_error("No \"icon_popover\" object in " + resource_file_);

	menubutton->set_popover(*popover);

	// Icon gridview
	gridview = ref_builder_->get_widget<Gtk::GridView>("icon_gridview");
	if (!gridview)
		throw std::runtime_error("No \"icon_gridview\" object in " + resource_file_);

	gridview->set_sensitive((widget_->theme().icon() != VideoWidget::Theme::IconDefault));
	gridview->set_model(icon_model_);
	gridview->set_factory(icon_factory_);
	gridview->set_min_columns(1);
	gridview->set_max_columns(10);

	// Icon gridview css
	gridview->get_style_context()->add_class("icon-grid");

	// Icon check button
	checkbutton = ref_builder_->get_widget<Gtk::CheckButton>("icon_usedefault_checkbutton");
	if (!checkbutton)
		throw std::runtime_error("No \"icon_usedefault_checkbutton\" object in " + resource_file_);

	checkbutton->set_active((widget_->theme().icon() == VideoWidget::Theme::IconDefault));

	// Connect icon checkbutton
	icon_checkbutton_connection_.disconnect();
	icon_checkbutton_connection_ = checkbutton->signal_toggled().connect(
			sigc::mem_fun(*this, &GPX2VideoBarShapeSettingsBox::on_icon_usedefault_toggled));

	// Connect icon zoomin button
	button = ref_builder_->get_widget<Gtk::Button>("icon_zoomin_button");
	if (!button)
		throw std::runtime_error("No \"icon_zoomin_button\" object in " + resource_file_);

	icon_zoomin_connection_.disconnect();
	icon_zoomin_connection_ = button->signal_clicked().connect(
			sigc::mem_fun(*this, &GPX2VideoBarShapeSettingsBox::on_icon_zoomin_clicked));

	// Connect icon zoomout button
	button = ref_builder_->get_widget<Gtk::Button>("icon_zoomout_button");
	if (!button)
		throw std::runtime_error("No \"icon_zoomout_button\" object in " + resource_file_);

	icon_zoomout_connection_.disconnect();
	icon_zoomout_connection_ = button->signal_clicked().connect(
			sigc::mem_fun(*this, &GPX2VideoBarShapeSettingsBox::on_icon_zoomout_clicked));

	// Connect icon button
	button = ref_builder_->get_widget<Gtk::Button>("icon_ok_button");
	if (!button)
		throw std::runtime_error("No \"icon_ok_button\" object in " + resource_file_);

	icon_ok_connection_.disconnect();
	icon_ok_connection_ = button->signal_clicked().connect(
			sigc::mem_fun(*this, &GPX2VideoBarShapeSettingsBox::on_icon_ok_clicked));
}


void GPX2VideoBarShapeSettingsBox::on_icon_ok_clicked(void) {
	log_call();

	bool use_default;

	Gtk::CheckButton *checkbutton;

	Glib::RefPtr<const GPX2VideoBarShapeSettingsBox::Icon> icon;

	// Use default icon 
	checkbutton = ref_builder_->get_widget<Gtk::CheckButton>("icon_usedefault_checkbutton");
	if (!checkbutton)
		throw std::runtime_error("No \"icon_usedefault_checkbutton\" object in " + resource_file_);

	use_default = checkbutton->get_active();

	// Get icon selected
	icon = std::dynamic_pointer_cast<GPX2VideoBarShapeSettingsBox::Icon>(icon_model_->get_selected_item());

	if (icon != NULL) {
		if (use_default) {
			log_notice("Widget %s: icon changed to 'default'", 
					widget_->name().c_str());

			widget_->theme().setIcon(VideoWidget::Theme::IconDefault);
			widget_->theme().setIconFile("");
		}
		else {
			std::string filename = (icon->icon() == VideoWidget::Theme::IconUserFile) ? icon->filename() : "";

			log_notice("Widget %s: icon changed to '%s%s'", 
					widget_->name().c_str(), VideoWidget::icon2string(icon->icon()).c_str(), filename.c_str());

			widget_->theme().setIcon(icon->icon());
			widget_->theme().setIconFile(filename);
		}

		// Update
		update_content();

		// Broadcast widget change
		widget_->dispatchEvent();
	}

	// Hide icon popover
	auto popover = ref_builder_->get_widget<Gtk::Popover>("icon_popover");
	if (!popover)
		throw std::runtime_error("No \"icon_popover\" object in " + resource_file_);

	popover->popdown();
}


void GPX2VideoBarShapeSettingsBox::on_icon_usedefault_toggled(void) {
	log_call();

	Gtk::GridView *gridview;
	Gtk::CheckButton *checkbutton;

	// Widget icon checkbutton
	checkbutton = ref_builder_->get_widget<Gtk::CheckButton>("icon_usedefault_checkbutton");
	if (!checkbutton)
		throw std::runtime_error("No \"icon_usedefault_checkbutton\" object in " + resource_file_);

	// Widget icon gridview
	gridview = ref_builder_->get_widget<Gtk::GridView>("icon_gridview");
	if (!gridview)
		throw std::runtime_error("No \"icon_gridview\" object in " + resource_file_);

	gridview->set_sensitive(!checkbutton->get_active());
}


void GPX2VideoBarShapeSettingsBox::on_icon_zoomin_clicked(void) {
	log_call();

	Gtk::GridView *gridview;

	// Increase zoom
	icon_pixel_size_ += 8;

	if (icon_pixel_size_ > icon_pixel_maxsize_)
		icon_pixel_size_ = icon_pixel_maxsize_;

	log_info("New icon size: %d", icon_pixel_size_);

	// Widget icon gridview
	gridview = ref_builder_->get_widget<Gtk::GridView>("icon_gridview");
	if (!gridview)
		throw std::runtime_error("No \"icon_gridview\" object in " + resource_file_);

	gridview->set_factory(NULL);
	gridview->set_factory(icon_factory_);

	// Apply limit
	update_boundaries();
}


void GPX2VideoBarShapeSettingsBox::on_icon_zoomout_clicked(void) {
	log_call();

	Gtk::GridView *gridview;

	// Decrease zoom
	icon_pixel_size_ -= 8;

	if (icon_pixel_size_ < icon_pixel_minsize_)
		icon_pixel_size_ = icon_pixel_minsize_;

	log_info("New icon size: %d", icon_pixel_size_);

	// Widget icon gridview
	gridview = ref_builder_->get_widget<Gtk::GridView>("icon_gridview");
	if (!gridview)
		throw std::runtime_error("No \"icon_gridview\" object in " + resource_file_);

	gridview->set_factory(NULL);
	gridview->set_factory(icon_factory_);

	// Apply limit
	update_boundaries();
}


void GPX2VideoBarShapeSettingsBox::on_model_changed(guint position, guint removed, guint added) {
	log_call();

	(void) position;
	(void) removed;
	(void) added;

	// Load models
	load_models();
}

