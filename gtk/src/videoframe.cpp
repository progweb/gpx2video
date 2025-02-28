#include "log.h"
#include "videoframe.h"

#include <gtkmm/entry.h>
#include <gtkmm/button.h>
#include <gtkmm/calendar.h>
#include <gtkmm/popover.h>
#include <gtkmm/spinbutton.h>


GPX2VideoVideoFrame::GPX2VideoVideoFrame()
	: Glib::ObjectBase("GPX2VideoVideoFrame")
	, ref_builder_(NULL)
	, dispatcher_()
	, media_(NULL)
	, source_(NULL) {
	log_call();
}


GPX2VideoVideoFrame::GPX2VideoVideoFrame(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &ref_builder) 
	: Glib::ObjectBase("GPX2VideoVideoFrame")
	, Gtk::Frame(cobject)
	, ref_builder_(ref_builder)
	, dispatcher_()
	, media_(NULL) 
	, source_(NULL) {
	log_call();

	// Connect datetimestart popover
	auto popover = ref_builder_->get_widget<Gtk::Popover>("datetimestart_popover");
	if (!popover)
		throw std::runtime_error("No \"popover\" object in video_frame.ui");

	popover->signal_show().connect(sigc::mem_fun(*this, &GPX2VideoVideoFrame::on_datetimestart_shown));

	// Connect datetimestart button
	auto button = ref_builder_->get_widget<Gtk::Button>("datetimestart_button");
	if (!button)
		throw std::runtime_error("No \"datetimestart_button\" object in video_frame.ui");

	button->signal_clicked().connect(sigc::mem_fun(*this, &GPX2VideoVideoFrame::on_datetimestart_clicked));

	// Update ui
	update_content();
}


GPX2VideoVideoFrame::~GPX2VideoVideoFrame() {
	log_call();
}


void GPX2VideoVideoFrame::set_visible(bool visible) {
	log_call();

	Gtk::Frame::set_visible(visible && (media_ != NULL));
}


void GPX2VideoVideoFrame::set_video_selected(MediaContainer *media) {
	log_call();

	// Save selected video
	media_ = media;

	// Update ui content
	update_content();
}


void GPX2VideoVideoFrame::set_telemetry(TelemetrySource *source) {
	log_call();

	// Save telemetry source
	source_ = source;
}



void GPX2VideoVideoFrame::on_action_use_creation_time(void) {
	log_call();

	Glib::DateTime creationtime;

	if (media_ == NULL)
		return;

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

	// Refresh video preview
	dispatcher_.emit();
}


void GPX2VideoVideoFrame::on_action_use_gpmf_stream(void) {
	log_call();

	log_info("Sync video starttime with GPMF video stream");
}


void GPX2VideoVideoFrame::on_action_use_gpx_data(void) {
	log_call();

	TelemetryData data;

	Glib::DateTime starttime;

	if ((media_ == NULL) || (source_ == NULL))
		return;

	log_info("Sync video starttime with the GPX first point");

	source_->retrieveFirst(data);

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

	// Refresh video preview
	dispatcher_.emit();
}


void GPX2VideoVideoFrame::update_content(void) {
	log_call();

	Glib::DateTime creationtime;

	// Frame is visible only as video is selected
	set_visible((media_ != NULL));

	// No widget
	if (media_ == NULL)
		return;

	// Video creation time
//#if GLIBMM_CHECK_VERSION(2, 80, 0)
//	creationtime = Glib::DateTime::create_from_utc_usec(media_->creationTime() * 1000);
//#else
//	creationtime = Glib::DateTime::create_from_iso8601(Datetime::timestamp2iso(media_->creationTime()));
//#endif
	creationtime = Glib::DateTime::create_now_utc(media_->creationTime() / 1000);

	// Update datetime start entry
	auto entry = ref_builder_->get_widget<Gtk::Entry>("datetimestart_entry");
	if (!entry)
		throw std::runtime_error("No \"datetimestart_entry\" object in video_frame.ui");

	entry->get_buffer()->set_text(media_->creationTime() ? creationtime.format("%Ex %EX").c_str() : "");
}


void GPX2VideoVideoFrame::on_datetimestart_shown(void) {
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
		throw std::runtime_error("No \"datestart_calendar\" object in video_frame.ui");

	calendar->select_day(creationtime);

	// Populate hour time start popover
   	spin = ref_builder_->get_widget<Gtk::SpinButton>("hourstart_spinbutton");
	if (!spin)
		throw std::runtime_error("No \"hourstart_spinbutton\" object in video_frame.ui");

	spin->set_value(creationtime.get_hour());

	// Populate minute time start popover
   	spin = ref_builder_->get_widget<Gtk::SpinButton>("minutestart_spinbutton");
	if (!spin)
		throw std::runtime_error("No \"minutestart_spinbutton\" object in video_frame.ui");

	spin->set_value(creationtime.get_minute());

	// Populate second time start popover
   	spin = ref_builder_->get_widget<Gtk::SpinButton>("secondstart_spinbutton");
	if (!spin)
		throw std::runtime_error("No \"secondstart_spinbutton\" object in video_frame.ui");

	spin->set_value(creationtime.get_second());
}


void GPX2VideoVideoFrame::on_datetimestart_clicked(void) {
	log_call();

	Gtk::Calendar *calendar;
	Gtk::SpinButton *spin;

	Glib::DateTime datetimestart;

	log_info("Datetime start button clicked");

	// Get date start popover
	calendar = ref_builder_->get_widget<Gtk::Calendar>("datestart_calendar");
	if (!calendar)
		throw std::runtime_error("No \"datestart_calendar\" object in video_frame.ui");

//#if GTKMM_CHECK_VERSION(4, 14, 0)
//	datetimestart = Glib::DateTime::create_local(calendar->get_year(), calendar->get_month() + 1, calendar->get_day(), 0, 0, 0);
//#endif
	datetimestart = Glib::DateTime::create_now_local(calendar->get_date().to_unix());

	// Get hour start popover
   	spin = ref_builder_->get_widget<Gtk::SpinButton>("hourstart_spinbutton");
	if (!spin)
		throw std::runtime_error("No \"hourstart_spinbutton\" object in video_frame.ui");

	datetimestart = datetimestart.add_hours(spin->get_value());

	// Get minute start popover
   	spin = ref_builder_->get_widget<Gtk::SpinButton>("minutestart_spinbutton");
	if (!spin)
		throw std::runtime_error("No \"minutestart_spinbutton\" object in video_frame.ui");

	datetimestart = datetimestart.add_minutes(spin->get_value());

	// Get second start popover
   	spin = ref_builder_->get_widget<Gtk::SpinButton>("secondstart_spinbutton");
	if (!spin)
		throw std::runtime_error("No \"secondstart_spinbutton\" object in video_frame.ui");

	datetimestart = datetimestart.add_seconds(spin->get_value());

	// Hide datetime start popover
	auto popover = ref_builder_->get_widget<Gtk::Popover>("datetimestart_popover");
	if (!popover)
		throw std::runtime_error("No \"datetimestart_popover\" object in video_frame.ui");

	popover->popdown();

	// Update media
	media_->setStartTime(datetimestart.to_unix() * 1000);

	// Update datetime start entry
	auto entry = ref_builder_->get_widget<Gtk::Entry>("datetimestart_entry");
	if (!entry)
		throw std::runtime_error("No \"datetimestart_entry\" object in video_frame.ui");

	entry->get_buffer()->set_text(datetimestart.format("%Ex %EX").c_str());

	// Refresh video preview
	dispatcher_.emit();
}


