#include <limits>

#include <glibmm/i18n.h>

#include <gtkmm/box.h>
#include <gtkmm/entry.h>
#include <gtkmm/switch.h>
#include <gtkmm/combobox.h>
#include <gtkmm/expander.h>
#include <gtkmm/fontbutton.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/colorbutton.h>

#include <pangomm/cairofontmap.h>
#include <pangomm/fontdescription.h>

#include "log_i.h"
#include "shape/arc.h"
#include "shape/bar.h"
#include "shape/text.h"
#include "widgets/image.h"
#include "widgets/map.h"
#include "widgets/text.h"
#include "widgets/track.h"
#include "widgetframe.h"



GPX2VideoWidgetFrame::GPX2VideoWidgetFrame()
	: Glib::ObjectBase("GPX2VideoWidgetFrame")
	, ref_builder_(NULL) 
	, dispatcher_()
	, media_model_(NULL)
	, renderer_(NULL)
	, widget_selected_(NULL)
	, shape_child_box_(NULL)
	, widget_child_box_(NULL) {
	log_call();

	loading_ = false;
	is_visible_ = false;
}


GPX2VideoWidgetFrame::GPX2VideoWidgetFrame(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &ref_builder,
		const Glib::RefPtr<GPX2VideoMediaListStore> &media_model) 
	: Glib::ObjectBase("GPX2VideoWidgetFrame")
	, Gtk::Frame(cobject)
	, ref_builder_(ref_builder) 
	, dispatcher_()
	, media_model_(media_model)
	, renderer_(NULL)
	, widget_selected_(NULL)
	, shape_child_box_(NULL)
	, widget_child_box_(NULL) {
	log_call();

	loading_ = false;
	is_visible_ = false;

	// Populate models
	load_models();

	// Binding
	bind_content();

	// Update ui
	update_content();
	update_boundaries();
}


void GPX2VideoWidgetFrame::load_models(void) {
	log_call();

	Glib::RefPtr<Gtk::ListStore> font_weight_model;

	orientation_model_ = Gtk::ListStore::create(model_);

	{
		auto iter = orientation_model_->append();
		auto row = *iter;
		row[model_.m_id] = VideoWidget::OrientationNone;
		row[model_.m_name] = _("None");
		row[model_.m_enable] = true;

		row = *(orientation_model_->append());
		row[model_.m_id] = VideoWidget::OrientationHorizontal;
		row[model_.m_name] = _("Horizontal");
		row[model_.m_enable] = true;

		row = *(orientation_model_->append());
		row[model_.m_id] = VideoWidget::OrientationVertical;
		row[model_.m_name] = _("Vertical");
		row[model_.m_enable] = true;
	}

	shape_model_ = Gtk::ListStore::create(model_);

	position_model_ = Gtk::ListStore::create(model_);

	{
		auto iter = position_model_->append();
		auto row = *iter;
		row[model_.m_id] = 0;
		row[model_.m_name] = _("None");
		row[model_.m_enable] = true;

		row = *(position_model_->append());
		row[model_.m_id] = 1;
		row[model_.m_name] = _("Left");
		row[model_.m_enable] = true;

		row = *(position_model_->append());
		row[model_.m_id] = 2;
		row[model_.m_name] = _("Right");
		row[model_.m_enable] = true;

		row = *(position_model_->append());
		row[model_.m_id] = 3;
		row[model_.m_name] = _("Bottom");
		row[model_.m_enable] = true;

		row = *(position_model_->append());
		row[model_.m_id] = 4;
		row[model_.m_name] = _("Top");
		row[model_.m_enable] = true;

		row = *(position_model_->append());
		row[model_.m_id] = 5;
		row[model_.m_name] = _("Bottom - Left");
		row[model_.m_enable] = true;

		row = *(position_model_->append());
		row[model_.m_id] = 6;
		row[model_.m_name] = _("Bottom - Right");
		row[model_.m_enable] = true;

		row = *(position_model_->append());
		row[model_.m_id] = 7;
		row[model_.m_name] = _("Top - Left");
		row[model_.m_enable] = true;

		row = *(position_model_->append());
		row[model_.m_id] = 8;
		row[model_.m_name] = _("Top - Right");
		row[model_.m_enable] = true;
	}

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

	value_format_model_ = Gtk::ListStore::create(model_);

	value_unit_model_ = Gtk::ListStore::create(model_);

	label_font_weight_model_ = duplicate_liststore(font_weight_model, model_);
	value_font_weight_model_ = duplicate_liststore(font_weight_model, model_);
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
	update_boundaries();
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
//		switch (widget_selected_->widget()->shape()) {
		switch (widget_selected_->shape()->type()) {
		case VideoWidget::ShapeArc:
			shape_child_box_ = GPX2VideoArcShapeSettingsBox::create(widget_selected_);
			break;

		case VideoWidget::ShapeBar:
			shape_child_box_ = GPX2VideoBarShapeSettingsBox::create(widget_selected_);
			break;

		case VideoWidget::ShapeText:
			shape_child_box_ = GPX2VideoTextShapeSettingsBox::create(widget_selected_, media_model_);
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
		case VideoWidget::WidgetImage:
			widget_child_box_ = GPX2VideoImageWidgetSettingsBox::create(widget_selected_, media_model_);
			break;

		case VideoWidget::WidgetMap:
			widget_child_box_ = GPX2VideoMapWidgetSettingsBox::create(widget_selected_, media_model_);
			break;

		case VideoWidget::WidgetText:
			widget_child_box_ = GPX2VideoTextWidgetSettingsBox::create(widget_selected_);
			break;

		case VideoWidget::WidgetTrack:
			widget_child_box_ = GPX2VideoTrackWidgetSettingsBox::create(widget_selected_, media_model_);
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

	combobox->signal_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_combobox_changed), combobox, 
					[this](const Gtk::TreeModel::const_iterator &iter) {
						int value = iter->get_value(model_.m_id);

						log_notice("Widget %s: shape changed to '%s'", 
								widget_selected_->widget()->name().c_str(), iter->get_value(model_.m_name).c_str());

						widget_selected_->widget()->setShape((VideoWidget::Shape) value);

						// Reload shape settings ui component
						build_shape_settings();

						// Widget shape settings
						update_shape_content();
						update_boundaries();

						// Compute new position
						renderer_->compute();

						// Broadcast widget change
						widget_selected_->dispatchEvent(false);
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

						log_notice("Widget %s: position changed to '%dx%d'", 
								widget_selected_->widget()->name().c_str(), value, y);

						widget_selected_->widget()->setPosition(value, y);

						// Broadcast widget change
						widget_selected_->dispatchEvent(false);
					}
			));

	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("y_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"y_spinbutton\" object in widget_frame.ui");
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_spin_changed), spinbutton, 
					[this](const int &value) {
						int x = widget_selected_->widget()->x();

						log_notice("Widget %s: position changed to '%dx%d'", 
								widget_selected_->widget()->name().c_str(), x, value);

						widget_selected_->widget()->setPosition(x, value);

						// Broadcast widget change
						widget_selected_->dispatchEvent(false);
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

						log_notice("Widget %s: position changed to '%s'", 
								widget_selected_->widget()->name().c_str(), iter->get_value(model_.m_name).c_str());

						widget_selected_->widget()->setPosition((VideoWidget::Position) value);

						// Compute new position
						renderer_->compute();

						// Broadcast widget change
						widget_selected_->dispatchEvent(false);
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

						log_notice("Widget %s: orientation changed to '%s'", 
								widget_selected_->widget()->name().c_str(), iter->get_value(model_.m_name).c_str());

						widget_selected_->widget()->setOrientation((VideoWidget::Orientation) value);

						// Compute new position
						renderer_->compute();

						// Broadcast widget change
						widget_selected_->dispatchEvent(false);
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

						log_notice("Widget %s: size changed to '%dx%d'", 
								widget_selected_->widget()->name().c_str(), value, height);

						widget_selected_->setSize(value, height);

						// Compute new position
						renderer_->compute();

						// Broadcast widget change
						widget_selected_->dispatchEvent(true);
					}
			));

	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("height_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"height_spinbutton\" object in widget_frame.ui");
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_spin_changed), spinbutton, 
					[this](const int &value) {
						int width = widget_selected_->widget()->theme().width();

						log_notice("Widget %s: size changed to '%dx%d'", 
								widget_selected_->widget()->name().c_str(), width, value);

						widget_selected_->setSize(width, value);

						// Compute new position
						renderer_->compute();

						// Broadcast widget change
						widget_selected_->dispatchEvent(true);
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
						log_notice("Widget %s: border changed to '%d'",
							   widget_selected_->widget()->name().c_str(), value);

						widget_selected_->widget()->theme().setBorder(value);

						// Broadcast widget change
						widget_selected_->dispatchEvent(true);
					}
			));

	// Border color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("border_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"border_color_button\" object in widget_frame.ui");
	colorbutton->signal_color_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_color_changed), colorbutton, 
					[this](const std::string &color) {
						log_notice("Widget %s: border color changed to '%s'", 
								widget_selected_->widget()->name().c_str(), color.c_str());

						widget_selected_->widget()->theme().setBorderColor(color);

						// Broadcast widget change
						widget_selected_->dispatchEvent(false);
					}
			));

	// Background color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("background_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"background_color_button\" object in widget_frame.ui");
	colorbutton->signal_color_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_color_changed), colorbutton, 
					[this](const std::string &color) {
						log_notice("Widget %s: background color changed to '%s'", 
								widget_selected_->widget()->name().c_str(), color.c_str());

						widget_selected_->widget()->theme().setBackgroundColor(color);

						// Broadcast widget change
						widget_selected_->dispatchEvent(false);
					}
			));

	// Label enable
	sw = ref_builder_->get_widget<Gtk::Switch>("label_enable_switch");
	if (!sw)
		throw std::runtime_error("No \"label_enable_switch\" object in widget_frame.ui");
	sw->signal_state_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_switch_changed), sw, 
					[this](const bool &state) {
						log_notice("Widget %s: label status changed to '%s'",
							   widget_selected_->widget()->name().c_str(), state ? "enabled" : "disabled");

						if (state)
							widget_selected_->widget()->theme().addFlag(VideoWidget::Theme::FlagLabel);
						else
							widget_selected_->widget()->theme().removeFlag(VideoWidget::Theme::FlagLabel);

						// Broadcast widget change
						widget_selected_->dispatchEvent(false);
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

						log_notice("Widget %s: label text changed to '%s'",
							   widget_selected_->widget()->name().c_str(), value.c_str());

						// Update label font family button
						button = ref_builder_->get_widget<Gtk::FontButton>("label_font_family_fontbutton");

						button->set_preview_text(value);

						widget_selected_->widget()->setLabel(value);

						// Broadcast widget change
						widget_selected_->dispatchEvent(false);
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

						log_notice("Widget %s: label font family changed to '%s'",
							   widget_selected_->widget()->name().c_str(), value.c_str());

						// Update label font weight combobox
						update_font_weight_model(label_font_weight_model_, value);

						widget_selected_->widget()->theme().setLabelFontFamily(value);

						// Broadcast widget change
						widget_selected_->dispatchEvent(false);
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

						log_notice("Widget %s: label font size changed to '%d'",
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

						// Broadcast widget change
						widget_selected_->dispatchEvent(false);
					}
			));

	// Label shadow opacity
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("label_shadow_opacity_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"label_shadow_opacity_spinbutton\" object in widget_frame.ui");
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_spin_changed), spinbutton, 
					[this](const int &value) {
						log_notice("Widget %s: label shadow opacity changed to '%d'",
							   widget_selected_->widget()->name().c_str(), value);

						widget_selected_->widget()->theme().setLabelShadowOpacity(value);

						// Broadcast widget change
						widget_selected_->dispatchEvent(false);
					}
			));

	// Label shadow distance
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("label_shadow_distance_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"label_shadow_distance_spinbutton\" object in widget_frame.ui");
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_spin_changed), spinbutton, 
					[this](const int &value) {
						log_notice("Widget %s: label shadow distance changed to '%d'",
							   widget_selected_->widget()->name().c_str(), value);

						widget_selected_->widget()->theme().setLabelShadowDistance(value);

						// Broadcast widget change
						widget_selected_->dispatchEvent(false);
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

						log_notice("Widget %s: label style changed to '%s'", 
								widget_selected_->widget()->name().c_str(), iter->get_value(model_.m_name).c_str());

						// Update label font family button
						button = ref_builder_->get_widget<Gtk::FontButton>("label_font_family_fontbutton");

						if (!button)
							throw std::runtime_error("No \"label_font_family_fontbutton\" object in widget_frame.ui");

						description = button->get_font_desc();

						description.set_style((Pango::Style) value);

						button->set_font_desc(description);

						widget_selected_->widget()->theme().setLabelFontStyle((VideoWidget::Theme::FontStyle) value);

						// Broadcast widget change
						widget_selected_->dispatchEvent(false);
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

						log_notice("Widget %s: label weight changed to '%s'", 
								widget_selected_->widget()->name().c_str(), iter->get_value(model_.m_name).c_str());

						// Update label font family button
						button = ref_builder_->get_widget<Gtk::FontButton>("label_font_family_fontbutton");

						if (!button)
							throw std::runtime_error("No \"label_font_family_fontbutton\" object in widget_frame.ui");

						description = button->get_font_desc();

						description.set_weight((Pango::Weight) value);

						button->set_font_desc(description);

						widget_selected_->widget()->theme().setLabelFontWeight((VideoWidget::Theme::FontWeight) value);

						// Broadcast widget change
						widget_selected_->dispatchEvent(false);
					}
			));

	// Label horizontal align
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("label_horizontal_align_combobox");
	if (!combobox)
		throw std::runtime_error("No \"label_horizontal_align_combobox\" object in widget_frame.ui");

	combobox->pack_start(*renderer, true);
	combobox->add_attribute(renderer->property_text(), model_.m_name);
	combobox->add_attribute(renderer->property_sensitive(), model_.m_enable);

//	combobox->pack_start(model_.m_name);
	combobox->signal_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_combobox_changed), combobox, 
					[this](const Gtk::TreeModel::const_iterator &iter) {
						int value = iter->get_value(model_.m_id);

						log_notice("Widget %s: label horizontal align changed to '%s'", 
								widget_selected_->widget()->name().c_str(), iter->get_value(model_.m_name).c_str());

						widget_selected_->widget()->theme().setLabelHorizontalAlign((VideoWidget::Theme::Align) value);

						// Broadcast widget change
						widget_selected_->dispatchEvent(false);
					}
			));

	// Label vertical align
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("label_vertical_align_combobox");
	if (!combobox)
		throw std::runtime_error("No \"label_vertical_align_combobox\" object in widget_frame.ui");

	combobox->pack_start(*renderer, true);
	combobox->add_attribute(renderer->property_text(), model_.m_name);
	combobox->add_attribute(renderer->property_sensitive(), model_.m_enable);

//	combobox->pack_start(model_.m_name);
	combobox->signal_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_combobox_changed), combobox, 
					[this](const Gtk::TreeModel::const_iterator &iter) {
						int value = iter->get_value(model_.m_id);

						log_notice("Widget %s: label vertical align changed to '%s'", 
								widget_selected_->widget()->name().c_str(), iter->get_value(model_.m_name).c_str());

						widget_selected_->widget()->theme().setLabelVerticalAlign((VideoWidget::Theme::Align) value);

						// Broadcast widget change
						widget_selected_->dispatchEvent(false);
					}
			));

	// Label color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("label_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"label_color_button\" object in widget_frame.ui");
	colorbutton->signal_color_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_color_changed), colorbutton, 
					[this](const std::string &color) {
						log_notice("Widget %s: label color changed to '%s'", 
								widget_selected_->widget()->name().c_str(), color.c_str());

						widget_selected_->widget()->theme().setLabelColor(color);

						// Broadcast widget change
						widget_selected_->dispatchEvent(false);
					}
			));

	// Label border width
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("label_border_width_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"label_border_width_spinbutton\" object in widget_frame.ui");
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_spin_changed), spinbutton, 
					[this](const int &value) {
						log_notice("Widget %s: label border width changed to '%d'",
							   widget_selected_->widget()->name().c_str(), value);

						widget_selected_->widget()->theme().setLabelBorderWidth(value);

						// Broadcast widget change
						widget_selected_->dispatchEvent(false);
					}
			));

	// Label border color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("label_border_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"label_border_color_button\" object in widget_frame.ui");
	colorbutton->signal_color_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_color_changed), colorbutton, 
					[this](const std::string &color) {
						log_notice("Widget %s: label border color changed to '%s'", 
								widget_selected_->widget()->name().c_str(), color.c_str());

						widget_selected_->widget()->theme().setLabelBorderColor(color);

						// Broadcast widget change
						widget_selected_->dispatchEvent(false);
					}
			));

	// Value enable
	sw = ref_builder_->get_widget<Gtk::Switch>("value_enable_switch");
	if (!sw)
		throw std::runtime_error("No \"value_enable_switch\" object in widget_frame.ui");
	sw->signal_state_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_switch_changed), sw, 
					[this](const bool &state) {
						log_notice("Widget %s: value status changed to '%s'",
							   widget_selected_->widget()->name().c_str(), state ? "enabled" : "disabled");

						if (state)
							widget_selected_->widget()->theme().addFlag(VideoWidget::Theme::FlagValue);
						else
							widget_selected_->widget()->theme().removeFlag(VideoWidget::Theme::FlagValue);

						// Broadcast widget change
						widget_selected_->dispatchEvent(false);
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

						log_notice("Widget %s: value font family changed to '%s'",
							   widget_selected_->widget()->name().c_str(), value.c_str());

						// Update value font weight combobox
						update_font_weight_model(value_font_weight_model_, value);

						widget_selected_->widget()->theme().setValueFontFamily(value);

						// Broadcast widget change
						widget_selected_->dispatchEvent(false);
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

						log_notice("Widget %s: value font size changed to '%d'",
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

						// Broadcast widget change
						widget_selected_->dispatchEvent(false);
					}
			));

	// Value shadow opacity
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("value_shadow_opacity_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"value_shadow_opacity_spinbutton\" object in widget_frame.ui");
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_spin_changed), spinbutton, 
					[this](const int &value) {
						log_notice("Widget %s: value shadow opacity_changed to '%d'",
							   widget_selected_->widget()->name().c_str(), value);

						widget_selected_->widget()->theme().setValueShadowOpacity(value);

						// Broadcast widget change
						widget_selected_->dispatchEvent(false);
					}
			));

	// Value shadow distance
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("value_shadow_distance_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"value_shadow_distance_spinbutton\" object in widget_frame.ui");
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_spin_changed), spinbutton, 
					[this](const int &value) {
						log_notice("Widget %s: value shadow distance_changed to '%d'",
							   widget_selected_->widget()->name().c_str(), value);

						widget_selected_->widget()->theme().setValueShadowDistance(value);

						// Broadcast widget change
						widget_selected_->dispatchEvent(false);
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

						log_notice("Widget %s: value style changed to '%s'", 
								widget_selected_->widget()->name().c_str(), iter->get_value(model_.m_name).c_str());

						// Update value font family button
						button = ref_builder_->get_widget<Gtk::FontButton>("value_font_family_fontbutton");

						if (!button)
							throw std::runtime_error("No \"value_font_family_fontbutton\" object in widget_frame.ui");

						description = button->get_font_desc();

						description.set_style((Pango::Style) value);

						button->set_font_desc(description);

						widget_selected_->widget()->theme().setValueFontStyle((VideoWidget::Theme::FontStyle) value);

						// Broadcast widget change
						widget_selected_->dispatchEvent(false);
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

						log_notice("Widget %s: value weight changed to '%s'", 
								widget_selected_->widget()->name().c_str(), iter->get_value(model_.m_name).c_str());

						// Update value font family button
						button = ref_builder_->get_widget<Gtk::FontButton>("value_font_family_fontbutton");

						if (!button)
							throw std::runtime_error("No \"value_font_family_fontbutton\" object in widget_frame.ui");

						description = button->get_font_desc();

						description.set_weight((Pango::Weight) value);

						button->set_font_desc(description);

						widget_selected_->widget()->theme().setValueFontWeight((VideoWidget::Theme::FontWeight) value);

						// Broadcast widget change
						widget_selected_->dispatchEvent(false);
					}
			));

	// Value horizontal align
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("value_horizontal_align_combobox");
	if (!combobox)
		throw std::runtime_error("No \"value_horizontal_align_combobox\" object in widget_frame.ui");

	combobox->pack_start(*renderer, true);
	combobox->add_attribute(renderer->property_text(), model_.m_name);
	combobox->add_attribute(renderer->property_sensitive(), model_.m_enable);

//	combobox->pack_start(model_.m_name);
	combobox->signal_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_combobox_changed), combobox, 
					[this](const Gtk::TreeModel::const_iterator &iter) {
						int value = iter->get_value(model_.m_id);

						log_notice("Widget %s: value horizontal align changed to '%s'", 
								widget_selected_->widget()->name().c_str(), iter->get_value(model_.m_name).c_str());

						widget_selected_->widget()->theme().setValueHorizontalAlign((VideoWidget::Theme::Align) value);

						// Broadcast widget change
						widget_selected_->dispatchEvent(false);
					}
			));

	// Value vertical align
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("value_vertical_align_combobox");
	if (!combobox)
		throw std::runtime_error("No \"value_vertical_align_combobox\" object in widget_frame.ui");

	combobox->pack_start(*renderer, true);
	combobox->add_attribute(renderer->property_text(), model_.m_name);
	combobox->add_attribute(renderer->property_sensitive(), model_.m_enable);

//	combobox->pack_start(model_.m_name);
	combobox->signal_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_combobox_changed), combobox, 
					[this](const Gtk::TreeModel::const_iterator &iter) {
						int value = iter->get_value(model_.m_id);

						log_notice("Widget %s: value vertical align changed to '%s'", 
								widget_selected_->widget()->name().c_str(), iter->get_value(model_.m_name).c_str());

						widget_selected_->widget()->theme().setValueVerticalAlign((VideoWidget::Theme::Align) value);

						// Broadcast widget change
						widget_selected_->dispatchEvent(false);
					}
			));

	// Value color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("value_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"value_color_button\" object in widget_frame.ui");
	colorbutton->signal_color_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_color_changed), colorbutton, 
					[this](const std::string &color) {
						log_notice("Widget %s: value color changed to '%s'", 
								widget_selected_->widget()->name().c_str(), color.c_str());

						widget_selected_->widget()->theme().setValueColor(color);

						// Broadcast widget change
						widget_selected_->dispatchEvent(false);
					}
			));

	// Value border width
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("value_border_width_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"value_border_width_spinbutton\" object in widget_frame.ui");
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_spin_changed), spinbutton, 
					[this](const int &value) {
						log_notice("Widget %s: value border width changed to '%d'",
							   widget_selected_->widget()->name().c_str(), value);

						widget_selected_->widget()->theme().setValueBorderWidth(value);

						// Broadcast widget change
						widget_selected_->dispatchEvent(false);
					}
			));

	// Value border color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("value_border_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"value_border_color_button\" object in widget_frame.ui");
	colorbutton->signal_color_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_color_changed), colorbutton, 
					[this](const std::string &color) {
						log_notice("Widget %s: value border color changed to '%s'", 
								widget_selected_->widget()->name().c_str(), color.c_str());

						widget_selected_->widget()->theme().setValueBorderColor(color);

						// Broadcast widget change
						widget_selected_->dispatchEvent(false);
					}
			));

	// Value min.
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("value_min_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"value_min_spinbutton\" object in widget_frame.ui");
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_spin_changed), spinbutton, 
					[this](const int &value) {
						log_notice("Widget %s: value min. changed to '%d'",
							   widget_selected_->widget()->name().c_str(), value);

						widget_selected_->widget()->theme().setValueMin(value);

						// Update limits
						update_boundaries();

						// Broadcast widget change
						widget_selected_->dispatchEvent(false);
					}
			));

	// Value max.
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("value_max_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"value_max_spinbutton\" object in widget_frame.ui");
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_spin_changed), spinbutton, 
					[this](const int &value) {
						log_notice("Widget %s: value max. changed to '%d'",
							   widget_selected_->widget()->name().c_str(), value);

						widget_selected_->widget()->theme().setValueMax(value);

						// Update limits
						update_boundaries();

						// Broadcast widget change
						widget_selected_->dispatchEvent(false);
					}
			));

	// Value unit
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("value_unit_combobox");
	if (!combobox)
		throw std::runtime_error("No \"value_unit_combobox\" object in widget_frame.ui");

	combobox->pack_start(*renderer, true);
	combobox->add_attribute(renderer->property_text(), model_.m_name);
	combobox->add_attribute(renderer->property_sensitive(), model_.m_enable);

	combobox->signal_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_combobox_changed), combobox, 
					[this](const Gtk::TreeModel::const_iterator &iter) {
						int value = iter->get_value(model_.m_id);

						log_notice("Widget %s: value unit changed to '%s'", 
								widget_selected_->widget()->name().c_str(), iter->get_value(model_.m_name).c_str());

						widget_selected_->widget()->setValueUnit((TelemetryData::Unit) value);

						// Broadcast widget change
						widget_selected_->dispatchEvent(false);
					}
			));

	// Value format
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("value_format_combobox");
	if (!combobox)
		throw std::runtime_error("No \"value_format_combobox\" object in widget_frame.ui");

	combobox->pack_start(*renderer, true);
	combobox->add_attribute(renderer->property_text(), model_.m_name);
	combobox->add_attribute(renderer->property_sensitive(), model_.m_enable);

	combobox->signal_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_combobox_changed), combobox, 
					[this](const Gtk::TreeModel::const_iterator &iter) {
						std::list<VideoWidget::ListItem> formats;

						int value = iter->get_value(model_.m_id);

						log_notice("Widget %s: value format changed to '%s'", 
								widget_selected_->widget()->name().c_str(), iter->get_value(model_.m_name).c_str());

						formats = widget_selected_->widget()->formats();

						for (auto item : formats) {
							if (item.id != value)
								continue;

							widget_selected_->widget()->setValueFormat(item.fmt);

							break;
						}

						// Broadcast widget change
						widget_selected_->dispatchEvent(false);
					}
			));

	// Unit enable
	sw = ref_builder_->get_widget<Gtk::Switch>("unit_enable_switch");
	if (!sw)
		throw std::runtime_error("No \"unit_enable_switch\" object in widget_frame.ui");
	sw->signal_state_set().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_switch_changed), sw, 
					[this](const bool &state) {
						log_notice("Widget %s: unit status changed to '%s'",
							   widget_selected_->widget()->name().c_str(), state ? "enabled" : "disabled");

						if (state)
							widget_selected_->widget()->theme().addFlag(VideoWidget::Theme::FlagUnit);
						else
							widget_selected_->widget()->theme().removeFlag(VideoWidget::Theme::FlagUnit);

						// Broadcast widget change
						widget_selected_->dispatchEvent(false);
					} 
			), false);

	// Unit font size
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("unit_font_size_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"unit_font_size_spinbutton\" object in widget_frame.ui");
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_spin_changed), spinbutton, 
					[this](const int &value) {
						log_notice("Widget %s: unit font size changed to '%d'",
							   widget_selected_->widget()->name().c_str(), value);

						widget_selected_->widget()->theme().setUnitFontSize(value);

						// Broadcast widget change
						widget_selected_->dispatchEvent(false);
					}
			));
}


void GPX2VideoWidgetFrame::update_content(void) {
	log_call();

	int margin;
	int width, height;

	Gdk::RGBA rgba;

	const float *color;

	Gtk::Box *box;
	Gtk::Entry *entry;
	Gtk::Switch *sw;
	Gtk::ComboBox *combobox;
	Gtk::Expander *expander;
	Gtk::FontButton *fontbutton;
	Gtk::SpinButton *spinbutton;
	Gtk::ColorButton *colorbutton;

	Pango::FontDescription description;

	Gtk::TreeModel::iterator iter;

	std::list<VideoWidget::ListItem> shapes;
	std::list<VideoWidget::ListItem> units;
	std::list<VideoWidget::ListItem> formats;

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

	// Widget shapes, units & formats supported list
	shapes = widget_selected_->widget()->shapes();
	units = widget_selected_->widget()->units();
	formats = widget_selected_->widget()->formats();

	// Widget shape expander container
	expander = ref_builder_->get_widget<Gtk::Expander>("shape_expander");

	expander->set_visible((shapes.size() > 1));

	// Widget shape
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("shape_combobox");
	if (!combobox)
		throw std::runtime_error("No \"shape_combobox\" object in widget_frame.ui");

	combobox->set_model(shape_model_);

	// Fill shape model
	shape_model_->clear();

	if (shapes.size() > 1) {
		for (auto item : shapes) {
			auto iter = shape_model_->append();
			auto row = *iter;

			row[model_.m_id] = item.id;
			row[model_.m_name] = item.name;
			row[model_.m_enable] = true;
		}

//		if (find_in_listtore(shape_model_, widget_selected_->widget()->shape(), iter))
		if (find_in_listtore(shape_model_, widget_selected_->shape()->type(), iter))
			combobox->set_active(iter);
	}

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

	// Widget label horizontal align
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("label_horizontal_align_combobox");
	if (!combobox)
		throw std::runtime_error("No \"label_horizontal_align_combobox\" object in widget_frame.ui");

	combobox->set_model(text_horizontal_align_model_);

	if (find_in_listtore(text_horizontal_align_model_, widget_selected_->widget()->theme().labelHorizontalAlign(), iter))
		combobox->set_active(iter);

	// Widget label vertical align
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("label_vertical_align_combobox");
	if (!combobox)
		throw std::runtime_error("No \"label_vertical_align_combobox\" object in widget_frame.ui");

	combobox->set_model(text_vertical_align_model_);

	if (find_in_listtore(text_vertical_align_model_, widget_selected_->widget()->theme().labelVerticalAlign(), iter))
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

	// Widget value horizontal align
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("value_horizontal_align_combobox");
	if (!combobox)
		throw std::runtime_error("No \"value_horizontal_align_combobox\" object in widget_frame.ui");

	combobox->set_model(text_horizontal_align_model_);

	if (find_in_listtore(text_horizontal_align_model_, widget_selected_->widget()->theme().valueHorizontalAlign(), iter))
		combobox->set_active(iter);

	// Widget value vertical align
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("value_vertical_align_combobox");
	if (!combobox)
		throw std::runtime_error("No \"value_vertical_align_combobox\" object in widget_frame.ui");

	combobox->set_model(text_vertical_align_model_);

	if (find_in_listtore(text_vertical_align_model_, widget_selected_->widget()->theme().valueVerticalAlign(), iter))
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

	// Widget unit expander container
	expander = ref_builder_->get_widget<Gtk::Expander>("unit_expander");
	if (!expander)
		throw std::runtime_error("No \"unit_expander\" object in widget_frame.ui");

	expander->set_visible(units.size() > 0);

	// Widget value unit box container
	box = ref_builder_->get_widget<Gtk::Box>("value_unit_box");
	if (!box)
		throw std::runtime_error("No \"value_unit_box\" object in widget_frame.ui");

	box->set_visible(units.size() > 1);

	// Widget value unit 
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("value_unit_combobox");
	if (!combobox)
		throw std::runtime_error("No \"value_unit_combobox\" object in widget_frame.ui");

	combobox->set_model(value_unit_model_);

	// Fill unit model
	value_unit_model_->clear();

	if (units.size() > 1) {
		for (auto item : units) {
			auto iter = value_unit_model_->append();
			auto row = *iter;

			row[model_.m_id] = item.id;
			row[model_.m_name] = item.name;
			row[model_.m_enable] = true;
		}

		if (find_in_listtore(value_unit_model_, widget_selected_->widget()->valueUnit(), iter))
			combobox->set_active(iter);
	}

	// Widget value format box container
	box = ref_builder_->get_widget<Gtk::Box>("value_format_box");

	box->set_visible(formats.size() > 1);

	// Widget value format
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("value_format_combobox");
	if (!combobox)
		throw std::runtime_error("No \"value_format_combobox\" object in widget_frame.ui");
	
	combobox->set_model(value_format_model_);

	// Fill format model
	value_format_model_->clear();

	if (formats.size() > 1) {
		int value = 0;

		for (auto item : formats) {
			auto iter = value_format_model_->append();
			auto row = *iter;

			row[model_.m_id] = item.id;
			row[model_.m_name] = item.name;
			row[model_.m_enable] = true;

			if (item.fmt == widget_selected_->widget()->valueFormat())
				value = item.id;
		}

		if (find_in_listtore(value_format_model_, value, iter))
			combobox->set_active(iter);
	}

	// Widget unit enable switch
	sw = ref_builder_->get_widget<Gtk::Switch>("unit_enable_switch");
	if (!sw)
		throw std::runtime_error("No \"unit_enable_switch\" object in widget_frame.ui");

	sw->set_active(widget_selected_->widget()->theme().hasFlag(VideoWidget::Theme::FlagUnit));

	// Widget unit size
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("unit_font_size_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"unit_font_size_spinbutton\" object in widget_frame.ui");

	spinbutton->set_value(widget_selected_->widget()->theme().unitFontSize());

	// Widget settings
	update_shape_content();
	update_widget_content();

	log_info("Widget settings loaded");

	// Unmask value changed
	loading_ = false;
}


void GPX2VideoWidgetFrame::update_boundaries(void) {
	int value;
	int margin;
	int width, height;

//	VideoWidget::Widget type;

	Gtk::Expander *expander;
	Gtk::SpinButton *spinbutton;

	log_call();

	// No widget
	if (widget_selected_ == NULL)
		return;

//	// Widget type
//	type = widget_selected_->widget()->type();

	// Paddings
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("padding_left_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"padding_left_spinbutton\" object in widget_frame.ui");

	width = widget_selected_->theme().width();

	spinbutton->set_range(0, width);

	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("padding_right_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"padding_right_spinbutton\" object in widget_frame.ui");

	width = widget_selected_->theme().width();

	spinbutton->set_range(0, width);

	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("padding_top_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"padding_top_spinbutton\" object in widget_frame.ui");

	height = widget_selected_->theme().height();

	spinbutton->set_range(0, height);

	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("padding_bottom_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"padding_bottom_spinbutton\" object in widget_frame.ui");

	height = widget_selected_->theme().height();

	spinbutton->set_range(0, height);

	// Update UI range for widget width
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("width_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"width_spinbutton\" object in widget_frame.ui");

	width = renderer_->width();
	if (widget_selected_->widget()->position() == VideoWidget::PositionNone) {
		margin = widget_selected_->widget()->margin(VideoWidget::MarginLeft) 
			+ widget_selected_->widget()->margin(VideoWidget::MarginRight);
	}
	else
		margin = 0;

	spinbutton->set_range(0, width - margin);

	// Update UI range for widget height
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("height_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"height_spinbutton\" object in widget_frame.ui");

	height = renderer_->height();
	if (widget_selected_->widget()->position() == VideoWidget::PositionNone) {
		margin = widget_selected_->widget()->margin(VideoWidget::MarginTop) 
			+ widget_selected_->widget()->margin(VideoWidget::MarginBottom);
	}
	else
		margin = 0;

	spinbutton->set_range(0, height - margin);

	// Label expander
	expander = ref_builder_->get_widget<Gtk::Expander>("label_expander");
	if (!expander)
		throw std::runtime_error("No \"label_expander\" object in widget_frame.ui");
//	expander->set_visible((type != VideoWidget::WidgetTrack) && (type != VideoWidget::WidgetMap));
	expander->set_visible(widget_selected_->shape()->hasFeature(ShapeBase::FeatureLabel));

	// Label font size
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("label_font_size_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"label_font_size_spinbutton\" object in widget_frame.ui");

	spinbutton->set_range(0, height - margin);

	// Value expander
	expander = ref_builder_->get_widget<Gtk::Expander>("value_expander");
	if (!expander)
		throw std::runtime_error("No \"value_expander\" object in widget_frame.ui");
//	expander->set_visible((type != VideoWidget::WidgetTrack) && (type != VideoWidget::WidgetMap));
	expander->set_visible(widget_selected_->shape()->hasFeature(ShapeBase::FeatureValue));

	// Value font size
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("value_font_size_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"value_font_size_spinbutton\" object in widget_frame.ui");

	spinbutton->set_range(0, height - margin);

	// Value min.
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("value_min_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"value_min_spinbutton\" object in widget_frame.ui");

	if (widget_selected_->widget()->theme().valueMax() > widget_selected_->widget()->theme().valueMin())
		value = widget_selected_->widget()->theme().valueMax();
	else
		value = std::numeric_limits<int>::max();

	spinbutton->set_range(std::numeric_limits<int>::min(), value);

	// Value max.
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("value_max_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"value_max_spinbutton\" object in widget_frame.ui");

	if (widget_selected_->widget()->theme().valueMax() > widget_selected_->widget()->theme().valueMin())
		value = widget_selected_->widget()->theme().valueMin();
	else
		value = std::numeric_limits<int>::min();

	spinbutton->set_range(value, std::numeric_limits<int>::max());
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

	// Lock
	std::lock_guard<std::mutex> lock(widget_selected_->mutex());

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

	// Update limit
	update_boundaries();

	// Broadcast widget change
	widget_selected_->dispatchEvent(true);
}


void GPX2VideoWidgetFrame::on_widget_font_changed(Gtk::FontButton *button, std::function<void(const Pango::FontDescription&)> set) {
	log_call();

	Pango::FontDescription description;

	if (loading_)
		return;

	// Lock
	std::lock_guard<std::mutex> lock(widget_selected_->mutex());

	// Read font description
	description = button->get_font_desc();

	// Set description
	set(description);
}


void GPX2VideoWidgetFrame::on_widget_spin_changed(Gtk::SpinButton *button, std::function<void(const int&)> set) {
	log_call();

	int value;

	if (loading_)
		return;

	// Lock
	std::lock_guard<std::mutex> lock(widget_selected_->mutex());

	// Read value
	value = button->get_value_as_int();

	// Set value
	set(value);
}


void GPX2VideoWidgetFrame::on_widget_color_changed(Gtk::ColorButton *button, std::function<void(const std::string&)> set) {
	log_call();

	Gdk::RGBA rgba;

	Glib::ustring color;

	if (loading_)
		return;

	// Lock
	std::lock_guard<std::mutex> lock(widget_selected_->mutex());

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


void GPX2VideoWidgetFrame::on_widget_entry_changed(Gtk::Entry *entry, std::function<void(const Glib::ustring&)> set) {
	log_call();

	if (loading_)
		return;

	// Lock
	std::lock_guard<std::mutex> lock(widget_selected_->mutex());

	// Set entry
	set(entry->get_text());
}


void GPX2VideoWidgetFrame::on_widget_combobox_changed(Gtk::ComboBox *combobox, std::function<void(const Gtk::TreeModel::const_iterator&)> set) {
	log_call();

	if (loading_)
		return;

	// Lock
	std::lock_guard<std::mutex> lock(widget_selected_->mutex());

	// Set combobox
	set(combobox->get_active());
}

bool GPX2VideoWidgetFrame::on_widget_switch_changed(bool state, Gtk::Switch *sw, std::function<void(const bool&)> set) {
	log_call();

	if (loading_)
		return false;

	// Lock
	std::lock_guard<std::mutex> lock(widget_selected_->mutex());

	// Text enable
	sw->set_state(state);

	// Set state
	set(state);

	return true;
}


void GPX2VideoWidgetFrame::on_widget_changed(bool schedule) {
	log_call();

	log_info("Widget extension properties changed");

	// Refresh widget required
	renderer_->refresh(widget_selected_, schedule);

	// Refresh video preview
	dispatcher_.emit();
}

