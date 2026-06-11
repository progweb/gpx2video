#include <glibmm/i18n.h>

#include <gtkmm/image.h>
#include <gtkmm/gridview.h>
#include <gtkmm/menubutton.h>

#include "../log_i.h"
#include "text.h"


GPX2VideoTextShapeSettingsBox::GPX2VideoTextShapeSettingsBox(BaseObjectType *cobject,
	const Glib::RefPtr<Gtk::Builder> &ref_builder, std::string resource_file, 
	GPX2VideoWidget *widget, const Glib::RefPtr<GPX2VideoMediaListStore> &media_model) 
	: GPX2VideoShapeBaseSettingsBox(cobject, ref_builder, "GPX2VideoTextShapeSettingsBox", resource_file, widget) 
	, media_model_(media_model) 
	, icon_model_(NULL) {
	log_call();

	loading_ = false;

	// Default icon size
	icon_pixel_size_ = 32;
	icon_pixel_minsize_ = 16;
	icon_pixel_maxsize_ = 96;

	// Populate models
	load_models();

	// Binding
	bind_content();
}


void GPX2VideoTextShapeSettingsBox::load_models(void) {
	log_call();

	auto model = Gio::ListStore<GPX2VideoTextShapeSettingsBox::Icon>::create();

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
		if (!item)
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
		auto item = std::dynamic_pointer_cast<GPX2VideoTextShapeSettingsBox::Icon>(list_item->get_item());

		if (image && item) {
			image->set_pixel_size(icon_pixel_size_);
			image->set(item->filename());
		}
	});
}


void GPX2VideoTextShapeSettingsBox::bind_content(void) {
	log_call();

	Gtk::Switch *sw;
//	Gtk::ComboBox *combobox;
	Gtk::MenuButton *menubutton;
	Gtk::SpinButton *spinbutton;
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

	// Icon menubutton
	menubutton = ref_builder_->get_widget<Gtk::MenuButton>("icon_menubutton");
	if (!menubutton)
		throw std::runtime_error("No \"icon_menubutton\" object in " + resource_file_);

	menubutton->set_create_popup_func(sigc::bind(
			sigc::mem_fun(*this, &GPX2VideoTextShapeSettingsBox::create_popover), menubutton));

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

	// Icon model
	media_model_->signal_items_changed().connect(sigc::mem_fun(*this, &GPX2VideoTextShapeSettingsBox::on_model_changed));
}


void GPX2VideoTextShapeSettingsBox::update_content(void) {
	log_call();

	Gdk::RGBA rgba;

	const float *color;

	Gtk::Switch *sw;
	Gtk::Image *image;
//	Gtk::ComboBox *combobox;
	Gtk::SpinButton *spinbutton;
	Gtk::ColorButton *colorbutton;

//	Gtk::TreeModel::iterator iter;

	// Mask value changed
	loading_ = true;

	// Widget icon enable switch
	sw = ref_builder_->get_widget<Gtk::Switch>("icon_enable_switch");
	if (!sw)
		throw std::runtime_error("No \"icon_enable_switch\" object in " + resource_file_);

	sw->set_active(widget_->theme().hasFlag(VideoWidget::Theme::FlagIcon));

	// Widget icon image
	image = ref_builder_->get_widget<Gtk::Image>("icon_image");
	if (!image)
		throw std::runtime_error("No \"icon_image\" object in " + resource_file_);

	image->set_pixel_size(24);
	image->set(widget_->widget()->getIconFilename(widget_->theme().icon()));

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

	// Apply limit
	update_boundaries();
}


void GPX2VideoTextShapeSettingsBox::update_boundaries(void) {
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


void GPX2VideoTextShapeSettingsBox::create_popover(Gtk::MenuButton *menubutton) {
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
			auto item = std::dynamic_pointer_cast<GPX2VideoTextShapeSettingsBox::Icon>(model->get_object(i));
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

	// Icon check button
	checkbutton = ref_builder_->get_widget<Gtk::CheckButton>("icon_usedefault_checkbutton");
	if (!checkbutton)
		throw std::runtime_error("No \"icon_usedefault_checkbutton\" object in " + resource_file_);

	checkbutton->set_active((widget_->theme().icon() == VideoWidget::Theme::IconDefault));

	// Connect icon checkbutton
	icon_checkbutton_connection_.disconnect();
	icon_checkbutton_connection_ = checkbutton->signal_toggled().connect(
			sigc::mem_fun(*this, &GPX2VideoTextShapeSettingsBox::on_icon_usedefault_toggled));

	// Connect icon zoomin button
	button = ref_builder_->get_widget<Gtk::Button>("icon_zoomin_button");
	if (!button)
		throw std::runtime_error("No \"icon_zoomin_button\" object in " + resource_file_);

	icon_zoomin_connection_.disconnect();
	icon_zoomin_connection_ = button->signal_clicked().connect(
			sigc::mem_fun(*this, &GPX2VideoTextShapeSettingsBox::on_icon_zoomin_clicked));

	// Connect icon zoomout button
	button = ref_builder_->get_widget<Gtk::Button>("icon_zoomout_button");
	if (!button)
		throw std::runtime_error("No \"icon_zoomout_button\" object in " + resource_file_);

	icon_zoomout_connection_.disconnect();
	icon_zoomout_connection_ = button->signal_clicked().connect(
			sigc::mem_fun(*this, &GPX2VideoTextShapeSettingsBox::on_icon_zoomout_clicked));

	// Connect icon button
	button = ref_builder_->get_widget<Gtk::Button>("icon_ok_button");
	if (!button)
		throw std::runtime_error("No \"icon_ok_button\" object in " + resource_file_);

	icon_ok_connection_.disconnect();
	icon_ok_connection_ = button->signal_clicked().connect(
			sigc::mem_fun(*this, &GPX2VideoTextShapeSettingsBox::on_icon_ok_clicked));
}


void GPX2VideoTextShapeSettingsBox::on_icon_ok_clicked(void) {
	log_call();

	bool use_default;

	Gtk::CheckButton *checkbutton;

	Glib::RefPtr<const GPX2VideoTextShapeSettingsBox::Icon> icon;

	// Use default icon 
	checkbutton = ref_builder_->get_widget<Gtk::CheckButton>("icon_usedefault_checkbutton");
	if (!checkbutton)
		throw std::runtime_error("No \"icon_usedefault_checkbutton\" object in " + resource_file_);

	use_default = checkbutton->get_active();

	// Get icon selected
	icon = std::dynamic_pointer_cast<GPX2VideoTextShapeSettingsBox::Icon>(icon_model_->get_selected_item());

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


void GPX2VideoTextShapeSettingsBox::on_icon_usedefault_toggled(void) {
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


void GPX2VideoTextShapeSettingsBox::on_icon_zoomin_clicked(void) {
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


void GPX2VideoTextShapeSettingsBox::on_icon_zoomout_clicked(void) {
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


void GPX2VideoTextShapeSettingsBox::on_model_changed(guint position, guint removed, guint added) {
	log_call();

	(void) position;
	(void) removed;
	(void) added;

	// Load models
	load_models();
}

