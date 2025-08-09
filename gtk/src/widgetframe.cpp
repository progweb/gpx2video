#include <gtkmm/combobox.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/colorbutton.h>

#include "log.h"
#include "widgetframe.h"



GPX2VideoWidgetFrame::GPX2VideoWidgetFrame()
	: Glib::ObjectBase("GPX2VideoWidgetFrame")
	, ref_builder_(NULL) 
	, dispatcher_()
	, renderer_(NULL)
	, widget_selected_(NULL) {
	log_call();

	loading_ = false;
}


GPX2VideoWidgetFrame::GPX2VideoWidgetFrame(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &ref_builder) 
	: Glib::ObjectBase("GPX2VideoWidgetFrame")
	, Gtk::Frame(cobject)
	, ref_builder_(ref_builder) 
	, dispatcher_()
	, renderer_(NULL)
	, widget_selected_(NULL) {
	log_call();

	Gtk::ComboBox *combobox;
	Gtk::SpinButton *spinbutton;
	Gtk::ColorButton *colorbutton;

	loading_ = false;

	// Populate models
	//-----------------

	align_model_ = Gtk::ListStore::create(model_);

	{
		auto iter = align_model_->append();
		auto row = *iter;
		row[model_.m_id] = 0;
		row[model_.m_name] = "None";

		row = *(align_model_->append());
		row[model_.m_id] = 1;
		row[model_.m_name] = "Horizontal";

		row = *(align_model_->append());
		row[model_.m_id] = 2;
		row[model_.m_name] = "Vertical";
	}

	position_model_ = Gtk::ListStore::create(model_);

	{
		auto iter = position_model_->append();
		auto row = *iter;
		row[model_.m_id] = 0;
		row[model_.m_name] = "None";

		row = *(position_model_->append());
		row[model_.m_id] = 1;
		row[model_.m_name] = "Left";

		row = *(position_model_->append());
		row[model_.m_id] = 2;
		row[model_.m_name] = "Right";

		row = *(position_model_->append());
		row[model_.m_id] = 3;
		row[model_.m_name] = "Bottom";

		row = *(position_model_->append());
		row[model_.m_id] = 4;
		row[model_.m_name] = "Top";

		row = *(position_model_->append());
		row[model_.m_id] = 5;
		row[model_.m_name] = "Bottom - Left";

		row = *(position_model_->append());
		row[model_.m_id] = 6;
		row[model_.m_name] = "Bottom - Right";

		row = *(position_model_->append());
		row[model_.m_id] = 7;
		row[model_.m_name] = "Top - Left";

		row = *(position_model_->append());
		row[model_.m_id] = 8;
		row[model_.m_name] = "Top - Right";
	}

	// Connect widgets button
	//------------------------

	// Position
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("position_combobox");
	if (!combobox)
		throw std::runtime_error("No \"position_combobox\" object in widget_frame.ui");
//	combobox->pack_start(model_.m_id);
	combobox->pack_start(model_.m_name);
	combobox->signal_changed().connect(sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_position_value_changed));

	// Alignment
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("align_combobox");
	if (!combobox)
		throw std::runtime_error("No \"alignn_combobox\" object in widget_frame.ui");
//	combobox->pack_start(model_.m_id);
	combobox->pack_start(model_.m_name);
	combobox->signal_changed().connect(sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_align_value_changed));

	// Width x Height
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("width_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"width_spinbutton\" object in widget_frame.ui");
	spinbutton->signal_value_changed().connect(sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_width_value_changed));

	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("height_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"height_spinbutton\" object in widget_frame.ui");
	spinbutton->signal_value_changed().connect(sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_height_value_changed));

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
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_padding_value_changed), VideoWidget::PaddingLeft));

	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("padding_right_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"padding_right_spinbutton\" object in widget_frame.ui");
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_padding_value_changed), VideoWidget::PaddingRight));

	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("padding_top_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"padding_top_spinbutton\" object in widget_frame.ui");
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_padding_value_changed), VideoWidget::PaddingTop));

	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("padding_bottom_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"padding_bottom_spinbutton\" object in widget_frame.ui");
	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_padding_value_changed), VideoWidget::PaddingBottom));

	// Text color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("text_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"text_color_button\" object in video_frame.ui");
	colorbutton->signal_color_set().connect(sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_text_color_set));

	// Border color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("border_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"border_color_button\" object in video_frame.ui");
	colorbutton->signal_color_set().connect(sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_border_color_set));

	// Border width
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("border_width_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"border_width_spinbutton\" object in widget_frame.ui");
	spinbutton->signal_value_changed().connect(sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_border_value_changed));

	// Background color
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("background_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"background_color_button\" object in video_frame.ui");
	colorbutton->signal_color_set().connect(sigc::mem_fun(*this, &GPX2VideoWidgetFrame::on_widget_background_color_set));

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

	Gtk::Frame::set_visible(visible && (widget_selected_ != NULL));
}


void GPX2VideoWidgetFrame::set_widget_selected(GPX2VideoWidget *widget) {
	log_call();

	// Save selected widget
	widget_selected_ = widget;

	// Update ui content
	update_content();
}


void GPX2VideoWidgetFrame::update_content(void) {
	log_call();

	int width, height;

	Gdk::RGBA rgba;

	const float *color;

	Gtk::ComboBox *combobox;
	Gtk::SpinButton *spinbutton;
	Gtk::ColorButton *colorbutton;

	// Frame is visible only as widget is selected
	set_visible((widget_selected_ != NULL));

	// No widget
	if (widget_selected_ == NULL)
		return;

	log_info("Load widget settings...");

	// Mask value changed
	loading_ = true;

	// Layout size
	width = renderer_->width();
	height = renderer_->height();

	// Widget position
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("position_combobox");
	if (!combobox)
		throw std::runtime_error("No \"position_combobox\" object in widget_frame.ui");

	combobox->set_model(position_model_);

	combobox->set_active(widget_selected_->widget()->position());

	// Widget align
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("align_combobox");
	if (!combobox)
		throw std::runtime_error("No \"align_combobox\" object in widget_frame.ui");

	combobox->set_model(align_model_);

	combobox->set_active(widget_selected_->widget()->align());

	// Widget width
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("width_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"width_spinbutton\" object in widget_frame.ui");

	spinbutton->set_range(0, width - widget_selected_->widget()->width());
	spinbutton->set_value(widget_selected_->widget()->width());

	// Widget height
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("height_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"height_spinbutton\" object in widget_frame.ui");

	spinbutton->set_range(0, height - widget_selected_->widget()->height());
	spinbutton->set_value(widget_selected_->widget()->height());

	// Widget margin left
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("margin_left_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"margin_left_spinbutton\" object in widget_frame.ui");

	spinbutton->set_range(0, width - widget_selected_->widget()->width());
	spinbutton->set_value(widget_selected_->widget()->margin(VideoWidget::MarginLeft));

	// Widget margin right
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("margin_right_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"margin_right_spinbutton\" object in widget_frame.ui");

	spinbutton->set_range(0, width - widget_selected_->widget()->width());
	spinbutton->set_value(widget_selected_->widget()->margin(VideoWidget::MarginRight));

	// Widget margin top
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("margin_top_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"margin_top_spinbutton\" object in widget_frame.ui");

	spinbutton->set_range(0, height - widget_selected_->widget()->height());
	spinbutton->set_value(widget_selected_->widget()->margin(VideoWidget::MarginTop));

	// Widget margin bottom
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("margin_bottom_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"margin_bottom_spinbutton\" object in widget_frame.ui");

	spinbutton->set_range(0, height - widget_selected_->widget()->height());
	spinbutton->set_value(widget_selected_->widget()->margin(VideoWidget::MarginBottom));

	// Widget padding left
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("padding_left_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"padding_left_spinbutton\" object in widget_frame.ui");

	spinbutton->set_value(widget_selected_->widget()->padding(VideoWidget::PaddingLeft));

	// Widget padding right
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("padding_right_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"padding_right_spinbutton\" object in widget_frame.ui");

	spinbutton->set_value(widget_selected_->widget()->padding(VideoWidget::PaddingRight));

	// Widget padding top
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("padding_top_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"padding_top_spinbutton\" object in widget_frame.ui");

	spinbutton->set_value(widget_selected_->widget()->padding(VideoWidget::PaddingTop));

	// Widget padding bottom
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("padding_bottom_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"padding_bottom_spinbutton\" object in widget_frame.ui");

	spinbutton->set_value(widget_selected_->widget()->padding(VideoWidget::PaddingBottom));

	// Widget text color button
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("text_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"text_color_button\" object in widget_frame.ui");

	color = widget_selected_->widget()->textColor();

	rgba.set_rgba(color[0], color[1], color[2], color[3]);

	colorbutton->set_rgba(rgba);

	// Widget border color button
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("border_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"border_color_button\" object in widget_frame.ui");

	color = widget_selected_->widget()->borderColor();

	rgba.set_rgba(color[0], color[1], color[2], color[3]);

	colorbutton->set_rgba(rgba);

	// Widget border width
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("border_width_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"border_width_spinbutton\" object in widget_frame.ui");

	spinbutton->set_value(widget_selected_->widget()->border());

	// Widget background color button
	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("background_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"background_color_button\" object in widget_frame.ui");

	color = widget_selected_->widget()->backgroundColor();

	rgba.set_rgba(color[0], color[1], color[2], color[3]);

	colorbutton->set_rgba(rgba);

	log_info("Widget settings loaded");

	// Mask value changed
	loading_ = false;
}


void GPX2VideoWidgetFrame::on_widget_position_value_changed(void) {
	log_call();

	int value;

	if (loading_)
		return;

	// Widget position
	auto combobox = ref_builder_->get_widget<Gtk::ComboBox>("position_combobox");
	if (!combobox)
		throw std::runtime_error("No \"position_combobox\" object in widget_frame.ui");

	value = combobox->get_active_row_number();

	log_info("Widget %s: position changed to '%d'", 
			widget_selected_->widget()->name().c_str(), value);

	widget_selected_->widget()->setPosition((VideoWidget::Position) value);
	renderer_->refresh(widget_selected_);

	// Compute new position
	renderer_->compute();

	// Refresh video preview
	dispatcher_.emit();
}


void GPX2VideoWidgetFrame::on_widget_align_value_changed(void) {
	log_call();

	int value;

	if (loading_)
		return;

	// Widget align
	auto combobox = ref_builder_->get_widget<Gtk::ComboBox>("align_combobox");
	if (!combobox)
		throw std::runtime_error("No \"align_combobox\" object in widget_frame.ui");

	value = combobox->get_active_row_number();

	log_info("Widget %s: align changed to '%d'", 
			widget_selected_->widget()->name().c_str(), value);

	widget_selected_->widget()->setAlign((VideoWidget::Align) value);
	renderer_->refresh(widget_selected_);

	// Compute new align
	renderer_->compute();

	// Refresh video preview
	dispatcher_.emit();
}


void GPX2VideoWidgetFrame::on_widget_width_value_changed(void) {
	log_call();

	int value;

	if (loading_)
		return;

	int height = widget_selected_->widget()->height();

	// Widget width
	auto spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("width_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"width_spinbutton\" object in widget_frame.ui");

	value = spinbutton->get_value_as_int();

	log_info("Widget %s: size changed to '%dx%d'", 
			widget_selected_->widget()->name().c_str(), value, height);

	widget_selected_->setSize(value, height);
	renderer_->refresh(widget_selected_);

	// Compute new position
	renderer_->compute();

	// Refresh video preview
	dispatcher_.emit();
}


void GPX2VideoWidgetFrame::on_widget_height_value_changed(void) {
	log_call();

	int value;

	if (loading_)
		return;

	int width = widget_selected_->widget()->width();

	// Widget height
	auto spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("height_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"height_spinbutton\" object in widget_frame.ui");

	value = spinbutton->get_value_as_int();

	log_info("Widget %s: size changed to '%dx%d'", 
			widget_selected_->widget()->name().c_str(), width, value);

	widget_selected_->setSize(width, value);
	renderer_->refresh(widget_selected_);

	// Compute new position
	renderer_->compute();

	// Refresh video preview
	dispatcher_.emit();
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

	// Compute new position
	renderer_->compute();

	// Refresh video preview
	dispatcher_.emit();
}


void GPX2VideoWidgetFrame::on_widget_padding_value_changed(const VideoWidget::Padding &padding) {
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

	widget_selected_->widget()->setPadding(padding, value);
	renderer_->refresh(widget_selected_);

	// Refresh video preview
	dispatcher_.emit();
}


void GPX2VideoWidgetFrame::on_widget_text_color_set(void) {
	log_call();

	Gdk::RGBA rgba;

	Glib::ustring color;

	if (loading_)
		return;

	// Widget color button
	auto button = ref_builder_->get_widget<Gtk::ColorButton>("text_color_button");
	if (!button)
		throw std::runtime_error("No \"text_color_button\" object in widget_frame.ui");

	rgba = button->get_rgba();

	// Convert to hexa string color
	color = Glib::ustring::sprintf("#%02X%02X%02X%02X",
			(unsigned char) std::round(rgba.get_red() * 255),
			(unsigned char) std::round(rgba.get_green() * 255),
			(unsigned char) std::round(rgba.get_blue() * 255),
			(unsigned char) std::round(rgba.get_alpha() * 255)
	);

	log_info("Widget '%s' text color changed to '%s'", 
			widget_selected_->widget()->name().c_str(), color.c_str());

	widget_selected_->widget()->setTextColor(color);
	renderer_->refresh(widget_selected_);

	// Refresh video preview
	dispatcher_.emit();
}


void GPX2VideoWidgetFrame::on_widget_border_color_set(void) {
	log_call();

	Gdk::RGBA rgba;

	Glib::ustring color;

	if (loading_)
		return;

	// Widget color button
	auto button = ref_builder_->get_widget<Gtk::ColorButton>("border_color_button");
	if (!button)
		throw std::runtime_error("No \"border_color_button\" object in widget_frame.ui");

	rgba = button->get_rgba();

	// Convert to hexa string color
	color = Glib::ustring::sprintf("#%02X%02X%02X%02X",
			(unsigned char) std::round(rgba.get_red() * 255),
			(unsigned char) std::round(rgba.get_green() * 255),
			(unsigned char) std::round(rgba.get_blue() * 255),
			(unsigned char) std::round(rgba.get_alpha() * 255)
	);

	log_info("Widget %s: border color changed to '%s'", 
			widget_selected_->widget()->name().c_str(), color.c_str());

	widget_selected_->widget()->setBorderColor(color);
	renderer_->refresh(widget_selected_);

	// Refresh video preview
	dispatcher_.emit();
}


void GPX2VideoWidgetFrame::on_widget_border_value_changed(void) {
	log_call();

	int border;

	if (loading_)
		return;

	// Widget border width
	auto spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("border_width_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"border_width_spinbutton\" object in widget_frame.ui");

	border = spinbutton->get_value_as_int();

	log_info("Widget %s: border changed to '%d'",
		   widget_selected_->widget()->name().c_str(), border);

	widget_selected_->widget()->setBorder(border);
	renderer_->refresh(widget_selected_);

	// Refresh video preview
	dispatcher_.emit();
}


void GPX2VideoWidgetFrame::on_widget_background_color_set(void) {
	log_call();

	Gdk::RGBA rgba;

	Glib::ustring color;

	if (loading_)
		return;

	// Widget color button
	auto button = ref_builder_->get_widget<Gtk::ColorButton>("background_color_button");
	if (!button)
		throw std::runtime_error("No \"background_color_button\" object in widget_frame.ui");

	rgba = button->get_rgba();

	// Convert to hexa string color
	color = Glib::ustring::sprintf("#%02X%02X%02X%02X",
			(unsigned char) std::round(rgba.get_red() * 255),
			(unsigned char) std::round(rgba.get_green() * 255),
			(unsigned char) std::round(rgba.get_blue() * 255),
			(unsigned char) std::round(rgba.get_alpha() * 255)
	);

	log_info("Widget %s: background color changed to '%s'", 
			widget_selected_->widget()->name().c_str(), color.c_str());

	widget_selected_->widget()->setBackgroundColor(color);
	renderer_->refresh(widget_selected_);

	// Refresh video preview
	dispatcher_.emit();
}


