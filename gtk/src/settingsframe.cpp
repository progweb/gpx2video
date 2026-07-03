#include <glibmm/i18n.h>

#include <gtkmm/box.h>
#include <gtkmm/combobox.h>
#include <gtkmm/fontbutton.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/checkbutton.h>

#include <pangomm/cairofontmap.h>
#include <pangomm/fontdescription.h>

#include "log_i.h"
#include "../../src/videowidget.h"
#include "settingsframe.h"


GPX2VideoSettingsFrame::GPX2VideoSettingsFrame()
	: Glib::ObjectBase("GPX2VideoSettingsFrame")
	, dispatcher_() {
	log_call();

	loading_ = false;
	is_visible_ = false;
}


GPX2VideoSettingsFrame::GPX2VideoSettingsFrame(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &ref_builder) 
	: Glib::ObjectBase("GPX2VideoSettingsFrame")
	, Gtk::Frame(cobject)
	, ref_builder_(ref_builder)
	, dispatcher_() {
	log_call();

	loading_ = false;
	is_visible_ = false;

	settings_section_ = GPX2VideoSettings::SectionNone;

	// Populate models
	load_models();

	// Binding
	bind_content();

	// Update ui
	update_content();
	update_boundaries();
}


GPX2VideoSettingsFrame::~GPX2VideoSettingsFrame() {
	log_call();
}


void GPX2VideoSettingsFrame::set_visible(bool visible) {
	log_call();

	is_visible_ = visible;

	update_content();
}


void GPX2VideoSettingsFrame::set_section(GPX2VideoSettings::Section section) {
	log_call();

	settings_section_ = section;

	// Update ui content
	update_content();
	update_boundaries();
}


void GPX2VideoSettingsFrame::load_models(void) {
	log_call();

	Glib::RefPtr<Gtk::ListStore> font_weight_model;

	font_style_model_ = Gtk::ListStore::create(model_);

	{
		auto iter = font_style_model_->append();
		auto row = *iter;
		row[model_.m_id] = VideoWidget::Theme::FontStyleNormal;
		row[model_.m_name] = _("Normal");
		row[model_.m_enable] = true;

		row = *(font_style_model_->append());
		row[model_.m_id] = VideoWidget::Theme::FontStyleItalic;
		row[model_.m_name] = _("Italic");
		row[model_.m_enable] = true;
	}

	font_weight_model = Gtk::ListStore::create(model_);

	{
		auto iter = font_weight_model->append();
		auto row = *iter;
		row[model_.m_id] = VideoWidget::Theme::FontWeightThin;
		row[model_.m_name] = _("Thin (100)");
		row[model_.m_enable] = true;

		row = *(font_weight_model->append());
		row[model_.m_id] = VideoWidget::Theme::FontWeightUltraLight;
		row[model_.m_name] = _("Ultra light (200)");
		row[model_.m_enable] = true;

		row = *(font_weight_model->append());
		row[model_.m_id] = VideoWidget::Theme::FontWeightLight;
		row[model_.m_name] = _("Light (300)");
		row[model_.m_enable] = true;

		row = *(font_weight_model->append());
		row[model_.m_id] = VideoWidget::Theme::FontWeightSemiLight;
		row[model_.m_name] = _("Semi light (350)");
		row[model_.m_enable] = true;

		row = *(font_weight_model->append());
		row[model_.m_id] = VideoWidget::Theme::FontWeightBook;
		row[model_.m_name] = _("Book (380)");
		row[model_.m_enable] = true;

		row = *(font_weight_model->append());
		row[model_.m_id] = VideoWidget::Theme::FontWeightNormal;
		row[model_.m_name] = _("Normal (400)");
		row[model_.m_enable] = true;

		row = *(font_weight_model->append());
		row[model_.m_id] = VideoWidget::Theme::FontWeightMedium;
		row[model_.m_name] = _("Medium (500)");
		row[model_.m_enable] = true;

		row = *(font_weight_model->append());
		row[model_.m_id] = VideoWidget::Theme::FontWeightSemiBold;
		row[model_.m_name] = _("Semi bold (600)");
		row[model_.m_enable] = true;

		row = *(font_weight_model->append());
		row[model_.m_id] = VideoWidget::Theme::FontWeightUltraBold;
		row[model_.m_name] = _("Ultra bold (800)");
		row[model_.m_enable] = true;

		row = *(font_weight_model->append());
		row[model_.m_id] = VideoWidget::Theme::FontWeightHeavy;
		row[model_.m_name] = _("Heavy (900)");
		row[model_.m_enable] = true;

		row = *(font_weight_model->append());
		row[model_.m_id] = VideoWidget::Theme::FontWeightUltraHeavy;
		row[model_.m_name] = _("Ultra heavy (1000)");
		row[model_.m_enable] = true;
	}

	text_horizontal_align_model_ = Gtk::ListStore::create(model_);

	{
		auto iter = text_horizontal_align_model_->append();
		auto row = *iter;
		row[model_.m_id] = VideoWidget::Theme::AlignLeft;
		row[model_.m_name] = _("Left");
		row[model_.m_enable] = true;

		row = *(text_horizontal_align_model_->append());
		row[model_.m_id] = VideoWidget::Theme::AlignCenter;
		row[model_.m_name] = _("Center");
		row[model_.m_enable] = true;

		row = *(text_horizontal_align_model_->append());
		row[model_.m_id] = VideoWidget::Theme::AlignRight;
		row[model_.m_name] = _("Right");
		row[model_.m_enable] = true;
	}

	text_vertical_align_model_ = Gtk::ListStore::create(model_);

	{
		auto iter = text_vertical_align_model_->append();
		auto row = *iter;
		row[model_.m_id] = VideoWidget::Theme::AlignTop;
		row[model_.m_name] = _("Top");
		row[model_.m_enable] = true;

		row = *(text_vertical_align_model_->append());
		row[model_.m_id] = VideoWidget::Theme::AlignCenter;
		row[model_.m_name] = _("Center");
		row[model_.m_enable] = true;

		row = *(text_vertical_align_model_->append());
		row[model_.m_id] = VideoWidget::Theme::AlignBottom;
		row[model_.m_name] = _("Bottom");
		row[model_.m_enable] = true;
	}

	label_font_weight_model_ = duplicate_liststore(font_weight_model, model_);
	value_font_weight_model_ = duplicate_liststore(font_weight_model, model_);
	unit_font_weight_model_ = duplicate_liststore(font_weight_model, model_);
}


Glib::RefPtr<Gtk::ListStore> GPX2VideoSettingsFrame::duplicate_liststore(const Glib::RefPtr<Gtk::ListStore> &source, class Model &columns) {
	log_call();

	Glib::RefPtr<Gtk::ListStore> destination = Gtk::ListStore::create(columns);

	for (const auto& item : source->children()) {
		auto row = *(destination->append());

		row[columns.m_id] = item[columns.m_id];
		row[columns.m_name] = item[columns.m_name];
		row[columns.m_enable] = item[columns.m_enable];
	}
	
	return destination;
}


bool GPX2VideoSettingsFrame::find_in_listtore(const Glib::RefPtr<Gtk::ListStore> &store, const int &value, Gtk::TreeModel::iterator &result) {
	log_call();

	for (auto iter = store->children().begin(); iter != store->children().end(); iter++) {
		if (iter->get_value(model_.m_id) != value)
			continue;

		result = iter;

		return true;
	}

	return false;
}


void GPX2VideoSettingsFrame::bind_content(void) {
	log_call();

	Gtk::Button *button;
	Gtk::ComboBox *combobox;
	Gtk::FontButton *fontbutton;
	Gtk::SpinButton *spinbutton;
	Gtk::ColorButton *colorbutton;

	auto renderer = Gtk::make_managed<Gtk::CellRendererText>();

	// Connect apply button
	//----------------------

	button = ref_builder_->get_widget<Gtk::Button>("apply_button");
	if (!button)
		throw std::runtime_error("No \"apply_button\" object in settings_frame.ui");

	button->signal_clicked().connect(
				sigc::mem_fun(*this, &GPX2VideoSettingsFrame::on_apply_clicked), true);

	// Connect widgets button
	//------------------------

	// Label font family
	fontbutton = ref_builder_->get_widget<Gtk::FontButton>("label_font_family_fontbutton");
	if (!fontbutton)
		throw std::runtime_error("No \"label_font_family_fontbutton\" object in settigs_frame.ui");

	fontbutton->signal_font_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoSettingsFrame::on_font_changed), fontbutton, 
					[this](const Pango::FontDescription &description) {
						std::string value = description.get_family();

						GPX2VideoSettings *settings = GPX2VideoSettings::handle();

						log_notice("Widget default settings: label font family changed to '%s'", value.c_str());

						// Update label font weight combobox
						update_font_weight_model(label_font_weight_model_, value);

						settings->widget().setLabelFontFamily(value);
					}
			));

	// Label font size
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("label_font_size_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"label_font_size_spinbutton\" object in settigs_frame.ui");
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoSettingsFrame::on_spin_double_changed), spinbutton, 
					[this](const double &value) {
						Gtk::FontButton *button;

						Pango::FontDescription description;

						GPX2VideoSettings *settings = GPX2VideoSettings::handle();

						log_notice("Widget default settings: label font size changed to '%.1f'", value);

						// Update label font family button
						button = ref_builder_->get_widget<Gtk::FontButton>("label_font_family_fontbutton");

						if (!button)
							throw std::runtime_error("No \"label_font_family_fontbutton\" object in settigs_frame.ui");

						description = button->get_font_desc();

//						description.set_size(value * Pango::SCALE);
						description.set_absolute_size(value * Pango::SCALE);

						button->set_font_desc(description);

						settings->widget().setLabelFontSize(value);
					}
			));

	// Label shadow opacity
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("label_shadow_opacity_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"label_shadow_opacity_spinbutton\" object in settigs_frame.ui");
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoSettingsFrame::on_spin_int_changed), spinbutton, 
					[this](const int &value) {
						GPX2VideoSettings *settings = GPX2VideoSettings::handle();

						log_notice("Widget default settings: label shadow opacity changed to '%d'", value);

						settings->widget().setLabelShadowOpacity(value);
					}
			));

	// Label shadow distance
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("label_shadow_distance_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"label_shadow_distance_spinbutton\" object in settigs_frame.ui");
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoSettingsFrame::on_spin_double_changed), spinbutton, 
					[this](const double &value) {
						GPX2VideoSettings *settings = GPX2VideoSettings::handle();

						log_notice("Widget default settings: label shadow distance changed to '%.1f'", value);

						settings->widget().setLabelShadowDistance(value);
					}
			));

	// Label font style
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("label_fontstyle_combobox");
	if (!combobox)
		throw std::runtime_error("No \"label_fontstyle_combobox\" object in settigs_frame.ui");

	combobox->pack_start(*renderer, true);
	combobox->add_attribute(renderer->property_text(), model_.m_name);
	combobox->add_attribute(renderer->property_sensitive(), model_.m_enable);

//	combobox->pack_start(model_.m_name);
	combobox->signal_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoSettingsFrame::on_combobox_changed), combobox, 
					[this](const Gtk::TreeModel::const_iterator &iter) {
						Gtk::FontButton *button;

						Pango::FontDescription description;

						int value = iter->get_value(model_.m_id);

						GPX2VideoSettings *settings = GPX2VideoSettings::handle();

						log_notice("Widget default settings: label style changed to '%s'", 
								iter->get_value(model_.m_name).c_str());

						// Update label font family button
						button = ref_builder_->get_widget<Gtk::FontButton>("label_font_family_fontbutton");

						if (!button)
							throw std::runtime_error("No \"label_font_family_fontbutton\" object in settigs_frame.ui");

						description = button->get_font_desc();

						description.set_style((Pango::Style) value);

						button->set_font_desc(description);

						settings->widget().setLabelFontStyle((VideoWidget::Theme::FontStyle) value);
					}
			));

	// Label font weight
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("label_fontweight_combobox");
	if (!combobox)
		throw std::runtime_error("No \"label_fontweight_combobox\" object in settigs_frame.ui");

	combobox->pack_start(*renderer, true);
	combobox->add_attribute(renderer->property_text(), model_.m_name);
	combobox->add_attribute(renderer->property_sensitive(), model_.m_enable);

//	combobox->pack_start(model_.m_name);
	combobox->signal_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoSettingsFrame::on_combobox_changed), combobox, 
					[this](const Gtk::TreeModel::const_iterator &iter) {
						Gtk::FontButton *button;

						Pango::FontDescription description;

						int value = iter->get_value(model_.m_id);

						GPX2VideoSettings *settings = GPX2VideoSettings::handle();

						log_notice("Widget default settings: label weight changed to '%s'", 
								iter->get_value(model_.m_name).c_str());

						// Update label font family button
						button = ref_builder_->get_widget<Gtk::FontButton>("label_font_family_fontbutton");

						if (!button)
							throw std::runtime_error("No \"label_font_family_fontbutton\" object in settigs_frame.ui");

						description = button->get_font_desc();

						description.set_weight((Pango::Weight) value);

						button->set_font_desc(description);

						settings->widget().setLabelFontWeight((VideoWidget::Theme::FontWeight) value);
					}
			));

	// Label horizontal align
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("label_horizontal_align_combobox");
	if (!combobox)
		throw std::runtime_error("No \"label_horizontal_align_combobox\" object in settigs_frame.ui");

	combobox->pack_start(*renderer, true);
	combobox->add_attribute(renderer->property_text(), model_.m_name);
	combobox->add_attribute(renderer->property_sensitive(), model_.m_enable);

//	combobox->pack_start(model_.m_name);
	combobox->signal_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoSettingsFrame::on_combobox_changed), combobox, 
					[this](const Gtk::TreeModel::const_iterator &iter) {
						int value = iter->get_value(model_.m_id);

						GPX2VideoSettings *settings = GPX2VideoSettings::handle();

						log_notice("Widget default settings: label horizontal align changed to '%s'", 
								iter->get_value(model_.m_name).c_str());

						settings->widget().setLabelHorizontalAlign((VideoWidget::Theme::Align) value);
					}
			));

	// Label vertical align
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("label_vertical_align_combobox");
	if (!combobox)
		throw std::runtime_error("No \"label_vertical_align_combobox\" object in settigs_frame.ui");

	combobox->pack_start(*renderer, true);
	combobox->add_attribute(renderer->property_text(), model_.m_name);
	combobox->add_attribute(renderer->property_sensitive(), model_.m_enable);

//	combobox->pack_start(model_.m_name);
	combobox->signal_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoSettingsFrame::on_combobox_changed), combobox, 
					[this](const Gtk::TreeModel::const_iterator &iter) {
						int value = iter->get_value(model_.m_id);

						GPX2VideoSettings *settings = GPX2VideoSettings::handle();

						log_notice("Widget default settings: label vertical align changed to '%s'", 
								iter->get_value(model_.m_name).c_str());

						settings->widget().setLabelVerticalAlign((VideoWidget::Theme::Align) value);
					}
			));

	// Label color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("label_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"label_color_button\" object in settigs_frame.ui");
	colorbutton->signal_color_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoSettingsFrame::on_color_changed), colorbutton, 
					[this](const std::string &color) {
						GPX2VideoSettings *settings = GPX2VideoSettings::handle();

						log_notice("Widget default settings: label color changed to '%s'", color.c_str());

						settings->widget().setLabelColor(color);
					}
			));

	// Label border width
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("label_border_width_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"label_border_width_spinbutton\" object in settigs_frame.ui");
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoSettingsFrame::on_spin_double_changed), spinbutton, 
					[this](const double &value) {
						GPX2VideoSettings *settings = GPX2VideoSettings::handle();

						log_notice("Widget default settings: label border width changed to '%.1f'", value);

						settings->widget().setLabelBorderWidth(value);
					}
			));

	// Label border color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("label_border_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"label_border_color_button\" object in settigs_frame.ui");
	colorbutton->signal_color_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoSettingsFrame::on_color_changed), colorbutton, 
					[this](const std::string &color) {
						GPX2VideoSettings *settings = GPX2VideoSettings::handle();

						log_notice("Widget default settings: label border color changed to '%s'", color.c_str());

						settings->widget().setLabelBorderColor(color);
					}
			));

	// Value font family
	fontbutton = ref_builder_->get_widget<Gtk::FontButton>("value_font_family_fontbutton");
	if (!fontbutton)
		throw std::runtime_error("No \"value_font_family_fontbutton\" object in settigs_frame.ui");

	fontbutton->signal_font_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoSettingsFrame::on_font_changed), fontbutton, 
					[this](const Pango::FontDescription &description) {
						std::string value = description.get_family();

						GPX2VideoSettings *settings = GPX2VideoSettings::handle();

						log_notice("Widget default settings: value font family changed to '%s'", value.c_str());

						// Update value font weight combobox
						update_font_weight_model(value_font_weight_model_, value);

						settings->widget().setValueFontFamily(value);
					}
			));

	// Value font size
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("value_font_size_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"value_font_size_spinbutton\" object in settigs_frame.ui");
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoSettingsFrame::on_spin_double_changed), spinbutton, 
					[this](const double &value) {
						Gtk::FontButton *button;

						Pango::FontDescription description;

						GPX2VideoSettings *settings = GPX2VideoSettings::handle();

						log_notice("Widget default settings: value font size changed to '%.1f'", value);

						// Update value font family button
						button = ref_builder_->get_widget<Gtk::FontButton>("value_font_family_fontbutton");

						if (!button)
							throw std::runtime_error("No \"value_font_family_fontbutton\" object in settigs_frame.ui");

						description = button->get_font_desc();

//						description.set_size(value * Pango::SCALE);
						description.set_absolute_size(value * Pango::SCALE);

						button->set_font_desc(description);

						settings->widget().setValueFontSize(value);
					}
			));

	// Value shadow opacity
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("value_shadow_opacity_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"value_shadow_opacity_spinbutton\" object in settigs_frame.ui");
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoSettingsFrame::on_spin_int_changed), spinbutton, 
					[this](const int &value) {
						GPX2VideoSettings *settings = GPX2VideoSettings::handle();

						log_notice("Widget default settings: value shadow opacity_changed to '%d'", value);

						settings->widget().setValueShadowOpacity(value);
					}
			));

	// Value shadow distance
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("value_shadow_distance_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"value_shadow_distance_spinbutton\" object in settigs_frame.ui");
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoSettingsFrame::on_spin_double_changed), spinbutton, 
					[this](const double &value) {
						GPX2VideoSettings *settings = GPX2VideoSettings::handle();

						log_notice("Widget default settings: value shadow distance_changed to '%.1f'", value);

						settings->widget().setValueShadowDistance(value);
					}
			));

	// Value font style
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("value_fontstyle_combobox");
	if (!combobox)
		throw std::runtime_error("No \"value_fontstyle_combobox\" object in settigs_frame.ui");

	combobox->pack_start(*renderer, true);
	combobox->add_attribute(renderer->property_text(), model_.m_name);
	combobox->add_attribute(renderer->property_sensitive(), model_.m_enable);

//	combobox->pack_start(model_.m_name);
	combobox->signal_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoSettingsFrame::on_combobox_changed), combobox, 
					[this](const Gtk::TreeModel::const_iterator &iter) {
						Gtk::FontButton *button;

						Pango::FontDescription description;

						int value = iter->get_value(model_.m_id);

						GPX2VideoSettings *settings = GPX2VideoSettings::handle();

						log_notice("Widget default settings: value style changed to '%s'", 
								iter->get_value(model_.m_name).c_str());

						// Update value font family button
						button = ref_builder_->get_widget<Gtk::FontButton>("value_font_family_fontbutton");

						if (!button)
							throw std::runtime_error("No \"value_font_family_fontbutton\" object in settigs_frame.ui");

						description = button->get_font_desc();

						description.set_style((Pango::Style) value);

						button->set_font_desc(description);

						settings->widget().setValueFontStyle((VideoWidget::Theme::FontStyle) value);
					}
			));

	// Value font weight
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("value_fontweight_combobox");
	if (!combobox)
		throw std::runtime_error("No \"value_fontweight_combobox\" object in settigs_frame.ui");

	combobox->pack_start(*renderer, true);
	combobox->add_attribute(renderer->property_text(), model_.m_name);
	combobox->add_attribute(renderer->property_sensitive(), model_.m_enable);

//	combobox->pack_start(model_.m_name);
	combobox->signal_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoSettingsFrame::on_combobox_changed), combobox, 
					[this](const Gtk::TreeModel::const_iterator &iter) {
						Gtk::FontButton *button;

						Pango::FontDescription description;

						int value = iter->get_value(model_.m_id);

						GPX2VideoSettings *settings = GPX2VideoSettings::handle();

						log_notice("Widget default settings: value weight changed to '%s'", 
								iter->get_value(model_.m_name).c_str());

						// Update value font family button
						button = ref_builder_->get_widget<Gtk::FontButton>("value_font_family_fontbutton");

						if (!button)
							throw std::runtime_error("No \"value_font_family_fontbutton\" object in settigs_frame.ui");

						description = button->get_font_desc();

						description.set_weight((Pango::Weight) value);

						button->set_font_desc(description);

						settings->widget().setValueFontWeight((VideoWidget::Theme::FontWeight) value);
					}
			));

	// Value horizontal align
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("value_horizontal_align_combobox");
	if (!combobox)
		throw std::runtime_error("No \"value_horizontal_align_combobox\" object in settigs_frame.ui");

	combobox->pack_start(*renderer, true);
	combobox->add_attribute(renderer->property_text(), model_.m_name);
	combobox->add_attribute(renderer->property_sensitive(), model_.m_enable);

//	combobox->pack_start(model_.m_name);
	combobox->signal_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoSettingsFrame::on_combobox_changed), combobox, 
					[this](const Gtk::TreeModel::const_iterator &iter) {
						int value = iter->get_value(model_.m_id);

						GPX2VideoSettings *settings = GPX2VideoSettings::handle();

						log_notice("Widget default settings: value horizontal align changed to '%s'", 
								iter->get_value(model_.m_name).c_str());

						settings->widget().setValueHorizontalAlign((VideoWidget::Theme::Align) value);
					}
			));

	// Value vertical align
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("value_vertical_align_combobox");
	if (!combobox)
		throw std::runtime_error("No \"value_vertical_align_combobox\" object in settigs_frame.ui");

	combobox->pack_start(*renderer, true);
	combobox->add_attribute(renderer->property_text(), model_.m_name);
	combobox->add_attribute(renderer->property_sensitive(), model_.m_enable);

//	combobox->pack_start(model_.m_name);
	combobox->signal_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoSettingsFrame::on_combobox_changed), combobox, 
					[this](const Gtk::TreeModel::const_iterator &iter) {
						int value = iter->get_value(model_.m_id);

						GPX2VideoSettings *settings = GPX2VideoSettings::handle();

						log_notice("Widget default settings: value vertical align changed to '%s'", 
								iter->get_value(model_.m_name).c_str());

						settings->widget().setValueVerticalAlign((VideoWidget::Theme::Align) value);
					}
			));

	// Value color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("value_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"value_color_button\" object in settigs_frame.ui");
	colorbutton->signal_color_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoSettingsFrame::on_color_changed), colorbutton, 
					[this](const std::string &color) {
						GPX2VideoSettings *settings = GPX2VideoSettings::handle();

						log_notice("Widget default settings: value color changed to '%s'", 
								color.c_str());

						settings->widget().setValueColor(color);
					}
			));

	// Value border width
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("value_border_width_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"value_border_width_spinbutton\" object in settigs_frame.ui");
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoSettingsFrame::on_spin_double_changed), spinbutton, 
					[this](const double &value) {
						GPX2VideoSettings *settings = GPX2VideoSettings::handle();

						log_notice("Widget default settings: value border width changed to '%.1f'",
							   value);

						settings->widget().setValueBorderWidth(value);
					}
			));

	// Value border color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("value_border_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"value_border_color_button\" object in settigs_frame.ui");
	colorbutton->signal_color_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoSettingsFrame::on_color_changed), colorbutton, 
					[this](const std::string &color) {
						GPX2VideoSettings *settings = GPX2VideoSettings::handle();

						log_notice("Widget default settings: value border color changed to '%s'", 
								color.c_str());

						settings->widget().setValueBorderColor(color);
					}
			));

	// Unit font family
	fontbutton = ref_builder_->get_widget<Gtk::FontButton>("unit_font_family_fontbutton");
	if (!fontbutton)
		throw std::runtime_error("No \"unit_font_family_fontbutton\" object in settigs_frame.ui");

	fontbutton->signal_font_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoSettingsFrame::on_font_changed), fontbutton, 
					[this](const Pango::FontDescription &description) {
						std::string value = description.get_family();

						GPX2VideoSettings *settings = GPX2VideoSettings::handle();

						log_notice("Widget default settings: unit font family changed to '%s'", value.c_str());

						// Update unit font weight combobox
						update_font_weight_model(unit_font_weight_model_, value);

						settings->widget().setUnitFontFamily(value);
					}
			));

	// Unit font size
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("unit_font_size_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"unit_font_size_spinbutton\" object in settigs_frame.ui");
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoSettingsFrame::on_spin_double_changed), spinbutton, 
					[this](const double &value) {
						Gtk::FontButton *button;

						Pango::FontDescription description;

						GPX2VideoSettings *settings = GPX2VideoSettings::handle();

						log_notice("Widget default settings: unit font size changed to '%.1f'",
							   value);

						// Update unit font family button
						button = ref_builder_->get_widget<Gtk::FontButton>("unit_font_family_fontbutton");

						if (!button)
							throw std::runtime_error("No \"unit_font_family_fontbutton\" object in settigs_frame.ui");

						description = button->get_font_desc();

//						description.set_size(value * Pango::SCALE);
						description.set_absolute_size(value * Pango::SCALE);

						button->set_font_desc(description);

						settings->widget().setUnitFontSize(value);
					}
			));

	// Unit shadow opacity
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("unit_shadow_opacity_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"unit_shadow_opacity_spinbutton\" object in settigs_frame.ui");
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoSettingsFrame::on_spin_int_changed), spinbutton, 
					[this](const int &value) {
						GPX2VideoSettings *settings = GPX2VideoSettings::handle();

						log_notice("Widget default settings: unit shadow opacity_changed to '%d'", value);

						settings->widget().setUnitShadowOpacity(value);
					}
			));

	// Unit shadow distance
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("unit_shadow_distance_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"unit_shadow_distance_spinbutton\" object in settigs_frame.ui");
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoSettingsFrame::on_spin_double_changed), spinbutton, 
					[this](const double &value) {
						GPX2VideoSettings *settings = GPX2VideoSettings::handle();

						log_notice("Widget default settings: unit shadow distance_changed to '%.1f'", value);

						settings->widget().setUnitShadowDistance(value);
					}
			));

	// Unit font style
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("unit_fontstyle_combobox");
	if (!combobox)
		throw std::runtime_error("No \"unit_fontstyle_combobox\" object in settigs_frame.ui");

	combobox->pack_start(*renderer, true);
	combobox->add_attribute(renderer->property_text(), model_.m_name);
	combobox->add_attribute(renderer->property_sensitive(), model_.m_enable);

//	combobox->pack_start(model_.m_name);
	combobox->signal_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoSettingsFrame::on_combobox_changed), combobox, 
					[this](const Gtk::TreeModel::const_iterator &iter) {
						Gtk::FontButton *button;

						Pango::FontDescription description;

						int value = iter->get_value(model_.m_id);

						GPX2VideoSettings *settings = GPX2VideoSettings::handle();

						log_notice("Widget default settings: unit style changed to '%s'", 
								iter->get_value(model_.m_name).c_str());

						// Update unit font family button
						button = ref_builder_->get_widget<Gtk::FontButton>("unit_font_family_fontbutton");

						if (!button)
							throw std::runtime_error("No \"unit_font_family_fontbutton\" object in settigs_frame.ui");

						description = button->get_font_desc();

						description.set_style((Pango::Style) value);

						button->set_font_desc(description);

						settings->widget().setUnitFontStyle((VideoWidget::Theme::FontStyle) value);
					}
			));

	// Unit font weight
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("unit_fontweight_combobox");
	if (!combobox)
		throw std::runtime_error("No \"unit_fontweight_combobox\" object in settigs_frame.ui");

	combobox->pack_start(*renderer, true);
	combobox->add_attribute(renderer->property_text(), model_.m_name);
	combobox->add_attribute(renderer->property_sensitive(), model_.m_enable);

//	combobox->pack_start(model_.m_name);
	combobox->signal_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoSettingsFrame::on_combobox_changed), combobox, 
					[this](const Gtk::TreeModel::const_iterator &iter) {
						Gtk::FontButton *button;

						Pango::FontDescription description;

						int value = iter->get_value(model_.m_id);

						GPX2VideoSettings *settings = GPX2VideoSettings::handle();

						log_notice("Widget default settings: unit weight changed to '%s'", 
								iter->get_value(model_.m_name).c_str());

						// Update unit font family button
						button = ref_builder_->get_widget<Gtk::FontButton>("unit_font_family_fontbutton");

						if (!button)
							throw std::runtime_error("No \"unit_font_family_fontbutton\" object in settigs_frame.ui");

						description = button->get_font_desc();

						description.set_weight((Pango::Weight) value);

						button->set_font_desc(description);

						settings->widget().setUnitFontWeight((VideoWidget::Theme::FontWeight) value);
					}
			));

	// Unit horizontal align
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("unit_horizontal_align_combobox");
	if (!combobox)
		throw std::runtime_error("No \"unit_horizontal_align_combobox\" object in settigs_frame.ui");

	combobox->pack_start(*renderer, true);
	combobox->add_attribute(renderer->property_text(), model_.m_name);
	combobox->add_attribute(renderer->property_sensitive(), model_.m_enable);

//	combobox->pack_start(model_.m_name);
	combobox->signal_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoSettingsFrame::on_combobox_changed), combobox, 
					[this](const Gtk::TreeModel::const_iterator &iter) {
						int value = iter->get_value(model_.m_id);

						GPX2VideoSettings *settings = GPX2VideoSettings::handle();

						log_notice("Widget default settings: unit horizontal align changed to '%s'", 
								iter->get_value(model_.m_name).c_str());

						settings->widget().setUnitHorizontalAlign((VideoWidget::Theme::Align) value);
					}
			));

	// Unit vertical align
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("unit_vertical_align_combobox");
	if (!combobox)
		throw std::runtime_error("No \"unit_vertical_align_combobox\" object in settigs_frame.ui");

	combobox->pack_start(*renderer, true);
	combobox->add_attribute(renderer->property_text(), model_.m_name);
	combobox->add_attribute(renderer->property_sensitive(), model_.m_enable);

//	combobox->pack_start(model_.m_name);
	combobox->signal_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoSettingsFrame::on_combobox_changed), combobox, 
					[this](const Gtk::TreeModel::const_iterator &iter) {
						int value = iter->get_value(model_.m_id);

						GPX2VideoSettings *settings = GPX2VideoSettings::handle();

						log_notice("Widget default settings: unit vertical align changed to '%s'", 
								iter->get_value(model_.m_name).c_str());

						settings->widget().setUnitVerticalAlign((VideoWidget::Theme::Align) value);
					}
			));

	// Unit color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("unit_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"unit_color_button\" object in settigs_frame.ui");
	colorbutton->signal_color_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoSettingsFrame::on_color_changed), colorbutton, 
					[this](const std::string &color) {
						GPX2VideoSettings *settings = GPX2VideoSettings::handle();

						log_notice("Widget default settings: unit color changed to '%s'", color.c_str());

						settings->widget().setUnitColor(color);
					}
			));

	// Unit border width
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("unit_border_width_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"unit_border_width_spinbutton\" object in settigs_frame.ui");
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoSettingsFrame::on_spin_double_changed), spinbutton, 
					[this](const double &value) {
						GPX2VideoSettings *settings = GPX2VideoSettings::handle();

						log_notice("Widget default settings: unit border width changed to '%.1f'", value);

						settings->widget().setUnitBorderWidth(value);
					}
			));

	// Unit border color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("unit_border_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"unit_border_color_button\" object in settigs_frame.ui");
	colorbutton->signal_color_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoSettingsFrame::on_color_changed), colorbutton, 
					[this](const std::string &color) {
						GPX2VideoSettings *settings = GPX2VideoSettings::handle();

						log_notice("Widget default settings: unit border color changed to '%s'", color.c_str());

						settings->widget().setUnitBorderColor(color);
					}
			));

	// Unit distance
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("unit_distance_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"unit_distance_spinbutton\" object in settigs_frame.ui");
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoSettingsFrame::on_spin_double_changed), spinbutton, 
					[this](const double &value) {
						GPX2VideoSettings *settings = GPX2VideoSettings::handle();

						log_notice("Widget default settings: unit distance changed to '%.1f'", value);

						settings->widget().setUnitDistance(value);
					}
			));
}


void GPX2VideoSettingsFrame::update_content(void) {
	log_call();

	Gdk::RGBA rgba;

	const float *color;

	Gtk::ComboBox *combobox;
	Gtk::FontButton *fontbutton;
	Gtk::SpinButton *spinbutton;
	Gtk::ColorButton *colorbutton;

	Pango::FontDescription description;

	Gtk::TreeModel::iterator iter;

	GPX2VideoSettings *settings = GPX2VideoSettings::handle();

	// Visible property
	Gtk::Frame::set_visible(is_visible_);

	log_info("Load settings...");

	// Mask value changed
	loading_ = true;

	// Label font family
	fontbutton = ref_builder_->get_widget<Gtk::FontButton>("label_font_family_fontbutton");
	if (!fontbutton)
		throw std::runtime_error("No \"label_font_family_fontbutton\" object in settigs_frame.ui");

	description.set_family(settings->widget().labelFontFamily());
	description.set_style((Pango::Style) settings->widget().labelFontStyle());
	description.set_variant(Pango::Variant::NORMAL);
	description.set_weight((Pango::Weight) settings->widget().labelFontWeight());
	description.set_stretch(Pango::Stretch::NORMAL);
	description.set_absolute_size(settings->widget().labelFontSize() * Pango::SCALE);

	fontbutton->set_level(Gtk::FontButton::Level::FAMILY);
	fontbutton->set_font_desc(description);

	// Label size
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("label_font_size_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"label_font_size_spinbutton\" object in settigs_frame.ui");

	spinbutton->set_value(settings->widget().labelFontSize());

	// Label shadow opacity
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("label_shadow_opacity_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"label_shadow_opacity_spinbutton\" object in settigs_frame.ui");

	spinbutton->set_value(settings->widget().labelShadowOpacity());

	// Label shadow distance
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("label_shadow_distance_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"label_shadow_distance_spinbutton\" object in settigs_frame.ui");

	spinbutton->set_value(settings->widget().labelShadowDistance());

	// Label font style
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("label_fontstyle_combobox");
	if (!combobox)
		throw std::runtime_error("No \"label_fontstyle_combobox\" object in settigs_frame.ui");

	combobox->set_model(font_style_model_);

	if (find_in_listtore(font_style_model_, settings->widget().labelFontStyle(), iter))
		combobox->set_active(iter);

	// Label font weight
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("label_fontweight_combobox");
	if (!combobox)
		throw std::runtime_error("No \"label_fontweight_combobox\" object in settigs_frame.ui");

	combobox->set_model(label_font_weight_model_);

	if (find_in_listtore(label_font_weight_model_, settings->widget().labelFontWeight(), iter))
		combobox->set_active(iter);

	// Label horizontal align
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("label_horizontal_align_combobox");
	if (!combobox)
		throw std::runtime_error("No \"label_horizontal_align_combobox\" object in settigs_frame.ui");

	combobox->set_model(text_horizontal_align_model_);

	if (find_in_listtore(text_horizontal_align_model_, settings->widget().labelHorizontalAlign(), iter))
		combobox->set_active(iter);

	// Label vertical align
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("label_vertical_align_combobox");
	if (!combobox)
		throw std::runtime_error("No \"label_vertical_align_combobox\" object in settigs_frame.ui");

	combobox->set_model(text_vertical_align_model_);

	if (find_in_listtore(text_vertical_align_model_, settings->widget().labelVerticalAlign(), iter))
		combobox->set_active(iter);

	// Label color button
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("label_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"label_color_button\" object in settigs_frame.ui");

	color = settings->widget().labelColor();

	rgba.set_rgba(color[0], color[1], color[2], color[3]);

	colorbutton->set_rgba(rgba);

	// Label border width
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("label_border_width_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"label_border_width_spinbutton\" object in settigs_frame.ui");

	spinbutton->set_value(settings->widget().labelBorderWidth());

	// Label border color button
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("label_border_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"label_border_color_button\" object in settigs_frame.ui");

	color = settings->widget().labelBorderColor();

	rgba.set_rgba(color[0], color[1], color[2], color[3]);

	colorbutton->set_rgba(rgba);

	// Value font family
	fontbutton = ref_builder_->get_widget<Gtk::FontButton>("value_font_family_fontbutton");
	if (!fontbutton)
		throw std::runtime_error("No \"value_font_family_fontbutton\" object in settigs_frame.ui");

	description.set_family(settings->widget().valueFontFamily());
	description.set_style((Pango::Style) settings->widget().valueFontStyle());
	description.set_variant(Pango::Variant::NORMAL);
	description.set_weight((Pango::Weight) settings->widget().valueFontWeight());
	description.set_stretch(Pango::Stretch::NORMAL);
	description.set_absolute_size(settings->widget().valueFontSize() * Pango::SCALE);

	fontbutton->set_level(Gtk::FontButton::Level::FAMILY);
	fontbutton->set_font_desc(description);

	// Value size
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("value_font_size_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"value_font_size_spinbutton\" object in settigs_frame.ui");

	spinbutton->set_value(settings->widget().valueFontSize());

	// Value shadow opacity
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("value_shadow_opacity_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"value_shadow_opacity_spinbutton\" object in settigs_frame.ui");

	spinbutton->set_value(settings->widget().valueShadowOpacity());

	// Value shadow distance
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("value_shadow_distance_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"value_shadow_distance_spinbutton\" object in settigs_frame.ui");

	spinbutton->set_value(settings->widget().valueShadowDistance());

	// Value font style
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("value_fontstyle_combobox");
	if (!combobox)
		throw std::runtime_error("No \"value_fontstyle_combobox\" object in settigs_frame.ui");

	combobox->set_model(font_style_model_);

	if (find_in_listtore(font_style_model_, settings->widget().valueFontStyle(), iter))
		combobox->set_active(iter);

	// Value font weight
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("value_fontweight_combobox");
	if (!combobox)
		throw std::runtime_error("No \"value_fontweight_combobox\" object in settigs_frame.ui");

	combobox->set_model(value_font_weight_model_);

	if (find_in_listtore(value_font_weight_model_, settings->widget().valueFontWeight(), iter))
		combobox->set_active(iter);

	// Value horizontal align
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("value_horizontal_align_combobox");
	if (!combobox)
		throw std::runtime_error("No \"value_horizontal_align_combobox\" object in settigs_frame.ui");

	combobox->set_model(text_horizontal_align_model_);

	if (find_in_listtore(text_horizontal_align_model_, settings->widget().valueHorizontalAlign(), iter))
		combobox->set_active(iter);

	// Value vertical align
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("value_vertical_align_combobox");
	if (!combobox)
		throw std::runtime_error("No \"value_vertical_align_combobox\" object in settigs_frame.ui");

	combobox->set_model(text_vertical_align_model_);

	if (find_in_listtore(text_vertical_align_model_, settings->widget().valueVerticalAlign(), iter))
		combobox->set_active(iter);

	// Value color button
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("value_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"value_color_button\" object in settigs_frame.ui");

	color = settings->widget().valueColor();

	rgba.set_rgba(color[0], color[1], color[2], color[3]);

	colorbutton->set_rgba(rgba);

	// Value border width
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("value_border_width_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"value_border_width_spinbutton\" object in settigs_frame.ui");

	spinbutton->set_value(settings->widget().valueBorderWidth());

	// Value border color button
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("value_border_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"value_border_color_button\" object in settigs_frame.ui");

	color = settings->widget().valueBorderColor();

	rgba.set_rgba(color[0], color[1], color[2], color[3]);

	colorbutton->set_rgba(rgba);

	// Unit font family
	fontbutton = ref_builder_->get_widget<Gtk::FontButton>("unit_font_family_fontbutton");
	if (!fontbutton)
		throw std::runtime_error("No \"unit_font_family_fontbutton\" object in settigs_frame.ui");

	description.set_family(settings->widget().unitFontFamily());
	description.set_style((Pango::Style) settings->widget().unitFontStyle());
	description.set_variant(Pango::Variant::NORMAL);
	description.set_weight((Pango::Weight) settings->widget().unitFontWeight());
	description.set_stretch(Pango::Stretch::NORMAL);
	description.set_absolute_size(settings->widget().unitFontSize() * Pango::SCALE);

	fontbutton->set_level(Gtk::FontButton::Level::FAMILY);
	fontbutton->set_font_desc(description);

	// Unit size
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("unit_font_size_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"unit_font_size_spinbutton\" object in settigs_frame.ui");

	spinbutton->set_value(settings->widget().unitFontSize());

	// Unit shadow opacity
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("unit_shadow_opacity_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"unit_shadow_opacity_spinbutton\" object in settigs_frame.ui");

	spinbutton->set_value(settings->widget().unitShadowOpacity());

	// Unit shadow distance
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("unit_shadow_distance_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"unit_shadow_distance_spinbutton\" object in settigs_frame.ui");

	spinbutton->set_value(settings->widget().unitShadowDistance());

	// Unit font style
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("unit_fontstyle_combobox");
	if (!combobox)
		throw std::runtime_error("No \"unit_fontstyle_combobox\" object in settigs_frame.ui");

	combobox->set_model(font_style_model_);

	if (find_in_listtore(font_style_model_, settings->widget().unitFontStyle(), iter))
		combobox->set_active(iter);

	// Unit font weight
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("unit_fontweight_combobox");
	if (!combobox)
		throw std::runtime_error("No \"unit_fontweight_combobox\" object in settigs_frame.ui");

	combobox->set_model(unit_font_weight_model_);

	if (find_in_listtore(unit_font_weight_model_, settings->widget().unitFontWeight(), iter))
		combobox->set_active(iter);

	// Unit horizontal align
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("unit_horizontal_align_combobox");
	if (!combobox)
		throw std::runtime_error("No \"unit_horizontal_align_combobox\" object in settigs_frame.ui");

	combobox->set_model(text_horizontal_align_model_);

	if (find_in_listtore(text_horizontal_align_model_, settings->widget().unitHorizontalAlign(), iter))
		combobox->set_active(iter);

	// Unit vertical align
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("unit_vertical_align_combobox");
	if (!combobox)
		throw std::runtime_error("No \"unit_vertical_align_combobox\" object in settigs_frame.ui");

	combobox->set_model(text_vertical_align_model_);

	if (find_in_listtore(text_vertical_align_model_, settings->widget().unitVerticalAlign(), iter))
		combobox->set_active(iter);

	// Unit color button
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("unit_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"unit_color_button\" object in settigs_frame.ui");

	color = settings->widget().unitColor();

	rgba.set_rgba(color[0], color[1], color[2], color[3]);

	colorbutton->set_rgba(rgba);

	// Unit border width
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("unit_border_width_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"unit_border_width_spinbutton\" object in settigs_frame.ui");

	spinbutton->set_value(settings->widget().unitBorderWidth());

	// Unit border color button
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("unit_border_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"unit_border_color_button\" object in settigs_frame.ui");

	color = settings->widget().unitBorderColor();

	rgba.set_rgba(color[0], color[1], color[2], color[3]);

	colorbutton->set_rgba(rgba);

	// Unit distance
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("unit_distance_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"unit_distance_spinbutton\" object in settigs_frame.ui");

	spinbutton->set_value(settings->widget().unitDistance());

	log_info("Settings loaded");

	// Unmask value changed
	loading_ = false;
}


void GPX2VideoSettingsFrame::update_boundaries(void) {
	log_call();

	Gtk::Box *box;

	// Video export settings box
	box = ref_builder_->get_widget<Gtk::Box>("settings_video_box");
	if (!box)
		throw std::runtime_error("No \"settings_video_box\" object in settings_frame.ui");
	box->set_visible((settings_section_ == GPX2VideoSettings::SectionVideo));

	// Widget default settings box
	box = ref_builder_->get_widget<Gtk::Box>("settings_widget_box");
	if (!box)
		throw std::runtime_error("No \"settings_widget_box\" object in settings_frame.ui");
	box->set_visible((settings_section_ == GPX2VideoSettings::SectionWidget));
}


void GPX2VideoSettingsFrame::update_font_weight_model(const Glib::RefPtr<Gtk::ListStore> &store, const std::string &value) {
	log_call();

	Gtk::TreeModel::iterator iter;

	Glib::RefPtr<Pango::FontMap> fontmap = Pango::CairoFontMap::get_default();

	Glib::RefPtr<const Pango::FontFamily> family = fontmap->get_family(value);

	std::vector<Glib::RefPtr<Pango::FontFace>> faces = family->list_faces();

	// Disable each item
	for (iter = store->children().begin(); iter != store->children().end(); iter++) {
		iter->set_value(model_.m_enable, false);
	}

	// Enable supported items
	for (Glib::RefPtr<Pango::FontFace> face : faces) {
		Pango::Weight weight = face->describe().get_weight();

		if (find_in_listtore(store, weight, iter))
			iter->set_value(model_.m_enable, true);
	}
}


void GPX2VideoSettingsFrame::on_font_changed(Gtk::FontButton *button, std::function<void(const Pango::FontDescription&)> set) {
	log_call();

	Pango::FontDescription description;

	if (loading_)
		return;

	// Read font description
	description = button->get_font_desc();

	// Set description
	set(description);
}


void GPX2VideoSettingsFrame::on_spin_int_changed(Gtk::SpinButton *button, std::function<void(const int&)> set) {
	log_call();

	int value;

	if (loading_)
		return;

	// Read value
	value = button->get_value_as_int();

	// Set value
	set(value);
}


void GPX2VideoSettingsFrame::on_spin_double_changed(Gtk::SpinButton *button, std::function<void(const double&)> set) {
	log_call();

	double value;

	if (loading_)
		return;

	// Read value
	value = button->get_value();

	// Set value
	set(value);
}


void GPX2VideoSettingsFrame::on_color_changed(Gtk::ColorButton *button, std::function<void(const std::string&)> set) {
	log_call();

	Gdk::RGBA rgba;

	Glib::ustring color;

	if (loading_)
		return;

	// Read color
	rgba = button->get_rgba();

	// Convert to hexa string color
	color = Glib::ustring::sprintf("#%02X%02X%02X%02X",
			(unsigned char) std::round(rgba.get_red() * 255),
			(unsigned char) std::round(rgba.get_green() * 255),
			(unsigned char) std::round(rgba.get_blue() * 255),
			(unsigned char) std::round(rgba.get_alpha() * 255)
	);

	// Set color
	set(color);
}


void GPX2VideoSettingsFrame::on_combobox_changed(Gtk::ComboBox *combobox, std::function<void(const Gtk::TreeModel::const_iterator&)> set) {
	log_call();

	if (loading_)
		return;

	// Set combobox
	set(combobox->get_active());
}


void GPX2VideoSettingsFrame::on_apply_clicked(void) {
	log_call();

	// Load widget default settings requested
	dispatcher_.emit();
}

