#include <gtkmm/box.h>
#include <gtkmm/entry.h>
#include <gtkmm/switch.h>
#include <gtkmm/combobox.h>
#include <gtkmm/fontbutton.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/colorbutton.h>

#include <pangomm/cairofontmap.h>
#include <pangomm/fontdescription.h>

#include "log.h"
#include "shape/arc.h"
#include "shape/bar.h"
#include "shape/text.h"
#include "widgets/map.h"
#include "widgetframe.h"



GPX2VideoWidgetFrame::GPX2VideoWidgetFrame()
	: Glib::ObjectBase("GPX2VideoWidgetFrame")
	, ref_builder_(NULL) 
	, dispatcher_()
	, renderer_(NULL)
	, widget_selected_(NULL)
	, shape_child_box_(NULL)
	, widget_child_box_(NULL) {
	log_call();

	loading_ = false;
	is_visible_ = false;
}


GPX2VideoWidgetFrame::GPX2VideoWidgetFrame(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &ref_builder) 
	: Glib::ObjectBase("GPX2VideoWidgetFrame")
	, Gtk::Frame(cobject)
	, ref_builder_(ref_builder) 
	, dispatcher_()
	, renderer_(NULL)
	, widget_selected_(NULL)
	, shape_child_box_(NULL)
	, widget_child_box_(NULL) {
	log_call();

	Glib::RefPtr<Gtk::ListStore> font_weight_model;

	loading_ = false;
	is_visible_ = false;

	// Populate models
	//-----------------

	orientation_model_ = Gtk::ListStore::create(model_);

	{
		auto iter = orientation_model_->append();
		auto row = *iter;
		row[model_.m_id] = VideoWidget::OrientationNone;
		row[model_.m_name] = "None";
		row[model_.m_enable] = true;

		row = *(orientation_model_->append());
		row[model_.m_id] = VideoWidget::OrientationHorizontal;
		row[model_.m_name] = "Horizontal";
		row[model_.m_enable] = true;

		row = *(orientation_model_->append());
		row[model_.m_id] = VideoWidget::OrientationVertical;
		row[model_.m_name] = "Vertical";
		row[model_.m_enable] = true;
	}

	shape_model_ = Gtk::ListStore::create(model_);

	{
		auto iter = shape_model_->append();
		auto row = *iter;
		row[model_.m_id] = VideoWidget::ShapeText;
		row[model_.m_name] = "Text";
		row[model_.m_enable] = true;

		row = *(shape_model_->append());
		row[model_.m_id] = VideoWidget::ShapeArc;
		row[model_.m_name] = "Arc";
		row[model_.m_enable] = true;

		row = *(shape_model_->append());
		row[model_.m_id] = VideoWidget::ShapeBar;
		row[model_.m_name] = "Bar";
		row[model_.m_enable] = true;
	}

	position_model_ = Gtk::ListStore::create(model_);

	{
		auto iter = position_model_->append();
		auto row = *iter;
		row[model_.m_id] = 0;
		row[model_.m_name] = "None";
		row[model_.m_enable] = true;

		row = *(position_model_->append());
		row[model_.m_id] = 1;
		row[model_.m_name] = "Left";
		row[model_.m_enable] = true;

		row = *(position_model_->append());
		row[model_.m_id] = 2;
		row[model_.m_name] = "Right";
		row[model_.m_enable] = true;

		row = *(position_model_->append());
		row[model_.m_id] = 3;
		row[model_.m_name] = "Bottom";
		row[model_.m_enable] = true;

		row = *(position_model_->append());
		row[model_.m_id] = 4;
		row[model_.m_name] = "Top";
		row[model_.m_enable] = true;

		row = *(position_model_->append());
		row[model_.m_id] = 5;
		row[model_.m_name] = "Bottom - Left";
		row[model_.m_enable] = true;

		row = *(position_model_->append());
		row[model_.m_id] = 6;
		row[model_.m_name] = "Bottom - Right";
		row[model_.m_enable] = true;

		row = *(position_model_->append());
		row[model_.m_id] = 7;
		row[model_.m_name] = "Top - Left";
		row[model_.m_enable] = true;

		row = *(position_model_->append());
		row[model_.m_id] = 8;
		row[model_.m_name] = "Top - Right";
		row[model_.m_enable] = true;
	}

	font_style_model_ = Gtk::ListStore::create(model_);

	{
		auto iter = font_style_model_->append();
		auto row = *iter;
		row[model_.m_id] = VideoWidget::Theme::FontStyleNormal;
		row[model_.m_name] = "Normal";
		row[model_.m_enable] = true;

		row = *(font_style_model_->append());
		row[model_.m_id] = VideoWidget::Theme::FontStyleItalic;
		row[model_.m_name] = "Italic";
		row[model_.m_enable] = true;
	}

	font_weight_model = Gtk::ListStore::create(model_);

	{
		auto iter = font_weight_model->append();
		auto row = *iter;
		row[model_.m_id] = VideoWidget::Theme::FontWeightThin;
		row[model_.m_name] = "Thin (100)";
		row[model_.m_enable] = true;

		row = *(font_weight_model->append());
		row[model_.m_id] = VideoWidget::Theme::FontWeightUltraLight;
		row[model_.m_name] = "Ultra light (200)";
		row[model_.m_enable] = true;

		row = *(font_weight_model->append());
		row[model_.m_id] = VideoWidget::Theme::FontWeightLight;
		row[model_.m_name] = "Light (300)";
		row[model_.m_enable] = true;

		row = *(font_weight_model->append());
		row[model_.m_id] = VideoWidget::Theme::FontWeightSemiLight;
		row[model_.m_name] = "Semi light (350)";
		row[model_.m_enable] = true;

		row = *(font_weight_model->append());
		row[model_.m_id] = VideoWidget::Theme::FontWeightBook;
		row[model_.m_name] = "Book (380)";
		row[model_.m_enable] = true;

		row = *(font_weight_model->append());
		row[model_.m_id] = VideoWidget::Theme::FontWeightNormal;
		row[model_.m_name] = "Normal (400)";
		row[model_.m_enable] = true;

		row = *(font_weight_model->append());
		row[model_.m_id] = VideoWidget::Theme::FontWeightMedium;
		row[model_.m_name] = "Medium (500)";
		row[model_.m_enable] = true;

		row = *(font_weight_model->append());
		row[model_.m_id] = VideoWidget::Theme::FontWeightSemiBold;
		row[model_.m_name] = "Semi bold (600)";
		row[model_.m_enable] = true;

		row = *(font_weight_model->append());
		row[model_.m_id] = VideoWidget::Theme::FontWeightUltraBold;
		row[model_.m_name] = "Ultra bold (800)";
		row[model_.m_enable] = true;

		row = *(font_weight_model->append());
		row[model_.m_id] = VideoWidget::Theme::FontWeightHeavy;
		row[model_.m_name] = "Heavy (900)";
		row[model_.m_enable] = true;

		row = *(font_weight_model->append());
		row[model_.m_id] = VideoWidget::Theme::FontWeightUltraHeavy;
		row[model_.m_name] = "Ultra heavy (1000)";
		row[model_.m_enable] = true;
	}

	text_align_model_ = Gtk::ListStore::create(model_);

	{
		auto iter = text_align_model_->append();
		auto row = *iter;
		row[model_.m_id] = VideoWidget::Theme::AlignLeft;
		row[model_.m_name] = "Left";
		row[model_.m_enable] = true;

		row = *(text_align_model_->append());
		row[model_.m_id] = VideoWidget::Theme::AlignCenter;
		row[model_.m_name] = "Center";
		row[model_.m_enable] = true;

		row = *(text_align_model_->append());
		row[model_.m_id] = VideoWidget::Theme::AlignRight;
		row[model_.m_name] = "Right";
		row[model_.m_enable] = true;
	}

	label_font_weight_model_ = duplicate_liststore(font_weight_model, model_);
	value_font_weight_model_ = duplicate_liststore(font_weight_model, model_);

	// Binding
	bind_content();

	// Update ui
	update_content();
}


GPX2VideoWidgetFrame::~GPX2VideoWidgetFrame() {
	log_call();
}


void GPX2VideoWidgetFrame::set_renderer(GPX2VideoRenderer *renderer) {
	log_call();

	renderer_ = renderer;
}


void GPX2VideoWidgetFrame::set_visible(bool visible) {
	log_call();

	is_visible_ = visible;

	update_content();
}


GPX2VideoWidget * GPX2VideoWidgetFrame::widget_selected(void) {
	log_call();

	return widget_selected_;
}


void GPX2VideoWidgetFrame::set_widget_selected(GPX2VideoWidget *widget) {
	log_call();

	// Unlisten widget events
	sigc_connection_.disconnect();

	// Save selected widget
	widget_selected_ = widget;

	if (widget) {
		// Listen widget events
		sigc_connection_ = widget_selected_->signal_changed().connect(sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_changed));
	}

	// Build extension settings ui component
	build_shape_settings();
	build_widget_settings();

	// Update ui content
	update_content();
}


void GPX2VideoWidgetFrame::build_shape_settings(void) {
	log_call();

	Gtk::Box *box;

	// Get settings box
	box = ref_builder_->get_widget<Gtk::Box>("settings_box");
	if (!box)
		throw std::runtime_error("No \"settings_box\" object in widget_frame.ui");

	// Remove previous shape settings box
	if (shape_child_box_) {
		box->remove(*shape_child_box_);

		shape_child_box_->release();
	}

	// Child box deleted
	shape_child_box_ = NULL;

	if (widget_selected_) {
		// Build shape settings box child
		switch (widget_selected_->widget()->shape()) {
		case VideoWidget::ShapeArc:
			shape_child_box_ = GPX2VideoArcShapeSettingsBox::create(widget_selected_);
			break;

		case VideoWidget::ShapeBar:
			shape_child_box_ = GPX2VideoBarShapeSettingsBox::create(widget_selected_);
			break;

		case VideoWidget::ShapeText:
			shape_child_box_ = GPX2VideoTextShapeSettingsBox::create(widget_selected_);
			break;

		default:
			break;
		}
	}

	// Append extension settings to ui
	if (shape_child_box_) {
		box->append(*shape_child_box_);
	}
}


void GPX2VideoWidgetFrame::build_widget_settings(void) {
	log_call();

	Gtk::Box *box;

	// Get settings box
	box = ref_builder_->get_widget<Gtk::Box>("settings_box");
	if (!box)
		throw std::runtime_error("No \"settings_box\" object in widget_frame.ui");

	// Remove previous widget settings box
	if (widget_child_box_) {
		box->remove(*widget_child_box_);

		widget_child_box_->release();
	}

	// Child box deleted
	widget_child_box_ = NULL;

	if (widget_selected_) {
		// Build widget settings box child
		switch (widget_selected_->widget()->type()) {
		case VideoWidget::WidgetMap:
			widget_child_box_ = GPX2VideoMapWidgetSettingsBox::create(widget_selected_);
			break;

		default:
			break;
		}
	}

	// Append extension settings to ui
	if (widget_child_box_) {
		box->append(*widget_child_box_);
	}
}


Glib::RefPtr<Gtk::ListStore> GPX2VideoWidgetFrame::duplicate_liststore(const Glib::RefPtr<Gtk::ListStore> &source, class Model &columns) {
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


bool GPX2VideoWidgetFrame::find_in_listtore(const Glib::RefPtr<Gtk::ListStore> &store, const int &value, Gtk::TreeModel::iterator &result) {
	log_call();

	for (auto iter = store->children().begin(); iter != store->children().end(); iter++) {
		if (iter->get_value(model_.m_id) != value)
			continue;

		result = iter;

		return true;
	}

	return false;
}

void GPX2VideoWidgetFrame::bind_content(void) {
	log_call();

	Gtk::Entry *entry;
	Gtk::Switch *sw;
	Gtk::ComboBox *combobox;
	Gtk::FontButton *fontbutton;
	Gtk::SpinButton *spinbutton;
	Gtk::ColorButton *colorbutton;

	auto renderer = Gtk::make_managed<Gtk::CellRendererText>();

	// Connect widgets button
	//------------------------

	// Shape
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("shape_combobox");
	if (!combobox)
		throw std::runtime_error("No \"shape_combobox\" object in widget_frame.ui");

	combobox->pack_start(*renderer, true);
	combobox->add_attribute(renderer->property_text(), model_.m_name);
	combobox->add_attribute(renderer->property_sensitive(), model_.m_enable);

//	combobox->pack_start(model_.m_id);
//	combobox->pack_start(model_.m_name);
	combobox->signal_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_combobox_changed), combobox, 
					[this](const Gtk::TreeModel::const_iterator &iter) {
						int value = iter->get_value(model_.m_id);

						log_info("Widget %s: shape changed to '%s'", 
								widget_selected_->widget()->name().c_str(), iter->get_value(model_.m_name).c_str());

						widget_selected_->widget()->setShape((VideoWidget::Shape) value);

						// Reload shape settings ui component
						build_shape_settings();

						// Widget shape settings
						update_shape_content();

						// Compute new position
						renderer_->compute();
					}
			));

	// X x Y
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("x_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"x_spinbutton\" object in widget_frame.ui");
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_spin_changed), spinbutton, 
					[this](const int &value) {
						int y = widget_selected_->widget()->y();

						log_info("Widget %s: position changed to '%dx%d'", 
								widget_selected_->widget()->name().c_str(), value, y);

						widget_selected_->widget()->setPosition(value, y);

						// Compute new position
						renderer_->compute();
					}
			));

	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("y_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"y_spinbutton\" object in widget_frame.ui");
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_spin_changed), spinbutton, 
					[this](const int &value) {
						int x = widget_selected_->widget()->x();

						log_info("Widget %s: position changed to '%dx%d'", 
								widget_selected_->widget()->name().c_str(), x, value);

						widget_selected_->widget()->setPosition(x, value);

						// Compute new position
						renderer_->compute();
					}
			));

	// Position
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("position_combobox");
	if (!combobox)
		throw std::runtime_error("No \"position_combobox\" object in widget_frame.ui");

	combobox->pack_start(*renderer, true);
	combobox->add_attribute(renderer->property_text(), model_.m_name);
	combobox->add_attribute(renderer->property_sensitive(), model_.m_enable);

//	combobox->pack_start(model_.m_id);
//	combobox->pack_start(model_.m_name);
	combobox->signal_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_combobox_changed), combobox, 
					[this](const Gtk::TreeModel::const_iterator &iter) {
						int value = iter->get_value(model_.m_id);

						log_info("Widget %s: position changed to '%s'", 
								widget_selected_->widget()->name().c_str(), iter->get_value(model_.m_name).c_str());

						widget_selected_->widget()->setPosition((VideoWidget::Position) value);

						// Compute new position
						renderer_->compute();
					}
			));

	// Orientation
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("orientation_combobox");
	if (!combobox)
		throw std::runtime_error("No \"orientationn_combobox\" object in widget_frame.ui");

	combobox->pack_start(*renderer, true);
	combobox->add_attribute(renderer->property_text(), model_.m_name);
	combobox->add_attribute(renderer->property_sensitive(), model_.m_enable);

//	combobox->pack_start(model_.m_id);
//	combobox->pack_start(model_.m_name);
	combobox->signal_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_combobox_changed), combobox, 
					[this](const Gtk::TreeModel::const_iterator &iter) {
						int value = iter->get_value(model_.m_id);

						log_info("Widget %s: orientation changed to '%s'", 
								widget_selected_->widget()->name().c_str(), iter->get_value(model_.m_name).c_str());

						widget_selected_->widget()->setOrientation((VideoWidget::Orientation) value);

						// Compute new position
						renderer_->compute();
					}
			));

	// Width x Height
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("width_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"width_spinbutton\" object in widget_frame.ui");
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_spin_changed), spinbutton, 
					[this](const int &value) {
						int height = widget_selected_->widget()->theme().height();

						log_info("Widget %s: size changed to '%dx%d'", 
								widget_selected_->widget()->name().c_str(), value, height);

						widget_selected_->setSize(value, height);

						// Compute new position
						renderer_->compute();
					}
			));

	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("height_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"height_spinbutton\" object in widget_frame.ui");
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_spin_changed), spinbutton, 
					[this](const int &value) {
						int width = widget_selected_->widget()->theme().width();

						log_info("Widget %s: size changed to '%dx%d'", 
								widget_selected_->widget()->name().c_str(), width, value);

						widget_selected_->setSize(width, value);

						// Compute new position
						renderer_->compute();
					}
			));

	// Margins
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("margin_left_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"margin_left_spinbutton\" object in widget_frame.ui");
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_margin_value_changed), VideoWidget::MarginLeft));

	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("margin_right_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"margin_right_spinbutton\" object in widget_frame.ui");
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_margin_value_changed), VideoWidget::MarginRight));

	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("margin_top_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"margin_top_spinbutton\" object in widget_frame.ui");
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_margin_value_changed), VideoWidget::MarginTop));

	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("margin_bottom_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"margin_bottom_spinbutton\" object in widget_frame.ui");
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_margin_value_changed), VideoWidget::MarginBottom));

	// Paddings
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("padding_left_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"padding_left_spinbutton\" object in widget_frame.ui");
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_padding_value_changed), VideoWidget::Theme::PaddingLeft));

	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("padding_right_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"padding_right_spinbutton\" object in widget_frame.ui");
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_padding_value_changed), VideoWidget::Theme::PaddingRight));

	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("padding_top_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"padding_top_spinbutton\" object in widget_frame.ui");
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_padding_value_changed), VideoWidget::Theme::PaddingTop));

	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("padding_bottom_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"padding_bottom_spinbutton\" object in widget_frame.ui");
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_padding_value_changed), VideoWidget::Theme::PaddingBottom));

	// Border width
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("border_width_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"border_width_spinbutton\" object in widget_frame.ui");
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_spin_changed), spinbutton, 
					[this](const int &value) {
						log_info("Widget %s: border changed to '%d'",
							   widget_selected_->widget()->name().c_str(), value);

						widget_selected_->widget()->theme().setBorder(value);
					}
			));

	// Border color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("border_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"border_color_button\" object in widget_frame.ui");
	colorbutton->signal_color_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_color_changed), colorbutton, 
					[this](const std::string &color) {
						log_info("Widget %s: border color changed to '%s'", 
								widget_selected_->widget()->name().c_str(), color.c_str());

						widget_selected_->widget()->theme().setBorderColor(color);
					}
			));

	// Background color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("background_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"background_color_button\" object in widget_frame.ui");
	colorbutton->signal_color_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_color_changed), colorbutton, 
					[this](const std::string &color) {
						log_info("Widget %s: background color changed to '%s'", 
								widget_selected_->widget()->name().c_str(), color.c_str());

						widget_selected_->widget()->theme().setBackgroundColor(color);
					}
			));

	// Label enable
	sw = ref_builder_->get_widget<Gtk::Switch>("label_enable_switch");
	if (!sw)
		throw std::runtime_error("No \"label_enable_switch\" object in widget_frame.ui");
	sw->signal_state_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_switch_changed), sw, 
					[this](const bool &state) {
						if (state)
							widget_selected_->widget()->theme().addFlag(VideoWidget::Theme::FlagLabel);
						else
							widget_selected_->widget()->theme().removeFlag(VideoWidget::Theme::FlagLabel);
					} 
			), false);

	// Label text entry
	entry = ref_builder_->get_widget<Gtk::Entry>("label_text_entry");
	if (!entry)
		throw std::runtime_error("No \"label_text_entry\" object in widget_frame.ui");
	entry->signal_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_entry_changed), entry, 
					[this](const Glib::ustring &value) {
						Gtk::FontButton *button;

						log_info("Widget %s: label text changed to '%s'",
							   widget_selected_->widget()->name().c_str(), value.c_str());

						// Update label font family button
						button = ref_builder_->get_widget<Gtk::FontButton>("label_font_family_fontbutton");

						button->set_preview_text(value);

						widget_selected_->widget()->setLabel(value);
					}
			));

	// Label font family
	fontbutton = ref_builder_->get_widget<Gtk::FontButton>("label_font_family_fontbutton");
	if (!fontbutton)
		throw std::runtime_error("No \"label_font_family_fontbutton\" object in widget_frame.ui");

	fontbutton->signal_font_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_font_changed), fontbutton, 
					[this](const Pango::FontDescription &description) {
						std::string value = description.get_family();

						log_info("Widget %s: label font family changed to '%s'",
							   widget_selected_->widget()->name().c_str(), value.c_str());

						// Update label font weight combobox
						update_font_weight_model(label_font_weight_model_, value);

						widget_selected_->widget()->theme().setLabelFontFamily(value);
					}
			));

	// Label font size
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("label_font_size_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"label_font_size_spinbutton\" object in widget_frame.ui");
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_spin_changed), spinbutton, 
					[this](const int &value) {
						Gtk::FontButton *button;

						Pango::FontDescription description;

						log_info("Widget %s: label font size changed to '%d'",
							   widget_selected_->widget()->name().c_str(), value);

						// Update label font family button
						button = ref_builder_->get_widget<Gtk::FontButton>("label_font_family_fontbutton");

						if (!button)
							throw std::runtime_error("No \"label_font_family_fontbutton\" object in widget_frame.ui");

						description = button->get_font_desc();

//						description.set_size(value * Pango::SCALE);
						description.set_absolute_size(value * Pango::SCALE);

						button->set_font_desc(description);

						widget_selected_->widget()->theme().setLabelFontSize(value);
					}
			));

	// Label shadow opacity
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("label_shadow_opacity_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"label_shadow_opacity_spinbutton\" object in widget_frame.ui");
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_spin_changed), spinbutton, 
					[this](const int &value) {
						log_info("Widget %s: label shadow opacity changed to '%d'",
							   widget_selected_->widget()->name().c_str(), value);

						widget_selected_->widget()->theme().setLabelShadowOpacity(value);
					}
			));

	// Label shadow distance
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("label_shadow_distance_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"label_shadow_distance_spinbutton\" object in widget_frame.ui");
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_spin_changed), spinbutton, 
					[this](const int &value) {
						log_info("Widget %s: label shadow distance changed to '%d'",
							   widget_selected_->widget()->name().c_str(), value);

						widget_selected_->widget()->theme().setLabelShadowDistance(value);
					}
			));

	// Label font style
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("label_fontstyle_combobox");
	if (!combobox)
		throw std::runtime_error("No \"label_fontstyle_combobox\" object in widget_frame.ui");

	combobox->pack_start(*renderer, true);
	combobox->add_attribute(renderer->property_text(), model_.m_name);
	combobox->add_attribute(renderer->property_sensitive(), model_.m_enable);

//	combobox->pack_start(model_.m_name);
	combobox->signal_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_combobox_changed), combobox, 
					[this](const Gtk::TreeModel::const_iterator &iter) {
						Gtk::FontButton *button;

						Pango::FontDescription description;

						int value = iter->get_value(model_.m_id);

						log_info("Widget %s: label style changed to '%s'", 
								widget_selected_->widget()->name().c_str(), iter->get_value(model_.m_name).c_str());

						// Update label font family button
						button = ref_builder_->get_widget<Gtk::FontButton>("label_font_family_fontbutton");

						if (!button)
							throw std::runtime_error("No \"label_font_family_fontbutton\" object in widget_frame.ui");

						description = button->get_font_desc();

						description.set_style((Pango::Style) value);

						button->set_font_desc(description);

						widget_selected_->widget()->theme().setLabelFontStyle((VideoWidget::Theme::FontStyle) value);
					}
			));

	// Label font weight
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("label_fontweight_combobox");
	if (!combobox)
		throw std::runtime_error("No \"label_fontweight_combobox\" object in widget_frame.ui");

	combobox->pack_start(*renderer, true);
	combobox->add_attribute(renderer->property_text(), model_.m_name);
	combobox->add_attribute(renderer->property_sensitive(), model_.m_enable);

//	combobox->pack_start(model_.m_name);
	combobox->signal_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_combobox_changed), combobox, 
					[this](const Gtk::TreeModel::const_iterator &iter) {
						Gtk::FontButton *button;

						Pango::FontDescription description;

						int value = iter->get_value(model_.m_id);

						log_info("Widget %s: label weight changed to '%s'", 
								widget_selected_->widget()->name().c_str(), iter->get_value(model_.m_name).c_str());

						// Update label font family button
						button = ref_builder_->get_widget<Gtk::FontButton>("label_font_family_fontbutton");

						if (!button)
							throw std::runtime_error("No \"label_font_family_fontbutton\" object in widget_frame.ui");

						description = button->get_font_desc();

						description.set_weight((Pango::Weight) value);

						button->set_font_desc(description);

						widget_selected_->widget()->theme().setLabelFontWeight((VideoWidget::Theme::FontWeight) value);
					}
			));

	// Label align
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("label_align_combobox");
	if (!combobox)
		throw std::runtime_error("No \"label_align_combobox\" object in widget_frame.ui");

	combobox->pack_start(*renderer, true);
	combobox->add_attribute(renderer->property_text(), model_.m_name);
	combobox->add_attribute(renderer->property_sensitive(), model_.m_enable);

//	combobox->pack_start(model_.m_name);
	combobox->signal_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_combobox_changed), combobox, 
					[this](const Gtk::TreeModel::const_iterator &iter) {
						int value = iter->get_value(model_.m_id);

						log_info("Widget %s: label align changed to '%s'", 
								widget_selected_->widget()->name().c_str(), iter->get_value(model_.m_name).c_str());

						widget_selected_->widget()->theme().setLabelAlign((VideoWidget::Theme::Align) value);
					}
			));

	// Label color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("label_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"label_color_button\" object in widget_frame.ui");
	colorbutton->signal_color_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_color_changed), colorbutton, 
					[this](const std::string &color) {
						log_info("Widget %s: label color changed to '%s'", 
								widget_selected_->widget()->name().c_str(), color.c_str());

						widget_selected_->widget()->theme().setLabelColor(color);
					}
			));

	// Label border width
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("label_border_width_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"label_border_width_spinbutton\" object in widget_frame.ui");
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_spin_changed), spinbutton, 
					[this](const int &value) {
						log_info("Widget %s: label border width changed to '%d'",
							   widget_selected_->widget()->name().c_str(), value);

						widget_selected_->widget()->theme().setLabelBorderWidth(value);
					}
			));

	// Label border color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("label_border_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"label_border_color_button\" object in widget_frame.ui");
	colorbutton->signal_color_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_color_changed), colorbutton, 
					[this](const std::string &color) {
						log_info("Widget %s: label border color changed to '%s'", 
								widget_selected_->widget()->name().c_str(), color.c_str());

						widget_selected_->widget()->theme().setLabelBorderColor(color);
					}
			));

	// Value enable
	sw = ref_builder_->get_widget<Gtk::Switch>("value_enable_switch");
	if (!sw)
		throw std::runtime_error("No \"value_enable_switch\" object in widget_frame.ui");
	sw->signal_state_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_switch_changed), sw, 
					[this](const bool &state) {
						if (state)
							widget_selected_->widget()->theme().addFlag(VideoWidget::Theme::FlagValue);
						else
							widget_selected_->widget()->theme().removeFlag(VideoWidget::Theme::FlagValue);
					} 
			), false);

	// Value font family
	fontbutton = ref_builder_->get_widget<Gtk::FontButton>("value_font_family_fontbutton");
	if (!fontbutton)
		throw std::runtime_error("No \"value_font_family_fontbutton\" object in widget_frame.ui");

	fontbutton->signal_font_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_font_changed), fontbutton, 
					[this](const Pango::FontDescription &description) {
						std::string value = description.get_family();

						log_info("Widget %s: value font family changed to '%s'",
							   widget_selected_->widget()->name().c_str(), value.c_str());

						// Update value font weight combobox
						update_font_weight_model(value_font_weight_model_, value);

						widget_selected_->widget()->theme().setValueFontFamily(value);
					}
			));

	// Value font size
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("value_font_size_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"value_font_size_spinbutton\" object in widget_frame.ui");
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_spin_changed), spinbutton, 
					[this](const int &value) {
						Gtk::FontButton *button;

						Pango::FontDescription description;

						log_info("Widget %s: value font size changed to '%d'",
							   widget_selected_->widget()->name().c_str(), value);

						// Update value font family button
						button = ref_builder_->get_widget<Gtk::FontButton>("value_font_family_fontbutton");

						if (!button)
							throw std::runtime_error("No \"value_font_family_fontbutton\" object in widget_frame.ui");

						description = button->get_font_desc();

//						description.set_size(value * Pango::SCALE);
						description.set_absolute_size(value * Pango::SCALE);

						button->set_font_desc(description);

						widget_selected_->widget()->theme().setValueFontSize(value);
					}
			));

	// Value shadow opacity
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("value_shadow_opacity_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"value_shadow_opacity_spinbutton\" object in widget_frame.ui");
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_spin_changed), spinbutton, 
					[this](const int &value) {
						log_info("Widget %s: value shadow opacity_changed to '%d'",
							   widget_selected_->widget()->name().c_str(), value);

						widget_selected_->widget()->theme().setValueShadowOpacity(value);
					}
			));

	// Value shadow distance
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("value_shadow_distance_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"value_shadow_distance_spinbutton\" object in widget_frame.ui");
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_spin_changed), spinbutton, 
					[this](const int &value) {
						log_info("Widget %s: value shadow distance_changed to '%d'",
							   widget_selected_->widget()->name().c_str(), value);

						widget_selected_->widget()->theme().setValueShadowDistance(value);
					}
			));

	// Value font style
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("value_fontstyle_combobox");
	if (!combobox)
		throw std::runtime_error("No \"value_fontstyle_combobox\" object in widget_frame.ui");

	combobox->pack_start(*renderer, true);
	combobox->add_attribute(renderer->property_text(), model_.m_name);
	combobox->add_attribute(renderer->property_sensitive(), model_.m_enable);

//	combobox->pack_start(model_.m_name);
	combobox->signal_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_combobox_changed), combobox, 
					[this](const Gtk::TreeModel::const_iterator &iter) {
						Gtk::FontButton *button;

						Pango::FontDescription description;

						int value = iter->get_value(model_.m_id);

						log_info("Widget %s: value style changed to '%s'", 
								widget_selected_->widget()->name().c_str(), iter->get_value(model_.m_name).c_str());

						// Update value font family button
						button = ref_builder_->get_widget<Gtk::FontButton>("value_font_family_fontbutton");

						if (!button)
							throw std::runtime_error("No \"value_font_family_fontbutton\" object in widget_frame.ui");

						description = button->get_font_desc();

						description.set_style((Pango::Style) value);

						button->set_font_desc(description);

						widget_selected_->widget()->theme().setValueFontStyle((VideoWidget::Theme::FontStyle) value);
					}
			));

	// Value font weight
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("value_fontweight_combobox");
	if (!combobox)
		throw std::runtime_error("No \"value_fontweight_combobox\" object in widget_frame.ui");

	combobox->pack_start(*renderer, true);
	combobox->add_attribute(renderer->property_text(), model_.m_name);
	combobox->add_attribute(renderer->property_sensitive(), model_.m_enable);

//	combobox->pack_start(model_.m_name);
	combobox->signal_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_combobox_changed), combobox, 
					[this](const Gtk::TreeModel::const_iterator &iter) {
						Gtk::FontButton *button;

						Pango::FontDescription description;

						int value = iter->get_value(model_.m_id);

						log_info("Widget %s: value weight changed to '%s'", 
								widget_selected_->widget()->name().c_str(), iter->get_value(model_.m_name).c_str());

						// Update value font family button
						button = ref_builder_->get_widget<Gtk::FontButton>("value_font_family_fontbutton");

						if (!button)
							throw std::runtime_error("No \"value_font_family_fontbutton\" object in widget_frame.ui");

						description = button->get_font_desc();

						description.set_weight((Pango::Weight) value);

						button->set_font_desc(description);

						widget_selected_->widget()->theme().setValueFontWeight((VideoWidget::Theme::FontWeight) value);
					}
			));

	// Value align
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("value_align_combobox");
	if (!combobox)
		throw std::runtime_error("No \"value_align_combobox\" object in widget_frame.ui");

	combobox->pack_start(*renderer, true);
	combobox->add_attribute(renderer->property_text(), model_.m_name);
	combobox->add_attribute(renderer->property_sensitive(), model_.m_enable);

//	combobox->pack_start(model_.m_name);
	combobox->signal_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_combobox_changed), combobox, 
					[this](const Gtk::TreeModel::const_iterator &iter) {
						int value = iter->get_value(model_.m_id);

						log_info("Widget %s: value align changed to '%s'", 
								widget_selected_->widget()->name().c_str(), iter->get_value(model_.m_name).c_str());

						widget_selected_->widget()->theme().setValueAlign((VideoWidget::Theme::Align) value);
					}
			));

	// Value color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("value_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"value_color_button\" object in widget_frame.ui");
	colorbutton->signal_color_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_color_changed), colorbutton, 
					[this](const std::string &color) {
						log_info("Widget %s: value color changed to '%s'", 
								widget_selected_->widget()->name().c_str(), color.c_str());

						widget_selected_->widget()->theme().setValueColor(color);
					}
			));

	// Value border width
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("value_border_width_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"value_border_width_spinbutton\" object in widget_frame.ui");
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_spin_changed), spinbutton, 
					[this](const int &value) {
						log_info("Widget %s: value border width changed to '%d'",
							   widget_selected_->widget()->name().c_str(), value);

						widget_selected_->widget()->theme().setValueBorderWidth(value);
					}
			));

	// Value border color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("value_border_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"value_border_color_button\" object in widget_frame.ui");
	colorbutton->signal_color_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_color_changed), colorbutton, 
					[this](const std::string &color) {
						log_info("Widget %s: value border color changed to '%s'", 
								widget_selected_->widget()->name().c_str(), color.c_str());

						widget_selected_->widget()->theme().setValueBorderColor(color);
					}
			));

	// Value min.
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("value_min_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"value_min_spinbutton\" object in widget_frame.ui");
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_spin_changed), spinbutton, 
					[this](const int &value) {
						log_info("Widget %s: value min. changed to '%d'",
							   widget_selected_->widget()->name().c_str(), value);

						widget_selected_->widget()->theme().setValueMin(value);
					}
			));

	// Value max.
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("value_max_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"value_max_spinbutton\" object in widget_frame.ui");
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_spin_changed), spinbutton, 
					[this](const int &value) {
						log_info("Widget %s: value max. changed to '%d'",
							   widget_selected_->widget()->name().c_str(), value);

						widget_selected_->widget()->theme().setValueMax(value);
					}
			));
}


void GPX2VideoWidgetFrame::update_content(void) {
	log_call();

	int margin;
	int width, height;

	Gdk::RGBA rgba;

	const float *color;

	Gtk::Entry *entry;
	Gtk::Switch *sw;
	Gtk::ComboBox *combobox;
	Gtk::FontButton *fontbutton;
	Gtk::SpinButton *spinbutton;
	Gtk::ColorButton *colorbutton;

	Pango::FontDescription description;

	Gtk::TreeModel::iterator iter;

	// Frame is visible only as widget is selected
	Gtk::Frame::set_visible(is_visible_ && (widget_selected_ != NULL));

	// No widget
	if (widget_selected_ == NULL)
		return;

	log_info("Load widget settings...");

	// Mask value changed
	loading_ = true;

	// Layout size
	width = renderer_->width();
	height = renderer_->height();

	// Widget shape
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("shape_combobox");
	if (!combobox)
		throw std::runtime_error("No \"shape_combobox\" object in widget_frame.ui");

	for (auto iter = shape_model_->children().begin(); iter != shape_model_->children().end(); iter++) {
		bool enable = widget_selected_->widget()->isShapeSupported((VideoWidget::Shape) iter->get_value(model_.m_id));

		iter->set_value(model_.m_enable, enable);
	}

	combobox->set_model(shape_model_);

	if (find_in_listtore(shape_model_, widget_selected_->widget()->shape(), iter))
		combobox->set_active(iter);

	// Widget X x Y
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("x_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"x_spinbutton\" object in widget_frame.ui");

	spinbutton->set_range(0, width);
	spinbutton->set_value(widget_selected_->widget()->x());
	spinbutton->set_sensitive(widget_selected_->widget()->position() == VideoWidget::PositionNone);

	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("y_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"y_spinbutton\" object in widget_frame.ui");

	spinbutton->set_range(0, height);
	spinbutton->set_value(widget_selected_->widget()->y());
	spinbutton->set_sensitive(widget_selected_->widget()->position() == VideoWidget::PositionNone);

	// Widget position
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("position_combobox");
	if (!combobox)
		throw std::runtime_error("No \"position_combobox\" object in widget_frame.ui");

	combobox->set_model(position_model_);

	if (find_in_listtore(position_model_, widget_selected_->widget()->position(), iter))
		combobox->set_active(iter);

	// Widget orientation
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("orientation_combobox");
	if (!combobox)
		throw std::runtime_error("No \"orientation_combobox\" object in widget_frame.ui");

	combobox->set_model(orientation_model_);

	if (find_in_listtore(orientation_model_, widget_selected_->widget()->orientation(), iter))
		combobox->set_active(iter);

	// Widget width
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("width_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"width_spinbutton\" object in widget_frame.ui");

	margin = widget_selected_->widget()->margin(VideoWidget::MarginLeft) 
		+ widget_selected_->widget()->margin(VideoWidget::MarginRight);

	spinbutton->set_range(0, width - margin);
	spinbutton->set_value(widget_selected_->widget()->theme().width());

	// Widget height
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("height_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"height_spinbutton\" object in widget_frame.ui");

	margin = widget_selected_->widget()->margin(VideoWidget::MarginTop) 
		+ widget_selected_->widget()->margin(VideoWidget::MarginBottom);

	spinbutton->set_range(0, height - margin);
	spinbutton->set_value(widget_selected_->widget()->theme().height());

	// Widget margin left
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("margin_left_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"margin_left_spinbutton\" object in widget_frame.ui");

	spinbutton->set_range(0, width - widget_selected_->widget()->theme().width());
	spinbutton->set_value(widget_selected_->widget()->margin(VideoWidget::MarginLeft));

	// Widget margin right
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("margin_right_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"margin_right_spinbutton\" object in widget_frame.ui");

	spinbutton->set_range(0, width - widget_selected_->widget()->theme().width());
	spinbutton->set_value(widget_selected_->widget()->margin(VideoWidget::MarginRight));

	// Widget margin top
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("margin_top_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"margin_top_spinbutton\" object in widget_frame.ui");

	spinbutton->set_range(0, height - widget_selected_->widget()->theme().height());
	spinbutton->set_value(widget_selected_->widget()->margin(VideoWidget::MarginTop));

	// Widget margin bottom
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("margin_bottom_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"margin_bottom_spinbutton\" object in widget_frame.ui");

	spinbutton->set_range(0, height - widget_selected_->widget()->theme().height());
	spinbutton->set_value(widget_selected_->widget()->margin(VideoWidget::MarginBottom));

	// Widget padding left
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("padding_left_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"padding_left_spinbutton\" object in widget_frame.ui");

	spinbutton->set_value(widget_selected_->widget()->theme().padding(VideoWidget::Theme::PaddingLeft));

	// Widget padding right
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("padding_right_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"padding_right_spinbutton\" object in widget_frame.ui");

	spinbutton->set_value(widget_selected_->widget()->theme().padding(VideoWidget::Theme::PaddingRight));

	// Widget padding top
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("padding_top_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"padding_top_spinbutton\" object in widget_frame.ui");

	spinbutton->set_value(widget_selected_->widget()->theme().padding(VideoWidget::Theme::PaddingTop));

	// Widget padding bottom
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("padding_bottom_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"padding_bottom_spinbutton\" object in widget_frame.ui");

	spinbutton->set_value(widget_selected_->widget()->theme().padding(VideoWidget::Theme::PaddingBottom));

	// Widget border width
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("border_width_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"border_width_spinbutton\" object in widget_frame.ui");

	spinbutton->set_value(widget_selected_->widget()->theme().border());

	// Widget border color button
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("border_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"border_color_button\" object in widget_frame.ui");

	color = widget_selected_->widget()->theme().borderColor();

	rgba.set_rgba(color[0], color[1], color[2], color[3]);

	colorbutton->set_rgba(rgba);

	// Widget background color button
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("background_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"background_color_button\" object in widget_frame.ui");

	color = widget_selected_->widget()->theme().backgroundColor();

	rgba.set_rgba(color[0], color[1], color[2], color[3]);

	colorbutton->set_rgba(rgba);

	// Widget label enable switch
	sw = ref_builder_->get_widget<Gtk::Switch>("label_enable_switch");
	if (!sw)
		throw std::runtime_error("No \"label_enable_switch\" object in widget_frame.ui");

	sw->set_active(widget_selected_->widget()->theme().hasFlag(VideoWidget::Theme::FlagLabel));

	// Widget label text entry
	entry = ref_builder_->get_widget<Gtk::Entry>("label_text_entry");
	if (!entry)
		throw std::runtime_error("No \"label_text_entry\" object in widget_frame.ui");

	entry->set_text(widget_selected_->widget()->label());

	// Label font family
	fontbutton = ref_builder_->get_widget<Gtk::FontButton>("label_font_family_fontbutton");
	if (!fontbutton)
		throw std::runtime_error("No \"label_font_family_fontbutton\" object in widget_frame.ui");

	description.set_family(widget_selected_->widget()->theme().labelFontFamily());
	description.set_style((Pango::Style) widget_selected_->widget()->theme().labelFontStyle());
	description.set_variant(Pango::Variant::NORMAL);
	description.set_weight((Pango::Weight) widget_selected_->widget()->theme().labelFontWeight());
	description.set_stretch(Pango::Stretch::NORMAL);
//	description.set_size(widget_selected_->widget()->theme().labelFontSize() * Pango::SCALE);
	description.set_absolute_size(widget_selected_->widget()->theme().labelFontSize() * Pango::SCALE);

	fontbutton->set_level(Gtk::FontButton::Level::FAMILY);
	fontbutton->set_preview_text(widget_selected_->widget()->label());
	fontbutton->set_font_desc(description);

	// Widget label size
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("label_font_size_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"label_font_size_spinbutton\" object in widget_frame.ui");

	spinbutton->set_value(widget_selected_->widget()->theme().labelFontSize());

	// Widget label shadow opacity
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("label_shadow_opacity_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"label_shadow_opacity_spinbutton\" object in widget_frame.ui");

	spinbutton->set_value(widget_selected_->widget()->theme().labelShadowOpacity());

	// Widget label shadow distance
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("label_shadow_distance_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"label_shadow_distance_spinbutton\" object in widget_frame.ui");

	spinbutton->set_value(widget_selected_->widget()->theme().labelShadowDistance());

	// Widget label font style
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("label_fontstyle_combobox");
	if (!combobox)
		throw std::runtime_error("No \"label_fontstyle_combobox\" object in widget_frame.ui");

	combobox->set_model(font_style_model_);

	if (find_in_listtore(font_style_model_, widget_selected_->widget()->theme().labelFontStyle(), iter))
		combobox->set_active(iter);

	// Widget label font weight
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("label_fontweight_combobox");
	if (!combobox)
		throw std::runtime_error("No \"label_fontweight_combobox\" object in widget_frame.ui");

	combobox->set_model(label_font_weight_model_);

	if (find_in_listtore(label_font_weight_model_, widget_selected_->widget()->theme().labelFontWeight(), iter))
		combobox->set_active(iter);

	// Widget label align
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("label_align_combobox");
	if (!combobox)
		throw std::runtime_error("No \"label_align_combobox\" object in widget_frame.ui");

	combobox->set_model(text_align_model_);

	if (find_in_listtore(text_align_model_, widget_selected_->widget()->theme().labelAlign(), iter))
		combobox->set_active(iter);

	// Widget label color button
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("label_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"label_color_button\" object in widget_frame.ui");

	color = widget_selected_->widget()->theme().labelColor();

	rgba.set_rgba(color[0], color[1], color[2], color[3]);

	colorbutton->set_rgba(rgba);

	// Widget label border width
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("label_border_width_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"label_border_width_spinbutton\" object in widget_frame.ui");

	spinbutton->set_value(widget_selected_->widget()->theme().labelBorderWidth());

	// Widget label border color button
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("label_border_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"label_border_color_button\" object in widget_frame.ui");

	color = widget_selected_->widget()->theme().labelBorderColor();

	rgba.set_rgba(color[0], color[1], color[2], color[3]);

	colorbutton->set_rgba(rgba);

	// Widget value enable switch
	sw = ref_builder_->get_widget<Gtk::Switch>("value_enable_switch");
	if (!sw)
		throw std::runtime_error("No \"value_enable_switch\" object in widget_frame.ui");

	sw->set_active(widget_selected_->widget()->theme().hasFlag(VideoWidget::Theme::FlagValue));

	// Value font family
	fontbutton = ref_builder_->get_widget<Gtk::FontButton>("value_font_family_fontbutton");
	if (!fontbutton)
		throw std::runtime_error("No \"value_font_family_fontbutton\" object in widget_frame.ui");

	description.set_family(widget_selected_->widget()->theme().valueFontFamily());
	description.set_style((Pango::Style) widget_selected_->widget()->theme().valueFontStyle());
	description.set_variant(Pango::Variant::NORMAL);
	description.set_weight((Pango::Weight) widget_selected_->widget()->theme().valueFontWeight());
	description.set_stretch(Pango::Stretch::NORMAL);
//	description.set_size(widget_selected_->widget()->theme().valueFontSize() * Pango::SCALE);
	description.set_absolute_size(widget_selected_->widget()->theme().valueFontSize() * Pango::SCALE);

	fontbutton->set_level(Gtk::FontButton::Level::FAMILY);
	fontbutton->set_font_desc(description);

	// Widget value size
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("value_font_size_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"value_font_size_spinbutton\" object in widget_frame.ui");

	spinbutton->set_value(widget_selected_->widget()->theme().valueFontSize());

	// Widget value shadow opacity
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("value_shadow_opacity_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"value_shadow_opacity_spinbutton\" object in widget_frame.ui");

	spinbutton->set_value(widget_selected_->widget()->theme().valueShadowOpacity());

	// Widget value shadow distance
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("value_shadow_distance_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"value_shadow_distance_spinbutton\" object in widget_frame.ui");

	spinbutton->set_value(widget_selected_->widget()->theme().valueShadowDistance());

	// Widget value font style
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("value_fontstyle_combobox");
	if (!combobox)
		throw std::runtime_error("No \"value_fontstyle_combobox\" object in widget_frame.ui");

	combobox->set_model(font_style_model_);

	if (find_in_listtore(font_style_model_, widget_selected_->widget()->theme().valueFontStyle(), iter))
		combobox->set_active(iter);

	// Widget value font weight
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("value_fontweight_combobox");
	if (!combobox)
		throw std::runtime_error("No \"value_fontweight_combobox\" object in widget_frame.ui");

	combobox->set_model(value_font_weight_model_);

	if (find_in_listtore(value_font_weight_model_, widget_selected_->widget()->theme().valueFontWeight(), iter))
		combobox->set_active(iter);

	// Widget value align
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("value_align_combobox");
	if (!combobox)
		throw std::runtime_error("No \"value_align_combobox\" object in widget_frame.ui");

	combobox->set_model(text_align_model_);

	if (find_in_listtore(text_align_model_, widget_selected_->widget()->theme().valueAlign(), iter))
		combobox->set_active(iter);

	// Widget value color button
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("value_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"value_color_button\" object in widget_frame.ui");

	color = widget_selected_->widget()->theme().valueColor();

	rgba.set_rgba(color[0], color[1], color[2], color[3]);

	colorbutton->set_rgba(rgba);

	// Widget value border width
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("value_border_width_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"value_border_width_spinbutton\" object in widget_frame.ui");

	spinbutton->set_value(widget_selected_->widget()->theme().valueBorderWidth());

	// Widget value border color button
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("value_border_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"value_border_color_button\" object in widget_frame.ui");

	color = widget_selected_->widget()->theme().valueBorderColor();

	rgba.set_rgba(color[0], color[1], color[2], color[3]);

	colorbutton->set_rgba(rgba);

	// Widget value min.
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("value_min_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"value_min_spinbutton\" object in widget_frame.ui");

	spinbutton->set_value(widget_selected_->widget()->theme().valueMin());

	// Widget value max.
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("value_max_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"value_max_spinbutton\" object in widget_frame.ui");

	spinbutton->set_value(widget_selected_->widget()->theme().valueMax());

	// Widget settings
	update_shape_content();
	update_widget_content();

	log_info("Widget settings loaded");

	// Unmask value changed
	loading_ = false;
}


void GPX2VideoWidgetFrame::update_boundaries(void) {
	int margin;
	int width, height;

	Gtk::SpinButton *spinbutton;

	log_call();

	// Layout size
	width = renderer_->width();
	height = renderer_->height();

	// Update UI range for widget width
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("width_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"width_spinbutton\" object in widget_frame.ui");

	margin = widget_selected_->widget()->margin(VideoWidget::MarginLeft) 
		+ widget_selected_->widget()->margin(VideoWidget::MarginRight);

	spinbutton->set_range(0, width - margin);

	// Update UI range for widget height
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("height_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"height_spinbutton\" object in widget_frame.ui");

	margin = widget_selected_->widget()->margin(VideoWidget::MarginTop) 
		+ widget_selected_->widget()->margin(VideoWidget::MarginBottom);

	spinbutton->set_range(0, height - margin);
}


void GPX2VideoWidgetFrame::update_shape_content(void) {
	log_call();

	if (shape_child_box_)
		shape_child_box_->update_content();
}


void GPX2VideoWidgetFrame::update_widget_content(void) {
	log_call();

	if (widget_child_box_)
		widget_child_box_->update_content();
}


void GPX2VideoWidgetFrame::update_font_weight_model(const Glib::RefPtr<Gtk::ListStore> &store, const std::string &value) {
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


void GPX2VideoWidgetFrame::on_widget_margin_value_changed(const VideoWidget::Margin &margin) {
	log_call();

	int value;

	const char *margins[] = {
		"",
		"margin_left_spinbutton",
		"margin_right_spinbutton",
		"margin_bottom_spinbutton",
		"margin_top_spinbutton"
	};

	if (loading_)
		return;

	// Widget margin
	auto spinbutton = ref_builder_->get_widget<Gtk::SpinButton>(margins[margin]);
	if (!spinbutton) {
		std::string str = std::string("No \"") + std::string(margins[margin]) + std::string("\" object in widget_frame.ui");
		throw std::runtime_error(str);
	}

	value = spinbutton->get_value_as_int();

	log_info("Widget %s: margin changed to '%d'", 
			widget_selected_->widget()->name().c_str(), value);

	widget_selected_->widget()->setMargin(margin, value);
	renderer_->refresh(widget_selected_);

	// Update limit
	update_boundaries();

	// Compute new position
	renderer_->compute();

	// Refresh video preview
	dispatcher_.emit();
}


void GPX2VideoWidgetFrame::on_widget_padding_value_changed(const VideoWidget::Theme::Padding &padding) {
	log_call();

	int value;

	const char *paddings[] = {
		"",
		"padding_left_spinbutton",
		"padding_right_spinbutton",
		"padding_bottom_spinbutton",
		"padding_top_spinbutton"
	};

	if (loading_)
		return;

	// Widget padding
	auto spinbutton = ref_builder_->get_widget<Gtk::SpinButton>(paddings[padding]);
	if (!spinbutton) {
		std::string str = std::string("No \"") + std::string(paddings[padding]) + std::string("\" object in widget_frame.ui");
		throw std::runtime_error(str);
	}

	value = spinbutton->get_value_as_int();

	log_info("Widget %s: padding changed to '%d'", 
			widget_selected_->widget()->name().c_str(), value);

	widget_selected_->widget()->theme().setPadding(padding, value);
	renderer_->refresh(widget_selected_);

	// Refresh video preview
	dispatcher_.emit();
}


void GPX2VideoWidgetFrame::on_widget_font_changed(Gtk::FontButton *button, std::function<void(const Pango::FontDescription&)> set) {
	log_call();

	Pango::FontDescription description;

	if (loading_)
		return;

	description = button->get_font_desc();

	// Set description
	set(description);

	// Refresh widget
	renderer_->refresh(widget_selected_);

	// Refresh video preview
	dispatcher_.emit();
}


void GPX2VideoWidgetFrame::on_widget_spin_changed(Gtk::SpinButton *button, std::function<void(const int&)> set) {
	log_call();

	int value;

	if (loading_)
		return;

	value = button->get_value_as_int();

	// Set value
	set(value);

	// Refresh widget
	renderer_->refresh(widget_selected_);

	// Refresh video preview
	dispatcher_.emit();
}


void GPX2VideoWidgetFrame::on_widget_color_changed(Gtk::ColorButton *button, std::function<void(const std::string&)> set) {
	log_call();

	Gdk::RGBA rgba;

	Glib::ustring color;

	if (loading_)
		return;

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

	// Refresh widget
	renderer_->refresh(widget_selected_);

	// Refresh video preview
	dispatcher_.emit();
}


void GPX2VideoWidgetFrame::on_widget_entry_changed(Gtk::Entry *entry, std::function<void(const Glib::ustring&)> set) {
	log_call();

	if (loading_)
		return;

	// Set entry
	set(entry->get_text());

	// Refresh widget
	renderer_->refresh(widget_selected_);

	// Refresh video preview
	dispatcher_.emit();
}


void GPX2VideoWidgetFrame::on_widget_combobox_changed(Gtk::ComboBox *combobox, std::function<void(const Gtk::TreeModel::const_iterator&)> set) {
	log_call();

	if (loading_)
		return;

	// Set combobox
	set(combobox->get_active());

	// Refresh widget
	renderer_->refresh(widget_selected_);

	// Refresh video preview
	dispatcher_.emit();
}

bool GPX2VideoWidgetFrame::on_widget_switch_changed(bool state, Gtk::Switch *sw, std::function<void(const bool&)> set) {
	log_call();

	if (loading_)
		return false;

	// Text enable
	sw->set_state(state);

	// Set state
	set(state);

	// Refresh widget
	renderer_->refresh(widget_selected_);

	// Refresh video preview
	dispatcher_.emit();

	return true;
}


void GPX2VideoWidgetFrame::on_widget_changed(void) {
	log_call();

	log_info("Widget extension properties changed");

	// Refresh widget required
	renderer_->refresh(widget_selected_);

	// Refresh video preview
	dispatcher_.emit();
}

