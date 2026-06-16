#include <glibmm/i18n.h>

#include <gtkmm/popover.h>

#include "../log_i.h"
#include "map.h"


void GPX2VideoMapWidgetSettingsBox::Icon::load_and_crop_svg(void) {
	GError *error = nullptr;

	// Load svg
	RsvgHandle *handle = rsvg_handle_new_from_file(filename_.c_str(), &error);
	if (!handle) {
		log_error("Load svg '%s' file error: %s", 
				filename_.c_str(), 
				error ? error->message : "unknown error");

		if (error)
			g_error_free(error);

		return;
	}

    // svg dimensions
	double svg_width, svg_height;
	rsvg_handle_get_intrinsic_size_in_pixels(handle, &svg_width, &svg_height);

	// svg viewport
	RsvgRectangle viewport = {
		.x = 0,
		.y = 0,
		.width = svg_width,
		.height = svg_height
	};

	// Get bounding box of the element
	RsvgRectangle bbox;
	if (!rsvg_handle_get_geometry_for_layer(handle, nullptr, &viewport, &bbox, nullptr, &error)) {
		log_error("Get svg '%s' geometry error: %s", 
				filename_.c_str(), 
				error ? error->message : "unknown error");

		if (error) 
			g_error_free(error);
		
		g_object_unref(handle);
	
		return;
	}

	// Create Cairo surface for cropped area
	int width = static_cast<int>(bbox.width);
	int height = static_cast<int>(bbox.height);

	if ((width == 0) || (height == 0)) {
		pixbuf_ = Gdk::Pixbuf::create_from_file(filename_);
		return;
	}

	auto surface = Cairo::ImageSurface::create(Cairo::Surface::Format::ARGB32, width, height);
	auto cairo = Cairo::Context::create(surface);

	// Translate so that the element is at (0,0)
	cairo->translate(-bbox.x, -bbox.y);

	// Render whole svg
	if (!rsvg_handle_render_document(handle, cairo->cobj(), &viewport, &error)) {
		log_error("Render svg '%s' error: %s", 
				filename_.c_str(), 
				error ? error->message : "unknown error");

		if (error) 
			g_error_free(error);

		g_object_unref(handle);

		return;
	}

	g_object_unref(handle);

	// Convert Cairo surface to Gdk::Pixbuf
	pixbuf_ = Gdk::Pixbuf::create(surface, 0, 0, width, height);
}


GPX2VideoMapWidgetSettingsBox::GPX2VideoMapWidgetSettingsBox(BaseObjectType *cobject,
	const Glib::RefPtr<Gtk::Builder> &ref_builder, std::string resource_file, 
	GPX2VideoWidget *widget, const Glib::RefPtr<GPX2VideoMediaListStore> &media_model) 
	: GPX2VideoWidgetBaseSettingsBox(cobject, ref_builder, "GPX2VideoMapWidgetSettingsBox", resource_file, widget) 
	, media_model_(media_model) { 
	log_call();

	// Default icon size
	icon_pixel_size_ = 40;
	icon_pixel_minsize_ = 16;
	icon_pixel_maxsize_ = 96;

	// Populate models
	load_models();

	// Binding
	bind_content();

	// Update ui
	update_content();
	update_boundaries();
}


void GPX2VideoMapWidgetSettingsBox::load_models(void) {
	log_call();

	auto model = Gio::ListStore<GPX2VideoMapWidgetSettingsBox::Icon>::create();

	// Internal icon
	for (int i=0; i != MapSettings::IconUnknown; i++) {
		if (i != MapSettings::IconUserFile) {
			MapSettings::Icon icon = (MapSettings::Icon) i;

			std::string filename = ((Map *) widget_->widget())->getIconFilename(icon);
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
				MapSettings::IconUserFile,
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
		auto item = std::dynamic_pointer_cast<GPX2VideoMapWidgetSettingsBox::Icon>(list_item->get_item());

		if (image && item) {
			image->set_pixel_size(icon_pixel_size_);
			image->set(item->pixbuf());
		}
	});

	// Map view model
	view_model_ = Gtk::ListStore::create(model_);

	{
		auto iter = view_model_->append();
		auto row = *iter;
		row[model_.m_id] = MapSettings::ViewDefault;
		row[model_.m_name] = _("Default");
		row[model_.m_enable] = true;

		row = *(view_model_->append());
		row[model_.m_id] = MapSettings::ViewLockCenter;
		row[model_.m_name] = _("Center");
		row[model_.m_enable] = true;

		row = *(view_model_->append());
		row[model_.m_id] = MapSettings::ViewZoomFit;
		row[model_.m_name] = _("Zoom fit");
		row[model_.m_enable] = true;
	}

	// Map source model
	source_model_ = Gtk::ListStore::create(model_);

	for (int i=MapSettings::SourceNull; i != MapSettings::SourceCount; i++) {
		std::string name = MapSettings::getFriendlyName((MapSettings::Source) i);
		std::string uri = MapSettings::getRepoURI((MapSettings::Source) i);

		if (i == MapSettings::SourceNull)
			continue;

		if (uri == "")
			continue;

		auto row = *(source_model_->append());
		row[model_.m_id] = i;
		row[model_.m_name] = name;
		row[model_.m_enable] = true;
	}
}


void GPX2VideoMapWidgetSettingsBox::bind_content(void) {
	log_call();

	Gtk::Switch *sw;
	Gtk::ComboBox *combobox;
	Gtk::MenuButton *menubutton;
	Gtk::SpinButton *spinbutton;
	Gtk::ColorButton *colorbutton;

	auto renderer = Gtk::make_managed<Gtk::CellRendererText>();

	// Connect widgets button
	//------------------------

	// Source
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("source_combobox");
	if (!combobox)
		throw std::runtime_error("No \"source_combobox\" object in" + resource_file_);

	combobox->pack_start(*renderer, true);
	combobox->add_attribute(renderer->property_text(), model_.m_name);
	combobox->add_attribute(renderer->property_sensitive(), model_.m_enable);

	combobox->signal_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoMapWidgetSettingsBox::on_widget_combobox_changed), combobox, 
					[this](const Gtk::TreeModel::const_iterator &iter) {
						int value = iter->get_value(model_.m_id);

						log_notice("Widget %s: source changed to '%s'", 
								widget_->widget()->name().c_str(), iter->get_value(model_.m_name).c_str());

						((Map *) widget_->widget())->settings().setSource((MapSettings::Source) value);

						// Update
						update_boundaries();

						// Broadcast widget change
						widget_->dispatchEvent(true);
					}
			));

	// View
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("view_combobox");
	if (!combobox)
		throw std::runtime_error("No \"view_combobox\" object in" + resource_file_);

	combobox->pack_start(*renderer, true);
	combobox->add_attribute(renderer->property_text(), model_.m_name);
	combobox->add_attribute(renderer->property_sensitive(), model_.m_enable);

	combobox->signal_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoMapWidgetSettingsBox::on_widget_combobox_changed), combobox, 
					[this](const Gtk::TreeModel::const_iterator &iter) {
						int value = iter->get_value(model_.m_id);

						log_notice("Widget %s: view changed to '%s'", 
								widget_->widget()->name().c_str(), iter->get_value(model_.m_name).c_str());

						((Map *) widget_->widget())->settings().setView((MapSettings::View) value);

						// Update
						update_boundaries();

						// Broadcast widget change
						widget_->dispatchEvent(true);
					}
			));

	// Zoom
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("zoom_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"zoom_spinbutton\" object in " + resource_file_);
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoMapWidgetSettingsBox::on_widget_spin_int_changed), spinbutton, 
					[this](const int &value) {
						log_notice("Widget %s: zoom changed to '%d'",
							   widget_->name().c_str(), value);

						((Map *) widget_->widget())->settings().setZoom(value);

						// Broadcast widget change
						widget_->dispatchEvent(true);
					}
			));

	// Factor
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("factor_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"factor_spinbutton\" object in " + resource_file_);
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoMapWidgetSettingsBox::on_widget_spin_double_changed), spinbutton, 
					[this](const double &value) {
						log_notice("Widget %s: factor changed to '%.1f'",
							   widget_->name().c_str(), value);

						((Map *) widget_->widget())->settings().setDivider(value);

						// Broadcast widget change
						widget_->dispatchEvent(true);
					}
			));

	// Path smooth
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("path_smooth_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"path_smooth_spinbutton\" object in " + resource_file_);

	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoMapWidgetSettingsBox::on_widget_spin_int_changed), spinbutton, 
					[this](const int &value) {
						log_notice("Widget %s: path smooth changed to '%d'",
							   widget_->name().c_str(), value);

						((Map *) widget_->widget())->settings().setPathSmooth(value);

						// Broadcast widget change
						widget_->dispatchEvent(true);
					}
			));

	// Path thick
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("path_thick_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"path_thick_spinbutton\" object in " + resource_file_);

	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoMapWidgetSettingsBox::on_widget_spin_double_changed), spinbutton, 
					[this](const double &value) {
						log_notice("Widget %s: path thick changed to '%.1f'",
							   widget_->name().c_str(), value);

						((Map *) widget_->widget())->settings().setPathThick(value);

						// Broadcast widget change
						widget_->dispatchEvent(true);
					}
			));

	// Path border
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("path_border_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"path_border_spinbutton\" object in " + resource_file_);

	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoMapWidgetSettingsBox::on_widget_spin_double_changed), spinbutton, 
					[this](const double &value) {
						log_notice("Widget %s: path border changed to '%.1f'",
							   widget_->name().c_str(), value);

						((Map *) widget_->widget())->settings().setPathBorder(value);

						// Broadcast widget change
						widget_->dispatchEvent(true);
					}
			));

	// Path border color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("path_border_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"path_border_color_button\" object in " + resource_file_);

	colorbutton->signal_color_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoMapWidgetSettingsBox::on_widget_color_changed), colorbutton, 
					[this](const std::string &color) {
						log_notice("Widget %s: path border color changed to '%s'", 
								widget_->name().c_str(), color.c_str());

						((Map *) widget_->widget())->settings().setPathBorderColor(color);

						// Broadcast widget change
						widget_->dispatchEvent(true);
					}
			));

	// Path primary color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("path_primary_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"path_primary_color_button\" object in " + resource_file_);

	colorbutton->signal_color_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoMapWidgetSettingsBox::on_widget_color_changed), colorbutton, 
					[this](const std::string &color) {
						log_notice("Widget %s: path primary color changed to '%s'", 
								widget_->name().c_str(), color.c_str());

						((Map *) widget_->widget())->settings().setPathPrimaryColor(color);

						// Broadcast widget change
						widget_->dispatchEvent(true);
					}
			));

	// Path secondary color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("path_secondary_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"path_secondary_color_button\" object in " + resource_file_);

	colorbutton->signal_color_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoMapWidgetSettingsBox::on_widget_color_changed), colorbutton, 
					[this](const std::string &color) {
						log_notice("Widget %s: path secondary color changed to '%s'", 
								widget_->name().c_str(), color.c_str());

						((Map *) widget_->widget())->settings().setPathSecondaryColor(color);

						// Broadcast widget change
						widget_->dispatchEvent(true);
					}
			));

	// Icon start enable
	sw = ref_builder_->get_widget<Gtk::Switch>("icon_start_enable_switch");
	if (!sw)
		throw std::runtime_error("No \"icon_start_enable_switch\" object in " + resource_file_);
	sw->signal_state_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoMapWidgetSettingsBox::on_widget_switch_changed), sw, 
					[this](const bool &state) {
						log_notice("Widget %s: icon start status changed to '%s'",
							   widget_->widget()->name().c_str(), state ? "enabled" : "disabled");

						if (state)
							widget_->widget()->theme().addFlag(VideoWidget::Theme::FlagIconStart);
						else
							widget_->widget()->theme().removeFlag(VideoWidget::Theme::FlagIconStart);

						// Broadcast widget change
						widget_->dispatchEvent(false);
					} 
			), false);

	// Icon start menubutton
	menubutton = ref_builder_->get_widget<Gtk::MenuButton>("icon_start_menubutton");
	if (!menubutton)
		throw std::runtime_error("No \"icon_start_menubutton\" object in " + resource_file_);

	menubutton->set_create_popup_func(sigc::bind(
			sigc::mem_fun(*this, &GPX2VideoMapWidgetSettingsBox::create_popover), menubutton, MapSettings::IconStart));

	// Icon start color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("icon_start_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"icon_start_color_button\" object in " + resource_file_);

	colorbutton->signal_color_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoMapWidgetSettingsBox::on_widget_color_changed), colorbutton, 
					[this](const std::string &color) {
						log_notice("Widget %s: icon start color changed to '%s'", 
								widget_->name().c_str(), color.c_str());

						((Map *) widget_->widget())->settings().setIconColor(MapSettings::IconStart, color);

						// Broadcast widget change
						widget_->dispatchEvent(true);
					}
			));

	// Icon start size
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("icon_start_size_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"icon_start_size_spinbutton\" object in " + resource_file_);
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoMapWidgetSettingsBox::on_widget_spin_double_changed), spinbutton, 
					[this](const double &value) {
						log_notice("Widget %s: icon start size changed to '%.1f'",
							   widget_->name().c_str(), value);

						((Map *) widget_->widget())->settings().setIconSize(MapSettings::IconStart, value);

						// Broadcast widget change
						widget_->dispatchEvent(true);
					}
			));

	// Icon end enable
	sw = ref_builder_->get_widget<Gtk::Switch>("icon_end_enable_switch");
	if (!sw)
		throw std::runtime_error("No \"icon_end_enable_switch\" object in " + resource_file_);
	sw->signal_state_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoMapWidgetSettingsBox::on_widget_switch_changed), sw, 
					[this](const bool &state) {
						log_notice("Widget %s: icon end status changed to '%s'",
							   widget_->widget()->name().c_str(), state ? "enabled" : "disabled");

						if (state)
							widget_->widget()->theme().addFlag(VideoWidget::Theme::FlagIconEnd);
						else
							widget_->widget()->theme().removeFlag(VideoWidget::Theme::FlagIconEnd);

						// Broadcast widget change
						widget_->dispatchEvent(false);
					} 
			), false);

	// Icon end menubutton
	menubutton = ref_builder_->get_widget<Gtk::MenuButton>("icon_end_menubutton");
	if (!menubutton)
		throw std::runtime_error("No \"icon_end_menubutton\" object in " + resource_file_);

	menubutton->set_create_popup_func(sigc::bind(
			sigc::mem_fun(*this, &GPX2VideoMapWidgetSettingsBox::create_popover), menubutton, MapSettings::IconEnd));

	// Icon end color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("icon_end_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"icon_end_color_button\" object in " + resource_file_);

	colorbutton->signal_color_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoMapWidgetSettingsBox::on_widget_color_changed), colorbutton, 
					[this](const std::string &color) {
						log_notice("Widget %s: icon end color changed to '%s'", 
								widget_->name().c_str(), color.c_str());

						((Map *) widget_->widget())->settings().setIconColor(MapSettings::IconEnd, color);

						// Broadcast widget change
						widget_->dispatchEvent(true);
					}
			));

	// Icon end size
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("icon_end_size_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"icon_end_size_spinbutton\" object in " + resource_file_);
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoMapWidgetSettingsBox::on_widget_spin_double_changed), spinbutton, 
					[this](const double &value) {
						log_notice("Widget %s: icon end size changed to '%.1f'",
							   widget_->name().c_str(), value);

						((Map *) widget_->widget())->settings().setIconSize(MapSettings::IconEnd, value);

						// Broadcast widget change
						widget_->dispatchEvent(true);
					}
			));

	// Icon position enable
	sw = ref_builder_->get_widget<Gtk::Switch>("icon_position_enable_switch");
	if (!sw)
		throw std::runtime_error("No \"icon_position_enable_switch\" object in " + resource_file_);
	sw->signal_state_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoMapWidgetSettingsBox::on_widget_switch_changed), sw, 
					[this](const bool &state) {
						log_notice("Widget %s: icon position status changed to '%s'",
							   widget_->widget()->name().c_str(), state ? "enabled" : "disabled");

						if (state)
							widget_->widget()->theme().addFlag(VideoWidget::Theme::FlagIconPosition);
						else
							widget_->widget()->theme().removeFlag(VideoWidget::Theme::FlagIconPosition);

						// Broadcast widget change
						widget_->dispatchEvent(false);
					} 
			), false);

	// Icon position menubutton
	menubutton = ref_builder_->get_widget<Gtk::MenuButton>("icon_position_menubutton");
	if (!menubutton)
		throw std::runtime_error("No \"icon_position_menubutton\" object in " + resource_file_);

	menubutton->set_create_popup_func(sigc::bind(
			sigc::mem_fun(*this, &GPX2VideoMapWidgetSettingsBox::create_popover), menubutton, MapSettings::IconPosition));

	// Icon position color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("icon_position_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"icon_position_color_button\" object in " + resource_file_);

	colorbutton->signal_color_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoMapWidgetSettingsBox::on_widget_color_changed), colorbutton, 
					[this](const std::string &color) {
						log_notice("Widget %s: icon position color changed to '%s'", 
								widget_->name().c_str(), color.c_str());

						((Map *) widget_->widget())->settings().setIconColor(MapSettings::IconPosition, color);

						// Broadcast widget change
						widget_->dispatchEvent(true);
					}
			));

	// Icon position size
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("icon_position_size_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"icon_position_size_spinbutton\" object in " + resource_file_);
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoMapWidgetSettingsBox::on_widget_spin_double_changed), spinbutton, 
					[this](const double &value) {
						log_notice("Widget %s: icon position size changed to '%.1f'",
							   widget_->name().c_str(), value);

						((Map *) widget_->widget())->settings().setIconSize(MapSettings::IconPosition, value);

						// Broadcast widget change
						widget_->dispatchEvent(true);
					}
			));

	// Icon position follow
	sw = ref_builder_->get_widget<Gtk::Switch>("icon_position_follow_switch");
	if (!sw)
		throw std::runtime_error("No \"icon_position_follow_switch\" object in " + resource_file_);
	sw->signal_state_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoMapWidgetSettingsBox::on_widget_switch_changed), sw, 
					[this](const bool &state) {
						log_notice("Widget %s: icon position follow  changed to '%s'",
							   widget_->widget()->name().c_str(), state ? "enabled" : "disabled");

						((Map *) widget_->widget())->settings().setFollowCourse(state);

						// Broadcast widget change
						widget_->dispatchEvent(false);
					} 
			), false);

	// Icon model
	media_model_->signal_items_changed().connect(sigc::mem_fun(*this, &GPX2VideoMapWidgetSettingsBox::on_model_changed));
}


void GPX2VideoMapWidgetSettingsBox::update_content(void) {
	log_call();

	Gdk::RGBA rgba;

	const float *color;

	std::string filename;

	Glib::RefPtr<GPX2VideoMapWidgetSettingsBox::Icon> icon;

	Gtk::Switch *sw;
	Gtk::Image *image;
	Gtk::ComboBox *combobox;
	Gtk::SpinButton *spinbutton;
	Gtk::ColorButton *colorbutton;

	Gtk::TreeModel::iterator iter;

	MapSettings &settings = ((Map *) widget_->widget())->settings();

	// Mask value changed
	loading_ = true;

	// source
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("source_combobox");
	if (!combobox)
		throw std::runtime_error("No \"source_combobox\" object in " + resource_file_);

	combobox->set_model(source_model_);

	if (find_in_listtore(source_model_, settings.source(), iter))
		combobox->set_active(iter);

	// view
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("view_combobox");
	if (!combobox)
		throw std::runtime_error("No \"view_combobox\" object in " + resource_file_);

	combobox->set_model(view_model_);

	if (find_in_listtore(view_model_, settings.view(), iter))
		combobox->set_active(iter);

	// zoom
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("zoom_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"zoom_spinbutton\" object in " + resource_file_);

	spinbutton->set_value(settings.zoom());

	// factor
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("factor_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"factor_spinbutton\" object in " + resource_file_);

	spinbutton->set_value(settings.divider());

	// Path smooth
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("path_smooth_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"path_smooth_spinbutton\" object in " + resource_file_);

	spinbutton->set_value(settings.pathSmooth());

	// Path thick
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("path_thick_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"path_thick_spinbutton\" object in " + resource_file_);

	spinbutton->set_value(settings.pathThick());

	// Path border
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("path_border_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"path_border_spinbutton\" object in " + resource_file_);

	spinbutton->set_value(settings.pathBorder());

	// Path border color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("path_border_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"path_border_color_button\" object in " + resource_file_);

	color = settings.pathBorderColor();

	rgba.set_rgba(color[0], color[1], color[2], color[3]);

	colorbutton->set_rgba(rgba);

	// Path primary color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("path_primary_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"path_primary_color_button\" object in " + resource_file_);

	color = settings.pathPrimaryColor();

	rgba.set_rgba(color[0], color[1], color[2], color[3]);

	colorbutton->set_rgba(rgba);

	// Path secondary color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("path_secondary_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"path_secondary_color_button\" object in " + resource_file_);

	color = settings.pathSecondaryColor();

	rgba.set_rgba(color[0], color[1], color[2], color[3]);

	colorbutton->set_rgba(rgba);

	// Icon start enable
	sw = ref_builder_->get_widget<Gtk::Switch>("icon_start_enable_switch");
	if (!sw)
		throw std::runtime_error("No \"icon_start_enable_switch\" object in " + resource_file_);

	sw->set_active(widget_->widget()->theme().hasFlag(VideoWidget::Theme::FlagIconStart));

	// Icon start image
	image = ref_builder_->get_widget<Gtk::Image>("icon_start_image");
	if (!image)
		throw std::runtime_error("No \"icon_start_image\" object in " + resource_file_);

	filename = ((Map *) widget_->widget())->getIconFilename(settings.icon(MapSettings::IconStart), MapSettings::IconStart);
	icon = this->find_icon(filename);

	image->set_pixel_size(24);
	image->set(icon ? icon->pixbuf() : NULL);

	// Icon start color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("icon_start_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"icon_start_color_button\" object in " + resource_file_);

	color = settings.iconColor(MapSettings::IconStart);

	rgba.set_rgba(color[0], color[1], color[2], color[3]);

	colorbutton->set_rgba(rgba);

	// Icon start size
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("icon_start_size_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"icon_start_size_spinbutton\" object in " + resource_file_);

	spinbutton->set_value(settings.iconSize(MapSettings::IconPosition));

	// Icon end enable
	sw = ref_builder_->get_widget<Gtk::Switch>("icon_end_enable_switch");
	if (!sw)
		throw std::runtime_error("No \"icon_end_enable_switch\" object in " + resource_file_);

	sw->set_active(widget_->widget()->theme().hasFlag(VideoWidget::Theme::FlagIconEnd));

	// Icon end image
	image = ref_builder_->get_widget<Gtk::Image>("icon_end_image");
	if (!image)
		throw std::runtime_error("No \"icon_end_image\" object in " + resource_file_);

	filename = ((Map *) widget_->widget())->getIconFilename(settings.icon(MapSettings::IconEnd), MapSettings::IconEnd);
	icon = this->find_icon(filename);

	image->set_pixel_size(24);
	image->set(icon ? icon->pixbuf() : NULL);

	// Icon end color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("icon_end_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"icon_end_color_button\" object in " + resource_file_);

	color = settings.iconColor(MapSettings::IconEnd);

	rgba.set_rgba(color[0], color[1], color[2], color[3]);

	colorbutton->set_rgba(rgba);

	// Icon end size
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("icon_end_size_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"icon_end_size_spinbutton\" object in " + resource_file_);

	spinbutton->set_value(settings.iconSize(MapSettings::IconEnd));

	// Icon position enable
	sw = ref_builder_->get_widget<Gtk::Switch>("icon_position_enable_switch");
	if (!sw)
		throw std::runtime_error("No \"icon_position_enable_switch\" object in " + resource_file_);

	sw->set_active(widget_->widget()->theme().hasFlag(VideoWidget::Theme::FlagIconPosition));

	// Icon position image
	image = ref_builder_->get_widget<Gtk::Image>("icon_position_image");
	if (!image)
		throw std::runtime_error("No \"icon_position_image\" object in " + resource_file_);

	filename = ((Map *) widget_->widget())->getIconFilename(settings.icon(MapSettings::IconPosition), MapSettings::IconPosition);
	icon = this->find_icon(filename);

	image->set_pixel_size(24);
	image->set(icon ? icon->pixbuf() : NULL);

	// Icon position color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("icon_position_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"icon_position_color_button\" object in " + resource_file_);

	color = settings.iconColor(MapSettings::IconPosition);

	rgba.set_rgba(color[0], color[1], color[2], color[3]);

	colorbutton->set_rgba(rgba);

	// Icon position size
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("icon_position_size_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"icon_position_size_spinbutton\" object in " + resource_file_);

	spinbutton->set_value(settings.iconSize(MapSettings::IconPosition));

	// Icon position follow
	sw = ref_builder_->get_widget<Gtk::Switch>("icon_position_follow_switch");
	if (!sw)
		throw std::runtime_error("No \"icon_position_follow_switch\" object in " + resource_file_);

	sw->set_active(((Map *) widget_->widget())->settings().followCourse());

	// Unmask value changed
	loading_ = false;
}


void GPX2VideoMapWidgetSettingsBox::update_boundaries(void) {
	log_call();

	Gtk::Button *button;
	Gtk::SpinButton *spinbutton;

	MapSettings::Source source = ((Map *) widget_->widget())->settings().source();

	// Update UI range for source
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("zoom_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"zoom_spinbutton\" object in " + resource_file_);

	spinbutton->set_range(
			MapSettings::getMinZoom(source),
			MapSettings::getMaxZoom(source));

	// Factor
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("factor_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"factor_spinbutton\" object in " + resource_file_);

	spinbutton->set_sensitive(((Map *) widget_->widget())->settings().view() != MapSettings::ViewZoomFit);

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


Glib::RefPtr<GPX2VideoMapWidgetSettingsBox::Icon> GPX2VideoMapWidgetSettingsBox::find_icon(const std::string &filename) {
	log_call();

	guint n_items;

	auto model = icon_model_->get_model();

	n_items = model->get_n_items();

	for (guint i=0; i < n_items; i++) {
		auto item = std::dynamic_pointer_cast<GPX2VideoMapWidgetSettingsBox::Icon>(model->get_object(i));

		if (!item || item->filename() != filename)
			continue;

		return item;
	}

	return NULL;
}


void GPX2VideoMapWidgetSettingsBox::create_popover(Gtk::MenuButton *menubutton, MapSettings::Icon type) {
	log_call();

	int index;

	Gtk::Button *button;
	Gtk::Popover *popover;
	Gtk::GridView *gridview;
	Gtk::CheckButton *checkbutton;

	MapSettings &settings = ((Map *) widget_->widget())->settings();

	// Icon model
	index = settings.icon(type);
	if (index == MapSettings::IconDefault)
		index = type;
	else if (index == MapSettings::IconUserFile) {
		guint n_items;

		auto model = icon_model_->get_model();

		index = -1;

		n_items = model->get_n_items();

		for (guint i=0; i < n_items; i++) {
			auto item = std::dynamic_pointer_cast<GPX2VideoMapWidgetSettingsBox::Icon>(model->get_object(i));
			if (!item || item->icon() != MapSettings::IconUserFile)
				continue;
			if (item->filename() != settings.iconFile(type))
				continue;
			index = i;
			break;
		}

		// Icon not found!
		if (index == -1) {
			// Auto append
			media_model_->append(GPX2VideoMedia::MediaIcon, settings.iconFile(type));

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

	gridview->set_sensitive((settings.icon(type) != MapSettings::IconDefault));
	gridview->set_model(icon_model_);
	gridview->set_factory(icon_factory_);
	gridview->set_min_columns(1);
	gridview->set_max_columns(10);

	// Icon gridview css
	gridview->get_style_context()->add_class("icon-grid");

	// Icon checkbutton
	checkbutton = ref_builder_->get_widget<Gtk::CheckButton>("icon_usedefault_checkbutton");
	if (!checkbutton)
		throw std::runtime_error("No \"icon_usedefault_checkbutton\" object in " + resource_file_);

	checkbutton->set_active((settings.icon(type) == MapSettings::IconDefault));

	// Connect icon checkbutton
	icon_checkbutton_connection_.disconnect();
	icon_checkbutton_connection_ = checkbutton->signal_toggled().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoMapWidgetSettingsBox::on_icon_usedefault_toggled), type));

	// Connect icon zoomin button
	button = ref_builder_->get_widget<Gtk::Button>("icon_zoomin_button");
	if (!button)
		throw std::runtime_error("No \"icon_zoomin_button\" object in " + resource_file_);

	icon_zoomin_connection_.disconnect();
	icon_zoomin_connection_ = button->signal_clicked().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoMapWidgetSettingsBox::on_icon_zoomin_clicked), type));

	// Connect icon zoomout button
	button = ref_builder_->get_widget<Gtk::Button>("icon_zoomout_button");
	if (!button)
		throw std::runtime_error("No \"icon_zoomout_button\" object in " + resource_file_);

	icon_zoomout_connection_.disconnect();
	icon_zoomout_connection_ = button->signal_clicked().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoMapWidgetSettingsBox::on_icon_zoomout_clicked), type));

	// Connect icon button
	button = ref_builder_->get_widget<Gtk::Button>("icon_ok_button");
	if (!button)
		throw std::runtime_error("No \"icon_ok_button\" object in " + resource_file_);

	icon_ok_connection_.disconnect();
	icon_ok_connection_ = button->signal_clicked().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoMapWidgetSettingsBox::on_icon_ok_clicked), type));
}


void GPX2VideoMapWidgetSettingsBox::on_icon_ok_clicked(MapSettings::Icon type) {
	log_call();

	bool use_default;

	std::string typestr;

	Gtk::CheckButton *checkbutton;

	Glib::RefPtr<const GPX2VideoMapWidgetSettingsBox::Icon> icon;

	MapSettings &settings = ((Map *) widget_->widget())->settings();

	// Convert type to string
	switch (type) {
	case MapSettings::IconStart:
		typestr = "start";
		break;
	case MapSettings::IconEnd:
		typestr = "end";
		break;
	case MapSettings::IconPosition:
		typestr = "position";
		break;
	default:
		break;
	}

	// Use default icon 
	checkbutton = ref_builder_->get_widget<Gtk::CheckButton>("icon_usedefault_checkbutton");
	if (!checkbutton)
		throw std::runtime_error("No \"icon_usedefault_checkbutton\" object in " + resource_file_);

	use_default = checkbutton->get_active();

	// Get icon selected
	icon = std::dynamic_pointer_cast<GPX2VideoMapWidgetSettingsBox::Icon>(icon_model_->get_selected_item());

	if (icon != NULL) {
		if (use_default) {
			log_notice("Widget %s: icon %s changed to 'default'", 
					widget_->name().c_str(), typestr.c_str());

			settings.setIcon(type, MapSettings::IconDefault);
			settings.setIconFile(type, "");
		}
		else {
			std::string filename = (icon->icon() == MapSettings::IconUserFile) ? icon->filename() : "";

			log_notice("Widget %s: icon %s changed to '%s%s'", 
					widget_->name().c_str(), typestr.c_str(), MapSettings::icon2string(icon->icon()).c_str(), filename.c_str());

			settings.setIcon(type, icon->icon());
			settings.setIconFile(type, filename);
		}

		// Update
		update_content();

		// Broadcast widget change
		widget_->dispatchEvent(true);
	}

	// Hide icon popover
	auto popover = ref_builder_->get_widget<Gtk::Popover>("icon_popover");
	if (!popover)
		throw std::runtime_error("No \"icon_popover\" object in " + resource_file_);

	popover->popdown();
}


void GPX2VideoMapWidgetSettingsBox::on_icon_usedefault_toggled(MapSettings::Icon type) {
	log_call();

	Gtk::GridView *gridview;
	Gtk::CheckButton *checkbutton;

	(void) type;

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


void GPX2VideoMapWidgetSettingsBox::on_icon_zoomin_clicked(MapSettings::Icon type) {
	log_call();

	Gtk::GridView *gridview;

	(void) type;

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


void GPX2VideoMapWidgetSettingsBox::on_icon_zoomout_clicked(MapSettings::Icon type) {
	log_call();

	Gtk::GridView *gridview;

	(void) type;

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


void GPX2VideoMapWidgetSettingsBox::on_model_changed(guint position, guint removed, guint added) {
	log_call();

	(void) position;
	(void) removed;
	(void) added;

	// Load models
	load_models();
}

