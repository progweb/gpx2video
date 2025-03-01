#include <iostream>
#include <exception>

#include <glibmm/binding.h>
#include <glibmm/datetime.h>
#include <glibmm/miscutils.h>

#include <gtkmm/window.h>
#include <gtkmm/application.h>
#include <gtkmm/applicationwindow.h>
#include <gtkmm/icontheme.h>
#include <gtkmm/filefilter.h>
#include <giomm/liststore.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/eventcontrollerkey.h>
#include <gtkmm/gestureclick.h>
#include <gtkmm/stackpage.h>
#include <gtkmm/frame.h>
#include <gtkmm/box.h>
#include <gtkmm/listbox.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>

extern "C" {
#include <event2/event.h>
#include <libavcodec/avcodec.h>
}

#include "log.h"
#include "compat.h"
#include "datetime.h"
#include "window.h"


GPX2VideoApplicationWindow::GPX2VideoApplicationWindow(BaseObjectType *cobject,
	const Glib::RefPtr<Gtk::Builder> &ref_builder) 
	: Gtk::ApplicationWindow(cobject)
	, GPXApplication(event_base_new())
	, ref_builder_(ref_builder)
	, media_file_("") 
	, layout_file_("") 
	, media_(NULL)
	, renderer_(NULL) {
	log_call();

	Gtk::Button *button;

	// Set the window icon.
	Gtk::IconTheme::get_for_display(get_display())->add_resource_path("/com/progweb/gpx2video/icons");
//	set_default_icon_name("com.progweb.gpx2video");
	set_icon_name("com.progweb.gpx2video");
//	set_theme_name("gpx2video");

	// Get widgets from the Gtk::Builder file
	gears_ = ref_builder_->get_widget<Gtk::MenuButton>("gears");
	if (!gears_)
		throw std::runtime_error("No \"gears\" object in window.ui");

	play_button_ = ref_builder_->get_widget<Gtk::Button>("play_button");
	if (!play_button_)
		throw std::runtime_error("No \"play_button\" object in window.ui");

	progress_scale_ = ref_builder_->get_widget<Gtk::Scale>("progress_scale");
	if (!progress_scale_)
		throw std::runtime_error("No \"progress_scale\" object in window.ui");

	video_area_ = Gtk::Builder::get_widget_derived<GPX2VideoArea>(ref_builder_, "video_area", *this);
	if (!video_area_)
		throw std::runtime_error("No \"video_area\" object in window.ui");

	// Connect the menu to the MenuButton gears_.
	// (The connection between action and menu item is specified in gears_menu.ui)
	Glib::RefPtr<Gtk::Builder> builder;

	builder = Gtk::Builder::create_from_resource("/com/progweb/gpx2video/ui/gears_menu.ui");
	auto menu = builder->get_object<Gio::MenuModel>("menu");
	if (!menu)
		throw std::runtime_error("No \"main_menu\" object in gears_menu.ui");

	gears_->set_menu_model(menu);

	// Video frame object
	builder = Gtk::Builder::create_from_resource("/com/progweb/gpx2video/ui/video_frame.ui");
	video_frame_ = Gtk::Builder::get_widget_derived<GPX2VideoVideoFrame>(builder, "video_frame");
	if (!video_frame_)
		throw std::runtime_error("No \"video_frame\" object in video_frame.ui");

	// Widget frame object
	builder = Gtk::Builder::create_from_resource("/com/progweb/gpx2video/ui/widget_frame.ui");
	widget_frame_ = Gtk::Builder::get_widget_derived<GPX2VideoWidgetFrame>(builder, "widget_frame");
	if (!widget_frame_)
		throw std::runtime_error("No \"widget_frame\" object in widget_frame.ui");

	// Telemetry frame object
	builder = Gtk::Builder::create_from_resource("/com/progweb/gpx2video/ui/telemetry_frame.ui");
	telemetry_frame_ = Gtk::Builder::get_widget_derived<GPX2VideoTelemetryFrame>(builder, "telemetry_frame");
	if (!telemetry_frame_)
		throw std::runtime_error("No \"telemetry_frame\" object in telemetry_frame.ui");

	// Append video, widget & telemetry frame objects to box
	auto box = ref_builder_->get_widget<Gtk::Box>("frames_box");
	if (!box)
		throw std::runtime_error("No \"frames_box\" object in window.ui");

	box->append(*video_frame_);
	box->append(*telemetry_frame_);
	box->append(*widget_frame_);

	// Add actions and keyboard accelerators for the headerbar actions
	add_action("open", sigc::mem_fun(*this, &GPX2VideoApplicationWindow::on_action_open));
	add_action("append", sigc::mem_fun(*this, &GPX2VideoApplicationWindow::on_action_append));

	add_action("use_creation_time", sigc::mem_fun(*video_frame_, &GPX2VideoVideoFrame::on_action_use_creation_time));
	add_action("use_gpmf_stream", sigc::mem_fun(*video_frame_, &GPX2VideoVideoFrame::on_action_use_gpmf_stream));
	add_action("use_gpx_data", sigc::mem_fun(*video_frame_, &GPX2VideoVideoFrame::on_action_use_gpx_data));

	// Video, telemetry & widget frame listener
	video_frame_->signal_video_changed().connect(sigc::mem_fun(*this, &GPX2VideoApplicationWindow::on_video_changed));
	widget_frame_->signal_widget_changed().connect(sigc::mem_fun(*this, &GPX2VideoApplicationWindow::on_widget_changed));

	// Connect signals
	play_button_->signal_clicked().connect(sigc::mem_fun(*this, &GPX2VideoApplicationWindow::on_play_clicked));

	auto adjustment = progress_scale_->get_adjustment();
	progress_scale_->signal_change_value().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoApplicationWindow::on_progress_change_value), adjustment), false);
	video_area_->set_adjustment(adjustment);

	// Click listener
	auto gesture = Gtk::GestureClick::create();
	progress_scale_->add_controller(gesture);

	gesture->signal_pressed().connect(sigc::mem_fun(*this, &GPX2VideoApplicationWindow::on_progress_scale_pressed), false);
	gesture->signal_released().connect(sigc::mem_fun(*this, &GPX2VideoApplicationWindow::on_progress_scale_released), false);

	// Key listener
	auto controller = Gtk::EventControllerKey::create();
	controller->set_propagation_phase(Gtk::PropagationPhase::CAPTURE);
	add_controller(controller);

	controller->signal_key_pressed().connect(sigc::mem_fun(*this, &GPX2VideoApplicationWindow::on_key_pressed), false);

//	// Register application handle
//	video_area_->set_application(this);

	// Create renderer context
	renderer_ = GPX2VideoRenderer::create(*this, renderer_settings_, telemetry_settings_);
	video_area_->set_renderer(renderer_);

	// Connect widget list
	auto list = ref_builder_->get_widget<Gtk::ListBox>("widgets_listbox");
	if (!list)
		throw std::runtime_error("No \"widgets_listbox\" object in window.ui");

	list->signal_row_selected().connect(sigc::mem_fun(*this, &GPX2VideoApplicationWindow::on_widget_selected));

	// Connect widgets button
	button = ref_builder_->get_widget<Gtk::Button>("widget_append_button");
	if (!button)
		throw std::runtime_error("No \"widget_append_button\" object in window.ui");

	button->signal_clicked().connect(sigc::mem_fun(*this, &GPX2VideoApplicationWindow::on_widget_append_clicked));

	// Listen stack changes
	info_stack_ = ref_builder_->get_widget<Gtk::Stack>("info_stack");
	if (!info_stack_)
		throw std::runtime_error("No \"info_stack\" object in window.ui");

	info_stack_->property_visible_child().signal_changed().connect(sigc::mem_fun(*this, &GPX2VideoApplicationWindow::on_stack_changed));

//	// WIDGET TEST
//	TimeWidget *widget = TimeWidget::create(*this);
//	widget->setPosition(30, 30);
//	widget->setSize(500, 100);
//	widget->setPadding(VideoWidget::PaddingAll, 5);
//	widget->setBorder(2);
//	widget->setTextColor("#ffffffff");
//	widget->setBorderColor("#0000ffff");
//	widget->setBackgroundColor("#00000099");
//	widget->setLabel("TIME");
//	widget->initialize();
//	video_area_->widget_append(widget);
}


GPX2VideoApplicationWindow::~GPX2VideoApplicationWindow() {
	log_call();

	if (video_area_)
		delete video_area_;
}


GPX2VideoApplicationWindow * GPX2VideoApplicationWindow::create(void) {
	log_call();

	GPXApplication dummy(NULL);

	// Create a dummy instance before the call to Gtk::Builder::create_from_resource
	// This creation registers GPX2VideoArea's class in the GType system.
	static_cast<void>(GPX2VideoArea(dummy));
	static_cast<void>(GPX2VideoVideoFrame());
	static_cast<void>(GPX2VideoWidgetFrame());
	static_cast<void>(GPX2VideoTelemetryFrame());

	// Load the Builder file and instantiate its widgets.
	auto ref_builder = Gtk::Builder::create_from_resource("/com/progweb/gpx2video/ui/window.ui");

	auto window = Gtk::Builder::get_widget_derived<GPX2VideoApplicationWindow>(ref_builder, "application_window");

	if (!window)
		throw std::runtime_error("No \"application_window\" object in window.ui");

	return window;
}


void GPX2VideoApplicationWindow::open_layout_file(const Glib::RefPtr<const Gio::File> &file) {
	log_call();

	layout_file_ = file->get_parse_name();

	// Video renderer
	renderer_->setLayoutFile(layout_file_);

	// Populate widgets list
	auto list = ref_builder_->get_widget<Gtk::ListBox>("widgets_listbox");
	if (!list)
		throw std::runtime_error("No \"widgets_listbox\" object in window.ui");

	for (GPX2VideoWidget *item : renderer_->widgets_) {
		auto box = Gtk::Box(Gtk::Orientation::HORIZONTAL, 0);
		auto label = Gtk::Label(item->widget()->name());
		auto button = Gtk::make_managed<Gtk::Button>(); //(new Gtk::Button());

		label.set_halign(Gtk::Align::START);
		label.set_hexpand(true);
		label.set_valign(Gtk::Align::CENTER);
		label.set_xalign(0.0);
		label.set_yalign(1.0);
	
		button->set_halign(Gtk::Align::END);
		button->set_valign(Gtk::Align::CENTER);
		button->set_icon_name("user-trash-symbolic");

		button->signal_clicked().connect(sigc::bind(
					sigc::mem_fun(*this, &GPX2VideoApplicationWindow::on_widget_remove_clicked), item), true);

		box.set_hexpand(true);
		box.append(label);
		box.append(*button);

		list->append(box);
	}

	// Refresh video preview
	video_area_->update_layout();
}


void GPX2VideoApplicationWindow::open_media_file(const Glib::RefPtr<const Gio::File> &file) {
	log_call();

	Gtk::Label *label;
	VideoStreamPtr stream;
	Glib::DateTime creationtime;

	auto info = file->query_info();

	media_file_ = file->get_parse_name();

	// Probe input media
	media_ = Decoder::probe(media_file_);

	// Video renderer
	renderer_->setMediaContainer(media_);

	// Video resolution
	stream = media_->getVideoStream();
	
	// Video creation time
//#if GLIBMM_CHECK_VERSION(2, 80, 0)
//	creationtime = Glib::DateTime::create_from_utc_usec(media_->creationTime() * 1000);
//#else
//	creationtime = Glib::DateTime::create_from_iso8601(Datetime::timestamp2iso(media_->creationTime()));
//#endif
	creationtime = Glib::DateTime::create_now_local(media_->creationTime() / 1000);

	// Populate video properties
	label = ref_builder_->get_widget<Gtk::Label>("date_label");
#if GLIBMM_CHECK_VERSION(2, 80, 0)
	label->set_label(media_->creationTime() ? creationtime.format("%Ex").c_str() : "-");
#else
	label->set_label(media_->creationTime() ? creationtime.format("%x").c_str() : "-");
#endif
	
	label = ref_builder_->get_widget<Gtk::Label>("time_label");
#if GLIBMM_CHECK_VERSION(2, 80, 0)
	label->set_label(media_->creationTime() ? creationtime.format("%EX").c_str() : "-");
#else
	label->set_label(media_->creationTime() ? creationtime.format("%X").c_str() : "-");
#endif
	
	label = ref_builder_->get_widget<Gtk::Label>("size_label");
	label->set_label(Glib::ustring::sprintf("%d × %d pixels", stream->width(), stream->height()));
	
	label = ref_builder_->get_widget<Gtk::Label>("filesize_label");
	label->set_label(Glib::format_size(info->get_size()));

	// Update video frame
	video_frame_->set_video_selected(media_);

	// Update video area widget
	video_area_->open_stream(media_);
}


void GPX2VideoApplicationWindow::open_telemetry_file(const Glib::RefPtr<const Gio::File> &file) {
	log_call();

	auto filename = file->get_parse_name();

	telemetry_settings_.setTelemetryMethod(TelemetrySettings::MethodInterpolate, 1000);

	// Load telemetry data
	TelemetrySource *source = TelemetryMedia::open(filename, telemetry_settings_, false);

	// Update ui components
	video_area_->set_telemetry(source);

	video_frame_->set_telemetry(source);
}


void GPX2VideoApplicationWindow::update_stack(void) {
	log_call();

//	  auto tab = dynamic_cast<Gtk::ScrolledWindow*>(m_stack->get_visible_child());
//  if (!tab)
//    return;
//
//  auto view = dynamic_cast<Gtk::TextView*>(tab->get_child());
//  if (!view)
//  {
//    std::cout << "ExampleAppWindow::update_words(): No visible text view." << std::endl;
//    return;
//  }
}


void GPX2VideoApplicationWindow::on_action_open(void) {
	log_call();

	// File filter
	auto filter_video = Gtk::FileFilter::create();
	filter_video->set_name("All video files");
	filter_video->add_mime_type("video/*");

	auto filter_layout = Gtk::FileFilter::create();
	filter_layout->set_name("All layout files");
//	filter_layout->add_mime_type("application/xml");
	filter_layout->add_suffix("xml");

	// Create file chooser dialog
#if GTKMM_CHECK_VERSION(4, 10, 0)
	auto dialog = Gtk::FileDialog::create(); 

	dialog->set_title("Open a file");
	dialog->set_modal(true);

	// Add filters, so that only certain file types can be selected:
	auto filters = Gio::ListStore<Gtk::FileFilter>::create();

	filters->append(filter_video);
	filters->append(filter_layout);

	dialog->set_default_filter((media_ == NULL) ? filter_video : filter_layout);
	dialog->set_filters(filters);

	// Bind open file signal
	dialog->open(*this, sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoApplicationWindow::on_file_dialog_open_clicked), dialog));
#else
	auto dialog = new Gtk::FileChooserDialog(*this, "Open a file", Gtk::FileChooserDialog::Action::OPEN);

	dialog->set_transient_for(*this);
	dialog->set_modal(true);
	dialog->set_default_size(640, 480);
	dialog->add_filter(filter_video);
	dialog->add_filter(filter_layout);
	dialog->set_filter((media_ == NULL) ? filter_video : filter_layout);
	dialog->add_button("Ok", Gtk::ResponseType::OK);
	dialog->show();

	dialog->signal_response().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoApplicationWindow::on_file_dialog_open_clicked), dialog));
#endif
}


void GPX2VideoApplicationWindow::on_action_append(void) {
	log_call();

	// File filter
	auto filter_gpx = Gtk::FileFilter::create();
	filter_gpx->set_name("All GPX files");
	filter_gpx->add_mime_type("application/gpx+xml");
//	filter_gpx->add_suffix("gpx");
//	filter_gpx->add_suffix("GPX");

	// Create file chooser dialog
#if GTKMM_CHECK_VERSION(4, 10, 0)
	auto dialog = Gtk::FileDialog::create(); 

	dialog->set_title("Open a telemetry file");
	dialog->set_modal(true);

	// Add filters, so that only certain file types can be selected:
	auto filters = Gio::ListStore<Gtk::FileFilter>::create();

	filters->append(filter_gpx);

	dialog->set_filters(filters);

	// Bind open file signal
	dialog->open(*this, sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoApplicationWindow::on_file_dialog_open_clicked), dialog));
#else
	auto dialog = new Gtk::FileChooserDialog(*this, "Open a telemetry file", Gtk::FileChooserDialog::Action::OPEN);

	dialog->set_transient_for(*this);
	dialog->set_modal(true);
	dialog->set_default_size(640, 480);
	dialog->add_filter(filter_gpx);
	dialog->add_button("Ok", Gtk::ResponseType::OK);
	dialog->show();

	dialog->signal_response().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoApplicationWindow::on_file_dialog_open_clicked), dialog));
#endif
}


void GPX2VideoApplicationWindow::on_video_changed(void) {
	log_call();

	log_info("Video metadata changed");

	video_area_->refresh();
}


void GPX2VideoApplicationWindow::on_widget_changed(void) {
	log_call();

	log_info("Widget properties changed");

	video_area_->refresh();
}


void GPX2VideoApplicationWindow::on_play_clicked(void) {
	log_call();

	video_area_->toggle_pause();

	if (video_area_->is_playing())
		play_button_->set_icon_name("media-playback-pause-symbolic");
	else
		play_button_->set_icon_name("media-playback-start-symbolic");
}


bool GPX2VideoApplicationWindow::on_progress_change_value(Gtk::ScrollType type, double value, 
		const Glib::RefPtr<Gtk::Adjustment> &adjustment) {
	log_call();

	double diff = (value - adjustment->get_value()) / 1000.0;

	(void) type;
//	(void) value;

//	log_notice("PROGESS CHANGE VALUE %f => %f", adjustment->get_value(), value);

	video_area_->seek(diff);

	return false;
}


void GPX2VideoApplicationWindow::on_progress_scale_pressed(guint n, double x, double y) {
	log_call();

	(void) n;
	(void) x;
	(void) y;

//	log_info("PROGRES SCALE PRESSED");

	video_area_->seeking(true);
}


void GPX2VideoApplicationWindow::on_progress_scale_released(guint n, double x, double y) {
	log_call();

	(void) n;
	(void) x;
	(void) y;

//	log_info("PROGRES SCALE RELEASED");

	video_area_->seeking(false);
}


bool GPX2VideoApplicationWindow::on_key_pressed(guint keyvalue, guint rawvalue, Gdk::ModifierType modifier) {
	log_call();

	(void) rawvalue;
	(void) modifier;

	switch (keyvalue) {
	case GDK_KEY_s:
		log_notice("Move to next frame (key 's')");
		video_area_->step_to_next_frame();
		break;

	case GDK_KEY_Left:
		log_notice("Seek -10.0s");
		video_area_->seek(-10.0);
		break;

	case GDK_KEY_Right:
		log_notice("Seek +10.0s");
		video_area_->seek(10.0);
		break;

	case GDK_KEY_Down:
		log_notice("Seek -60.0s");
		video_area_->seek(-60.0);
		break;

	case GDK_KEY_Up:
		log_notice("Seek +60.0s");
		video_area_->seek(60.0);
		break;

	default:
//		log_notice("KEY PRESSED %x", keyvalue);
		return false;
	}

	return true;
}


void GPX2VideoApplicationWindow::on_stack_changed(void) {
	log_call();

	// Video / Telemetry / Widget editing
	Glib::ustring name = info_stack_->get_visible_child_name();

	log_info("Stack changed: %s", name.c_str());

	// Widget append button
	auto button = ref_builder_->get_widget<Gtk::Button>("widget_append_button");
	if (!button)
		throw std::runtime_error("No \"widget_append_button\" object in window.ui");

	button->set_visible((name == "widgets_page"));

	// Update visible frame
	video_frame_->set_visible((name == "video_page"));
	telemetry_frame_->set_visible((name == "telemetry_page"));
	widget_frame_->set_visible((name == "widgets_page"));
}


void GPX2VideoApplicationWindow::on_widget_selected(Gtk::ListBoxRow *row) {
	log_call();

	int index = row->get_index();

	log_info("Widget selected at index: %d", index);

	// Widget item
	int i = 0;

	for (GPX2VideoWidget *item : renderer_->widgets_) {
		if (i++ != index)
			continue;

		widget_frame_->set_widget_selected(item);

		break;
	}
}


void GPX2VideoApplicationWindow::on_widget_append_clicked(void) {
	log_call();

	log_info("Append new widget");
}


void GPX2VideoApplicationWindow::on_widget_remove_clicked(GPX2VideoWidget *widget) {
	log_call();

	log_info("Remove widget '%s'", widget->widget()->name().c_str());
}


#if GTKMM_CHECK_VERSION(4, 10, 0)
void GPX2VideoApplicationWindow::on_file_dialog_open_clicked(const Glib::RefPtr<Gio::AsyncResult> &result,
		const Glib::RefPtr<Gtk::FileDialog> &dialog) {
	log_call();

	auto file = dialog->open_finish(result);

	if (!file)
		return;

	auto info = file->query_info();
	auto type = info->get_content_type();

	log_info("Open %s file with '%s' mimetype", file->get_parse_name().c_str(), std::string(type).c_str());

	// Open layout or media file
	if (type == "application/gpx+xml")
		open_telemetry_file(file);
	else if (type == "application/xml")
		open_layout_file(file);
	else
		open_media_file(file);
}
#else
void GPX2VideoApplicationWindow::on_file_dialog_open_clicked(
		int response_id, Gtk::FileChooserDialog *dialog) {
	log_call();

	Glib::RefPtr<const Gio::File> file = NULL;

	switch (response_id) {
	case Gtk::ResponseType::OK:
		file = dialog->get_file();
		break;

	default:
		break;
	}

	// Close & destroy file chooser dialog
	delete dialog;

	if (!file)
		return;

	// Open layout or media file
	auto info = file->query_info();
	auto type = info->get_content_type();

	log_info("Open %s file with '%s' mimetype", file->get_parse_name().c_str(), std::string(type).c_str());

	if (type == "application/gpx+xml")
		open_telemetry_file(file);
	else if (type == "application/xml")
		open_layout_file(file);
	else
		open_media_file(file);
}
#endif

