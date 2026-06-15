#include <glibmm/i18n.h>

#include <gtkmm/image.h>
#include <gtkmm/gridview.h>
#include <gtkmm/menubutton.h>

#include "../log_i.h"
#include "../../src/videowidget.h"
#include "image.h"


GPX2VideoImageWidgetSettingsBox::GPX2VideoImageWidgetSettingsBox(BaseObjectType *cobject,
	const Glib::RefPtr<Gtk::Builder> &ref_builder, std::string resource_file,
	GPX2VideoWidget *widget, const Glib::RefPtr<GPX2VideoMediaListStore> &media_model) 
	: GPX2VideoWidgetBaseSettingsBox(cobject, ref_builder, "GPX2VideotWidgetSettingsBox", resource_file, widget)
	, media_model_(media_model) { 
	log_call();

	// Default image size 64, 128, 192, 256
	image_pixel_step_ = 64;
	image_pixel_size_ = 64;
	image_pixel_minsize_ = 64;
	image_pixel_maxsize_ = 256;

	// Populate models
	load_models();

	// Binding
	bind_content();

	// Update ui
	update_content();
	update_boundaries();
}


void GPX2VideoImageWidgetSettingsBox::load_models(void) {
	log_call();

	auto model = Gio::ListStore<GPX2VideoImageWidgetSettingsBox::Image>::create();

	// User image
	guint n_items = media_model_->get_n_items();

	for (guint i=0; i < n_items; i++) {
		auto item = media_model_->get_item(i);
		if (!item || (item->media() != GPX2VideoMedia::MediaImage))
			continue;

		model->append(Image::create(
				item->filename()
			)
		);
	}

	// Wrap store in a SingleSelection model
	image_model_ = Gtk::SingleSelection::create(model);

	// Create a factory to build and bind list images
	image_factory_ = Gtk::SignalListItemFactory::create();

	image_factory_->signal_setup().connect([this] (const Glib::RefPtr<Gtk::ListItem>& list_item) {
		auto image = Gtk::make_managed<Gtk::Image>();

		image->set_pixel_size(image_pixel_size_);

		list_item->set_child(*image);
	});

	image_factory_->signal_bind().connect([this] (const Glib::RefPtr<Gtk::ListItem>& list_item) {
		auto image = dynamic_cast<Gtk::Image*>(list_item->get_child());
		auto item = std::dynamic_pointer_cast<GPX2VideoImageWidgetSettingsBox::Image>(list_item->get_item());

		if (image && item) {
			image->set_pixel_size(image_pixel_size_);
			image->set(item->filename());
		}
	});

	// zoom
	zoom_model_ = Gtk::ListStore::create(model_);

	{
		auto iter = zoom_model_->append();
		auto row = *iter;
		row[model_.m_id] = VideoWidget::ZoomNone;
		row[model_.m_name] = _("None");
		row[model_.m_enable] = true;

		row = *(zoom_model_->append());
		row[model_.m_id] = VideoWidget::ZoomFit;
		row[model_.m_name] = _("Fit");
		row[model_.m_enable] = true;

		row = *(zoom_model_->append());
		row[model_.m_id] = VideoWidget::ZoomFill;
		row[model_.m_name] = _("Fill");
		row[model_.m_enable] = true;

		row = *(zoom_model_->append());
		row[model_.m_id] = VideoWidget::ZoomStretch;
		row[model_.m_name] = _("Stretch");
		row[model_.m_enable] = true;
	}

}


void GPX2VideoImageWidgetSettingsBox::bind_content(void) {
	log_call();

	Gtk::ComboBox *combobox;
	Gtk::MenuButton *menubutton;

	auto renderer = Gtk::make_managed<Gtk::CellRendererText>();

	// Connect widgets button
	//------------------------
	
	// image menubutton
	menubutton = ref_builder_->get_widget<Gtk::MenuButton>("image_menubutton");
	if (!menubutton)
		throw std::runtime_error("No \"image_menubutton\" object in " + resource_file_);

	menubutton->set_create_popup_func(sigc::bind(
			sigc::mem_fun(*this, &GPX2VideoImageWidgetSettingsBox::create_popover), menubutton));

	// zoom
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("zoom_combobox");
	if (!combobox)
		throw std::runtime_error("No \"zoom_combobox\" object in " + resource_file_);

	combobox->pack_start(*renderer, true);
	combobox->add_attribute(renderer->property_text(), model_.m_name);
	combobox->add_attribute(renderer->property_sensitive(), model_.m_enable);

	combobox->signal_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoImageWidgetSettingsBox::on_widget_combobox_changed), combobox, 
					[this](const Gtk::TreeModel::const_iterator &iter) {
						int value = iter->get_value(model_.m_id);

						log_notice("Widget %s: zoom changed to '%s'", 
								widget_->widget()->name().c_str(), iter->get_value(model_.m_name).c_str());

						widget_->widget()->setZoom((VideoWidget::Zoom) value);

						// Broadcast widget change
						widget_->dispatchEvent(true);
					}
			));

	// Image model
	media_model_->signal_items_changed().connect(sigc::mem_fun(*this, &GPX2VideoImageWidgetSettingsBox::on_model_changed));
}


void GPX2VideoImageWidgetSettingsBox::update_content(void) {
	log_call();

	Gtk::Image *image;
	Gtk::ComboBox *combobox;

	Gtk::TreeModel::iterator iter;

	// Mask value changed
	loading_ = true;

	// image
	image = ref_builder_->get_widget<Gtk::Image>("image_image");
	if (!image)
		throw std::runtime_error("No \"image_image\" object in " + resource_file_);

	image->set_pixel_size(128);
	image->set(widget_->widget()->source());

	// zoom
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("zoom_combobox");
	if (!combobox)
		throw std::runtime_error("No \"zoom_combobox\" object in " + resource_file_);

	combobox->set_model(zoom_model_);

	if (find_in_listtore(zoom_model_, widget_->widget()->zoom(), iter))
		combobox->set_active(iter);

	// Unmask value changed
	loading_ = false;
}


void GPX2VideoImageWidgetSettingsBox::update_boundaries(void) {
	log_call();

	Gtk::Button *button;

	// Icon zoomin button
	button = ref_builder_->get_widget<Gtk::Button>("image_zoomin_button");
	if (!button)
		throw std::runtime_error("No \"image_zoomin_button\" object in " + resource_file_);

	button->set_sensitive(image_pixel_size_ < image_pixel_maxsize_);

	// Icon zoomout button
	button = ref_builder_->get_widget<Gtk::Button>("image_zoomout_button");
	if (!button)
		throw std::runtime_error("No \"image_zoomout_button\" object in " + resource_file_);

	button->set_sensitive(image_pixel_size_ > image_pixel_minsize_);
}


void GPX2VideoImageWidgetSettingsBox::create_popover(Gtk::MenuButton *menubutton) {
	log_call();

	int index;

	Gtk::Button *button;
	Gtk::Popover *popover;
	Gtk::GridView *gridview;

	// Widget image model
	index = -1;

	{
		guint n_items;

		auto model = image_model_->get_model();

		n_items = model->get_n_items();

		for (guint i=0; i < n_items; i++) {
			auto item = std::dynamic_pointer_cast<GPX2VideoImageWidgetSettingsBox::Image>(model->get_object(i));
			if (!item)
				continue;

			if (item->filename() != widget_->widget()->source())
				continue;

			index = i;
			break;
		}

		// Image not found!
		if ((index == -1) && !widget_->widget()->source().empty()) {
			// Auto append
			media_model_->append(GPX2VideoMedia::MediaImage, widget_->widget()->source());

			// Select
			index = n_items;
		}
	}

	image_model_->set_selected(index);

	// Image popover
	popover = ref_builder_->get_widget<Gtk::Popover>("image_popover");
	if (!popover)
		throw std::runtime_error("No \"image_popover\" object in " + resource_file_);

	menubutton->set_popover(*popover);

	// Icon gridview
	gridview = ref_builder_->get_widget<Gtk::GridView>("image_gridview");
	if (!gridview)
		throw std::runtime_error("No \"image_gridview\" object in " + resource_file_);

	gridview->set_model(image_model_);
	gridview->set_factory(image_factory_);
	gridview->set_min_columns(1);
	gridview->set_max_columns(4);

	// Connect image zoomin button
	button = ref_builder_->get_widget<Gtk::Button>("image_zoomin_button");
	if (!button)
		throw std::runtime_error("No \"image_zoomin_button\" object in " + resource_file_);

	image_zoomin_connection_.disconnect();
	image_zoomin_connection_ = button->signal_clicked().connect(
			sigc::mem_fun(*this, &GPX2VideoImageWidgetSettingsBox::on_image_zoomin_clicked));

	// Connect image zoomout button
	button = ref_builder_->get_widget<Gtk::Button>("image_zoomout_button");
	if (!button)
		throw std::runtime_error("No \"image_zoomout_button\" object in " + resource_file_);

	image_zoomout_connection_.disconnect();
	image_zoomout_connection_ = button->signal_clicked().connect(
			sigc::mem_fun(*this, &GPX2VideoImageWidgetSettingsBox::on_image_zoomout_clicked));

	// Connect icon button
	button = ref_builder_->get_widget<Gtk::Button>("image_ok_button");
	if (!button)
		throw std::runtime_error("No \"image_ok_button\" object in " + resource_file_);

	image_ok_connection_.disconnect();
	image_ok_connection_ = button->signal_clicked().connect(
			sigc::mem_fun(*this, &GPX2VideoImageWidgetSettingsBox::on_image_ok_clicked));
}


void GPX2VideoImageWidgetSettingsBox::on_image_zoomin_clicked(void) {
	log_call();

	Gtk::GridView *gridview;

	// Increase zoom
	image_pixel_size_ += image_pixel_step_;

	if (image_pixel_size_ > image_pixel_maxsize_)
		image_pixel_size_ = image_pixel_maxsize_;

	log_info("New image size: %d", image_pixel_size_);

	// Widget image gridview
	gridview = ref_builder_->get_widget<Gtk::GridView>("image_gridview");
	if (!gridview)
		throw std::runtime_error("No \"image_gridview\" object in " + resource_file_);

	gridview->set_factory(NULL);
	gridview->set_factory(image_factory_);

	// Apply limit
	update_boundaries();
}


void GPX2VideoImageWidgetSettingsBox::on_image_ok_clicked(void) {
	log_call();

	Glib::RefPtr<const GPX2VideoImageWidgetSettingsBox::Image> image;

	// Get image selected
	image = std::dynamic_pointer_cast<GPX2VideoImageWidgetSettingsBox::Image>(image_model_->get_selected_item());

	if (image != NULL) {
		std::string filename = image->filename();

		log_notice("Widget %s: image changed to '%s'", 
				widget_->name().c_str(), filename.c_str());

		widget_->widget()->setSource(filename);

		// Update
		update_content();

		// Broadcast widget change
		widget_->dispatchEvent(true);
	}

	// Hide image popover
	auto popover = ref_builder_->get_widget<Gtk::Popover>("image_popover");
	if (!popover)
		throw std::runtime_error("No \"image_popover\" object in " + resource_file_);

	popover->popdown();
}


void GPX2VideoImageWidgetSettingsBox::on_image_zoomout_clicked(void) {
	log_call();

	Gtk::GridView *gridview;

	// Decrease zoom
	image_pixel_size_ -= image_pixel_step_;

	if (image_pixel_size_ < image_pixel_minsize_)
		image_pixel_size_ = image_pixel_minsize_;

	log_info("New image size: %d", image_pixel_size_);

	// Widget image gridview
	gridview = ref_builder_->get_widget<Gtk::GridView>("image_gridview");
	if (!gridview)
		throw std::runtime_error("No \"image_gridview\" object in " + resource_file_);

	gridview->set_factory(NULL);
	gridview->set_factory(image_factory_);

	// Apply limit
	update_boundaries();
}


void GPX2VideoImageWidgetSettingsBox::on_model_changed(guint position, guint removed, guint added) {
	log_call();

	(void) position;
	(void) removed;
	(void) added;

	// Load models
	load_models();
}

