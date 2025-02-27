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
#include <gtkmm/calendar.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/colorbutton.h>

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
	Gtk::ColorButton *colorbutton;

	// Set the window icon.
	Gtk::IconTheme::get_for_display(get_display())->add_resource_path("/com/progweb/gpx2video/icons");
//	set_default_icon_name("com.progweb.gpx2video");
	set_icon_name("com.progweb.gpx2video");
//	set_theme_name("gpx2video");

	// Get widgets from the Gtk::Builder file
//    stack_ = ref_builder_->get_widget<Gtk::Stack>("stack");
//    if (!stack_)
//        throw std::runtime_error("No \"stack\" object in window.ui");

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
	auto menu_builder = Gtk::Builder::create_from_resource("/com/progweb/gpx2video/ui/gears_menu.ui");
	auto menu = menu_builder->get_object<Gio::MenuModel>("menu");
	if (!menu)
		throw std::runtime_error("No \"main_menu\" object in gears_menu.ui");

	gears_->set_menu_model(menu);

	// Add actions and keyboard accelerators for the headerbar actions
	add_action("open", sigc::mem_fun(*this, &GPX2VideoApplicationWindow::on_action_open));
	add_action("append", sigc::mem_fun(*this, &GPX2VideoApplicationWindow::on_action_append));

	add_action("use_creation_time", sigc::mem_fun(*this, &GPX2VideoApplicationWindow::on_action_use_creation_time));
	add_action("use_gpmf_stream", sigc::mem_fun(*this, &GPX2VideoApplicationWindow::on_action_use_gpmf_stream));
	add_action("use_gpx_data", sigc::mem_fun(*this, &GPX2VideoApplicationWindow::on_action_use_gpx_data));

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

	colorbutton = ref_builder_->get_widget<Gtk::ColorButton>("background_color_button");
	if (!colorbutton)
		throw std::runtime_error("No \"background_color_button\" object in window.ui");
	colorbutton->signal_color_set().connect(sigc::mem_fun(*this, &GPX2VideoApplicationWindow::on_widget_background_color_set));

	// Connect datetimestart popover
	auto popover = ref_builder_->get_widget<Gtk::Popover>("datetimestart_popover");
	if (!popover)
		throw std::runtime_error("No \"popover\" object in window.ui");

	popover->signal_show().connect(sigc::mem_fun(*this, &GPX2VideoApplicationWindow::on_datetimestart_shown));

	// Connect datetimestart button
	button = ref_builder_->get_widget<Gtk::Button>("datetimestart_button");
	if (!button)
		throw std::runtime_error("No \"datetimestart_button\" object in window.ui");

	button->signal_clicked().connect(sigc::mem_fun(*this, &GPX2VideoApplicationWindow::on_datetimestart_clicked));

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

//	// Settings
//	settings().setInputfile("/data/video/2023-08-07 - La Planche des Belles Filles/activity_11733414834.gpx");

	// Probe input media
	media_ = Decoder::probe(media_file_);

	// Video renderer
	renderer_->setMediaContainer(media_);

//	// Video renderer
//	renderer_->setLayoutFile("/data/gpx2video/samples/my-layout-simple.xml");
//	video_area_->update_layout();

	// Video resolution
	stream = media_->getVideoStream();
	
	// Video creation time
//#if GLIBMM_CHECK_VERSION(2, 80, 0)
//	creationtime = Glib::DateTime::create_from_utc_usec(media_->creationTime() * 1000);
//#else
//	creationtime = Glib::DateTime::create_from_iso8601(Datetime::timestamp2iso(media_->creationTime()));
//#endif
	creationtime = Glib::DateTime::create_now_utc(media_->creationTime() / 1000);

	// Populate video properties
	label = ref_builder_->get_widget<Gtk::Label>("date_label");
	label->set_label(media_->creationTime() ? creationtime.format("%Ex").c_str() : "-");
	
	label = ref_builder_->get_widget<Gtk::Label>("time_label");
	label->set_label(media_->creationTime() ? creationtime.format("%EX").c_str() : "-");
	
	label = ref_builder_->get_widget<Gtk::Label>("size_label");
	label->set_label(Glib::ustring::sprintf("%d × %d pixels", stream->width(), stream->height()));
	
	label = ref_builder_->get_widget<Gtk::Label>("filesize_label");
	label->set_label(Glib::format_size(info->get_size()));

	// Update datetime start entry
	auto entry = ref_builder_->get_widget<Gtk::Entry>("datetimestart_entry");
	if (!entry)
		throw std::runtime_error("No \"datetimestart_entry\" object in window.ui");

	entry->get_buffer()->set_text(media_->creationTime() ? creationtime.format("%Ex %EX").c_str() : "");

	// Update video area widget
	video_area_->open_stream(media_);
}


void GPX2VideoApplicationWindow::open_telemetry_file(const Glib::RefPtr<const Gio::File> &file) {
	log_call();

	auto filename = file->get_parse_name();

	telemetry_settings_.setTelemetryMethod(TelemetrySettings::MethodInterpolate, 1000);

	video_area_->open_telemetry(filename);
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


void GPX2VideoApplicationWindow::on_action_use_creation_time(void) {
	log_call();

	Glib::DateTime creationtime;

	// Video creation time
//#if GLIBMM_CHECK_VERSION(2, 80, 0)
//	creationtime = Glib::DateTime::create_from_utc_usec(media_->creationTime() * 1000);
//#else
//	creationtime = Glib::DateTime::create_from_iso8601(Datetime::timestamp2iso(media_->creationTime()));
//#endif
	creationtime = Glib::DateTime::create_now_utc(media_->creationTime() / 1000);

	if (!media_->creationTime())
		return;

	log_info("Sync video starttime with creation time video metadata");

	// Set video start time
	media_->setStartTime(media_->creationTime());

	// Update datetime start entry
	auto entry = ref_builder_->get_widget<Gtk::Entry>("datetimestart_entry");
	if (!entry)
		throw std::runtime_error("No \"datetimestart_entry\" object in window.ui");

	entry->get_buffer()->set_text(creationtime.format("%Ex %EX").c_str());
}


void GPX2VideoApplicationWindow::on_action_use_gpmf_stream(void) {
	log_call();

	log_info("Sync video starttime with GPMF video stream");
}


void GPX2VideoApplicationWindow::on_action_use_gpx_data(void) {
	log_call();

	TelemetryData data;

	TelemetrySource *source;

	Glib::DateTime starttime;

	// Telemetry source
	source = video_area_->telemetry();

	if (source == NULL)
		return;

	log_info("Sync video starttime with the GPX first point");

	source->retrieveFirst(data);

	// Use first point as start time
	media_->setStartTime(data.timestamp());

	// Convert video start time
//#if GLIBMM_CHECK_VERSION(2, 80, 0)
//	starttime = Glib::DateTime::create_from_utc_usec(media_->startTime() * 1000);
//#else
//	starttime = Glib::DateTime::create_from_iso8601(Datetime::timestamp2iso(media_->startTime()));
//#endif
	starttime = Glib::DateTime::create_now_utc(media_->startTime() / 1000);

	// Update datetime start entry
	auto entry = ref_builder_->get_widget<Gtk::Entry>("datetimestart_entry");
	if (!entry)
		throw std::runtime_error("No \"datetimestart_entry\" object in window.ui");

	entry->get_buffer()->set_text(starttime.format("%Ex %EX").c_str());
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


void GPX2VideoApplicationWindow::on_datetimestart_shown(void) {
	log_call();

	Gtk::Calendar *calendar;
	Gtk::SpinButton *spin;

	Glib::DateTime creationtime;

	log_info("Datetime start popover shown");

	if (media_ == NULL)
		return;

	// Video creation time
//#if GLIBMM_CHECK_VERSION(2, 80, 0)
//	creationtime = Glib::DateTime::create_from_utc_usec(media_->creationTime() * 1000);
//#else
//	creationtime = Glib::DateTime::create_from_iso8601(Datetime::timestamp2iso(media_->creationTime()));
//#endif
	creationtime = Glib::DateTime::create_now_utc(media_->creationTime() / 1000);

	// Populate date start popover
	calendar = ref_builder_->get_widget<Gtk::Calendar>("datestart_calendar");
	if (!calendar)
		throw std::runtime_error("No \"datestart_calendar\" object in window.ui");

	calendar->select_day(creationtime);

	// Populate hour time start popover
   	spin = ref_builder_->get_widget<Gtk::SpinButton>("hourstart_spinbutton");
	if (!spin)
		throw std::runtime_error("No \"hourstart_spinbutton\" object in window.ui");

	spin->set_value(creationtime.get_hour());

	// Populate minute time start popover
   	spin = ref_builder_->get_widget<Gtk::SpinButton>("minutestart_spinbutton");
	if (!spin)
		throw std::runtime_error("No \"minutestart_spinbutton\" object in window.ui");

	spin->set_value(creationtime.get_minute());

	// Populate second time start popover
   	spin = ref_builder_->get_widget<Gtk::SpinButton>("secondstart_spinbutton");
	if (!spin)
		throw std::runtime_error("No \"secondstart_spinbutton\" object in window.ui");

	spin->set_value(creationtime.get_second());
}


void GPX2VideoApplicationWindow::on_datetimestart_clicked(void) {
	log_call();

	Gtk::Calendar *calendar;
	Gtk::SpinButton *spin;

	Glib::DateTime datetimestart;

	log_info("Datetime start button clicked");

	// Get date start popover
	calendar = ref_builder_->get_widget<Gtk::Calendar>("datestart_calendar");
	if (!calendar)
		throw std::runtime_error("No \"datestart_calendar\" object in window.ui");

//#if GTKMM_CHECK_VERSION(4, 14, 0)
//	datetimestart = Glib::DateTime::create_local(calendar->get_year(), calendar->get_month() + 1, calendar->get_day(), 0, 0, 0);
//#endif
	datetimestart = Glib::DateTime::create_now_local(calendar->get_date().to_unix());

	// Get hour start popover
   	spin = ref_builder_->get_widget<Gtk::SpinButton>("hourstart_spinbutton");
	if (!spin)
		throw std::runtime_error("No \"hourstart_spinbutton\" object in window.ui");

	datetimestart = datetimestart.add_hours(spin->get_value());

	// Get minute start popover
   	spin = ref_builder_->get_widget<Gtk::SpinButton>("minutestart_spinbutton");
	if (!spin)
		throw std::runtime_error("No \"minutestart_spinbutton\" object in window.ui");

	datetimestart = datetimestart.add_minutes(spin->get_value());

	// Get second start popover
   	spin = ref_builder_->get_widget<Gtk::SpinButton>("secondstart_spinbutton");
	if (!spin)
		throw std::runtime_error("No \"secondstart_spinbutton\" object in window.ui");

	datetimestart = datetimestart.add_seconds(spin->get_value());

	// Hide datetime start popover
	auto popover = ref_builder_->get_widget<Gtk::Popover>("datetimestart_popover");
	if (!popover)
		throw std::runtime_error("No \"datetimestart_popover\" object in window.ui");

	popover->popdown();

	// Update datetime start entry
	auto entry = ref_builder_->get_widget<Gtk::Entry>("datetimestart_entry");
	if (!entry)
		throw std::runtime_error("No \"datetimestart_entry\" object in window.ui");

	entry->get_buffer()->set_text(datetimestart.format("%Ex %EX").c_str());
}


void GPX2VideoApplicationWindow::on_stack_changed(void) {
	log_call();

	Gtk::Frame *frame = NULL;

	// Video / Telemetry / Widget editing
	Glib::ustring name = info_stack_->get_visible_child_name();

	log_info("Stack changed: %s", name.c_str());

	// Widget append button
	auto button = ref_builder_->get_widget<Gtk::Button>("widget_append_button");
	if (!button)
		throw std::runtime_error("No \"widget_append_button\" object in window.ui");

	button->set_visible((name == "widgets_page"));

	// Video frame
	frame = ref_builder_->get_widget<Gtk::Frame>("video_frame");

	frame->set_visible((name == "video_page"));

	// Telemetry frame
	frame = ref_builder_->get_widget<Gtk::Frame>("telemetry_frame");

	frame->set_visible((name == "telemetry_page"));

	// Widget frame
	frame = ref_builder_->get_widget<Gtk::Frame>("widget_frame");

	frame->set_visible((name == "widgets_page"));
}


void GPX2VideoApplicationWindow::on_widget_selected(Gtk::ListBoxRow *row) {
	log_call();

	Gdk::RGBA rgba;

	int index = row->get_index();

	log_info("Widget selected at index: %d", index);

	// Widget color button
	auto button = ref_builder_->get_widget<Gtk::ColorButton>("background_color_button");
	if (!button)
		throw std::runtime_error("No \"background_color_button\" object in window.ui");

	// Widget item
	int i = 0;

	for (GPX2VideoWidget *item : renderer_->widgets_) {
		if (i++ != index)
			continue;

		const float *color = item->widget()->backgroundColor();

		rgba.set_rgba(color[0], color[1], color[2], color[3]);

		button->set_rgba(rgba);

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


void GPX2VideoApplicationWindow::on_widget_background_color_set(void) {
	log_call();

	Gdk::RGBA rgba;

	Glib::ustring color;

	// Widget color button
	auto button = ref_builder_->get_widget<Gtk::ColorButton>("background_color_button");
	if (!button)
		throw std::runtime_error("No \"background_color_button\" object in window.ui");

	rgba = button->get_rgba();

	// Convert to hexa string color
	color = Glib::ustring::sprintf("#%02X%02X%02X%02X",
			(unsigned char) std::round(rgba.get_red() * 255),
			(unsigned char) std::round(rgba.get_green() * 255),
			(unsigned char) std::round(rgba.get_blue() * 255),
			(unsigned char) std::round(rgba.get_alpha() * 255)
	);

	log_info("Background color changed to '%s'", color.c_str());

	// Widgets list
	auto list = ref_builder_->get_widget<Gtk::ListBox>("widgets_listbox");
	if (!list)
		throw std::runtime_error("No \"widgets_listbox\" object in window.ui");

	int index = list->get_selected_row()->get_index();

	// Widget item
	int i = 0;

	for (GPX2VideoWidget *item : renderer_->widgets_) {
		if (i++ != index)
			continue;

		item->widget()->setBackgroundColor(color);

		break;
	}

	// Refresh
	video_area_->refresh();
}


void GPX2VideoApplicationWindow::on_widget_border_color_clicked(void) {
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

