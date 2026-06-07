#include <gtkmm/image.h>
#include <gtkmm/gridview.h>
#include <gtkmm/menubutton.h>

#include "../log_i.h"
#include "text.h"


GPX2VideoTextShapeSettingsBox::GPX2VideoTextShapeSettingsBox(BaseObjectType *cobject,
	const Glib::RefPtr<Gtk::Builder> &ref_builder, std::string resource_file, GPX2VideoWidget *widget) 
	: GPX2VideoShapeBaseSettingsBox(cobject, ref_builder, "GPX2VideoTextShapeSettingsBox", resource_file, widget) {
	log_call();

	loading_ = false;

	// Populate models
	//-----------------

	auto model = Gio::ListStore<GPX2VideoTextShapeSettingsBox::Icon>::create();

	for (int i=0; i != VideoWidget::Theme::IconUnknown; i++) {
		if (i != VideoWidget::Theme::IconDefault) {
			VideoWidget::Theme::Icon icon = (VideoWidget::Theme::Icon) i;

			std::string filename = widget_->widget()->getIconFilename(icon);

			model->append(Icon::create(
					icon,
					filename	
				)
			);
		}
	}

	// Wrap store in a SingleSelection model
	icon_model_ = Gtk::SingleSelection::create(model);

	// Binding
	bind_content();
}


void GPX2VideoTextShapeSettingsBox::bind_content(void) {
	log_call();

	Gtk::Switch *sw;
	Gtk::Button *button;
//	Gtk::ComboBox *combobox;
	Gtk::GridView *gridview;
	Gtk::SpinButton *spinbutton;
	Gtk::CheckButton *checkbutton;
	Gtk::ColorButton *colorbutton;

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

	// Create a factory to build and bind list items
	auto factory = Gtk::SignalListItemFactory::create();

	factory->signal_setup().connect([](const Glib::RefPtr<Gtk::ListItem>& list_item) {
		auto image = Gtk::make_managed<Gtk::Image>();

		image->set_pixel_size(32);

		list_item->set_child(*image);
	});

	factory->signal_bind().connect([this] (const Glib::RefPtr<Gtk::ListItem>& list_item) {
		auto image = dynamic_cast<Gtk::Image*>(list_item->get_child());
		auto item = std::dynamic_pointer_cast<GPX2VideoTextShapeSettingsBox::Icon>(list_item->get_item());

		if (image && item)
			image->set(item->filename());
	});

	// Icon gridview
	gridview = ref_builder_->get_widget<Gtk::GridView>("icon_gridview");
	if (!gridview)
		throw std::runtime_error("No \"icon_gridview\" object in " + resource_file_);

	gridview->set_model(icon_model_);
	gridview->set_factory(factory);
	gridview->set_min_columns(6);
	gridview->set_max_columns(10);

	// Connect icon checkbutton
	checkbutton = ref_builder_->get_widget<Gtk::CheckButton>("icon_checkbutton");
	if (!checkbutton)
		throw std::runtime_error("No \"icon_checkbutton\" object in " + resource_file_);

	checkbutton->signal_toggled().connect(sigc::mem_fun(*this, &GPX2VideoTextShapeSettingsBox::on_icon_toggled));

	// Connect icon button
	button = ref_builder_->get_widget<Gtk::Button>("icon_button");
	if (!button)
		throw std::runtime_error("No \"icon_button\" object in " + resource_file_);

	button->signal_clicked().connect(sigc::mem_fun(*this, &GPX2VideoTextShapeSettingsBox::on_icon_clicked));

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

	// Misc. linespace
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("linespace_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"linespace_spinbutton\" object in " + resource_file_);
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoTextShapeSettingsBox::on_widget_spin_changed), spinbutton, 
					[this](const int &value) {
						log_notice("Widget %s: linespace changed to '%d'",
							   widget_->name().c_str(), value);

						widget_->theme().setLineSpace(value);

						// Broadcast widget change
						widget_->dispatchEvent();
					}
			));
}


void GPX2VideoTextShapeSettingsBox::update_content(void) {
	log_call();

	int index;

	Gdk::RGBA rgba;

	const float *color;

	Gtk::Switch *sw;
	Gtk::Image *image;
//	Gtk::ComboBox *combobox;
	Gtk::GridView *gridview;
	Gtk::SpinButton *spinbutton;
	Gtk::CheckButton *checkbutton;
	Gtk::ColorButton *colorbutton;

//	Gtk::TreeModel::iterator iter;

	// Mask value changed
	loading_ = true;

	// Widget icon model
	index = widget_->theme().icon();
	if (index == VideoWidget::Theme::IconDefault)
		index = widget_->widget()->type();

	icon_model_->set_selected(index);

	// Widget icon enable switch
	sw = ref_builder_->get_widget<Gtk::Switch>("icon_enable_switch");
	if (!sw)
		throw std::runtime_error("No \"icon_enable_switch\" object in " + resource_file_);

	sw->set_active(widget_->theme().hasFlag(VideoWidget::Theme::FlagIcon));

	// Widget icon button
	image = ref_builder_->get_widget<Gtk::Image>("icon_image");
	if (!image)
		throw std::runtime_error("No \"icon_image\" object in " + resource_file_);

	image->set_pixel_size(24);
	image->set(widget_->widget()->getIconFilename(widget_->theme().icon()));

	// Widget icon gridview
	gridview = ref_builder_->get_widget<Gtk::GridView>("icon_gridview");
	if (!gridview)
		throw std::runtime_error("No \"icon_gridview\" object in " + resource_file_);

	gridview->set_sensitive((widget_->theme().icon() != VideoWidget::Theme::IconDefault));

	// Widget icon checkbutton
	checkbutton = ref_builder_->get_widget<Gtk::CheckButton>("icon_checkbutton");
	if (!checkbutton)
		throw std::runtime_error("No \"icon_checkbutton\" object in " + resource_file_);

	checkbutton->set_active((widget_->theme().icon() == VideoWidget::Theme::IconDefault));

	// Widget icon color button
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("icon_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"icon_color_button\" object in " + resource_file_);

	color = widget_->theme().iconColor();

	rgba.set_rgba(color[0], color[1], color[2], color[3]);

	colorbutton->set_rgba(rgba);

	// Widget linespace button
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("linespace_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"linespace_spinbutton\" object in " + resource_file_);

	spinbutton->set_value(widget_->theme().lineSpace());

	// Unmask value changed
	loading_ = false;
}


void GPX2VideoTextShapeSettingsBox::on_icon_clicked(void) {
	log_call();

	bool use_default;

	Gtk::CheckButton *checkbutton;

	Glib::RefPtr<const GPX2VideoTextShapeSettingsBox::Icon> icon;

	// Use default icon 
	checkbutton = ref_builder_->get_widget<Gtk::CheckButton>("icon_checkbutton");
	if (!checkbutton)
		throw std::runtime_error("No \"icon_checkbutton\" object in " + resource_file_);

	use_default = checkbutton->get_active();

	// Get icon selected
	icon = std::dynamic_pointer_cast<GPX2VideoTextShapeSettingsBox::Icon>(icon_model_->get_selected_item());

	if (icon != NULL) {
		if (use_default) {
			log_notice("Widget %s: icon changed to 'default'", 
					widget_->name().c_str());

			widget_->theme().setIcon(VideoWidget::Theme::IconDefault);
		}
		else {
			log_notice("Widget %s: icon changed to '%s'", 
					widget_->name().c_str(), VideoWidget::icon2string(icon->icon()).c_str());

			widget_->theme().setIcon(icon->icon());
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


void GPX2VideoTextShapeSettingsBox::on_icon_toggled(void) {
	log_call();

	Gtk::GridView *gridview;
	Gtk::CheckButton *checkbutton;

	// Widget icon checkbutton
	checkbutton = ref_builder_->get_widget<Gtk::CheckButton>("icon_checkbutton");
	if (!checkbutton)
		throw std::runtime_error("No \"icon_checkbutton\" object in " + resource_file_);

	// Widget icon gridview
	gridview = ref_builder_->get_widget<Gtk::GridView>("icon_gridview");
	if (!gridview)
		throw std::runtime_error("No \"icon_gridview\" object in " + resource_file_);

	gridview->set_sensitive(!checkbutton->get_active());
}

