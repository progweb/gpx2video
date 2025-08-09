#include <iostream>
#include <memory>

#include "log.h"
#include "datetime.h"
#include "widgets/gpx.h"
#include "widgets/date.h"
#include "widgets/distance.h"
#include "widgets/duration.h"
#include "widgets/grade.h"
#include "widgets/heading.h"
#include "widgets/elevation.h"
#include "widgets/cadence.h"
#include "widgets/heartrate.h"
#include "widgets/power.h"
#include "widgets/lap.h"
#include "widgets/position.h"
#include "widgets/image.h"
#include "widgets/speed.h"
#include "widgets/maxspeed.h"
#include "widgets/avgspeed.h"
#include "widgets/avgridespeed.h"
#include "widgets/text.h"
#include "widgets/time.h"
#include "widgets/gforce.h"
#include "widgets/temperature.h"
#include "widgets/verticalspeed.h"
#include "widgets/track.h"
#include "widgets/map.h"
#include "renderer.h"


GPX2VideoRenderer::GPX2VideoRenderer(GPXApplication &app, 
		RendererSettings &renderer_settings, TelemetrySettings &telemetry_settings)
	: Renderer(app, renderer_settings, telemetry_settings) 
	, source_(NULL)
	, dispatcher_() {
	log_call();

	is_ready_ = false;

	seek_pos_ = 0.0;
	seek_req_ = false;

	app.append(this);
}


GPX2VideoRenderer::~GPX2VideoRenderer() {
	log_call();
}


GPX2VideoRenderer * GPX2VideoRenderer::create(GPXApplication &app, 
		RendererSettings &renderer_settings, TelemetrySettings &telemetry_settings,
		MediaContainer *container) {
	log_call();

	GPX2VideoRenderer *renderer = new GPX2VideoRenderer(app, renderer_settings, telemetry_settings);

	if (renderer->init(container) == false)
		goto skip;

	renderer->load();

skip:
	return renderer;
}


bool GPX2VideoRenderer::init(MediaContainer *container) {
	log_call();

	is_ready_ = false;
	
	if (!Renderer::init(container))
		return false;

	// Retrieve video streams
	VideoStreamPtr video_stream = container_->getVideoStream();

	// No orientation
	// Compute layout size from width & height and DAR
	//   DAR = width / height * SAR
	//   SAR = video_stream->pixelAspectRatio()
	switch (video_stream->orientation()) {
	case -90:
	case 90:
	case -270:
	case 270:
		layout_width_ = video_stream->height();
		layout_height_ = round((double) video_stream->width() * av_q2d(video_stream->pixelAspectRatio()));
		break;

	default:
		layout_width_ = round((double) video_stream->width() * av_q2d(video_stream->pixelAspectRatio()));
		layout_height_ = video_stream->height();
		break;
	}

	return true;
}


void GPX2VideoRenderer::setMediaContainer(MediaContainer *container) {
	log_call();

	if (init(container) == false)
		return;

	// TOREMOVE !!!
	container_->setTimeOffset(-139000);
}


void GPX2VideoRenderer::setLayoutFile(const Glib::ustring &layout_file) {
	log_call();

	is_ready_ = false;

	app_.remove(this);

	// Load layout
	renderer_settings_.setLayoutfile(layout_file);

	// Reset & load each widgets
	reset();
	load();

	app_.append(this);

//	// Request buffering
//	refresh();
}


void GPX2VideoRenderer::set_telemetry(TelemetrySource *source) {
	log_call();

	is_ready_ = false;

	// Save telemetry source
	source_ = source;

	// Reset telemetry data
	data_ = TelemetryData();

//	// Set telemetry source for each widget
//	for (GPX2VideoWidget *item : widgets_)
//		item->setTelemetry(source_);

	// Request buffering
	refresh();
}


void GPX2VideoRenderer::set_layout_size(int width, int height) {
	log_call();

	log_info("Set widget layout %d x %d", width, height);

	for (GPX2VideoWidget *item : widgets_)
		item->setLayoutSize(width, height);
}


const uint64_t& GPX2VideoRenderer::time(void) const {
	log_call();

	return timestamp_;
}


void GPX2VideoRenderer::set_timestamp(uint64_t timestamp) {
	log_call();

	int rate = telemetrySettings().telemetryRate();

	// Save current timestamp
	timestamp_ = timestamp;

	// Set timestamp for each widget
	timestamp -= (timestamp % rate);

	for (GPX2VideoWidget *item : widgets_)
		item->set_timestamp(timestamp);
}


const std::list<GPX2VideoWidget *>& GPX2VideoRenderer::widgets(void) {
	log_call();

	return widgets_;
}


void GPX2VideoRenderer::seek(double pos) {
	log_call();

	log_info("Widgets seek position %s", Datetime::timestamp2string(pos, Datetime::FormatDatetime).c_str());

	if (!seek_req_) {
		seek_pos_ = pos;
		seek_req_ = true;
	}

	// Request buffering
	refresh();
}


void GPX2VideoRenderer::load(void) {
	log_call();

	std::string s;

	// Load layout & widget initialization
	Renderer::load();
	Renderer::computeWidgetsPosition();

	for (VideoWidget *item : Renderer::widgets_) {
		GPX2VideoWidget *widget; // = GPX2VideoWidget::create(item);

		// Type
		s = item->name();

		if (s == "gpx")
			widget = GPX2VideoGPXWidget::create(item);
		else if (s == "date") 
			widget = GPX2VideoDateWidget::create(item);
		else if (s == "time") 
			widget = GPX2VideoTimeWidget::create(item);
		else if (s == "text") 
			widget = GPX2VideoTextWidget::create(item);
		else if (s == "distance") 
			widget = GPX2VideoDistanceWidget::create(item);
		else if (s == "duration") 
			widget = GPX2VideoDurationWidget::create(item);
		else if (s == "position") 
			widget = GPX2VideoPositionWidget::create(item);
		else if (s == "speed") 
			widget = GPX2VideoSpeedWidget::create(item);
		else if (s == "maxspeed") 
			widget = GPX2VideoMaxSpeedWidget::create(item);
		else if (s == "avgspeed") 
			widget = GPX2VideoAvgSpeedWidget::create(item);
		else if (s == "avgridespeed") 
			widget = GPX2VideoAvgRideSpeedWidget::create(item);
		else if (s == "grade") 
			widget = GPX2VideoGradeWidget::create(item);
		else if (s == "heading") 
			widget = GPX2VideoHeadingWidget::create(item);
		else if (s == "image")
			widget = GPX2VideoImageWidget::create(item);
		else if (s == "elevation") 
			widget = GPX2VideoElevationWidget::create(item);
		else if (s == "cadence") 
			widget = GPX2VideoCadenceWidget::create(item);
		else if (s == "heartrate") 
			widget = GPX2VideoHeartRateWidget::create(item);
		else if (s == "temperature")
			widget = GPX2VideoTemperatureWidget::create(item);
		else if (s == "power") 
			widget = GPX2VideoPowerWidget::create(item);
		else if (s == "gforce")
			widget = GPX2VideoGForceWidget::create(item);
		else if (s == "vspeed")
			widget = GPX2VideoVerticalSpeedWidget::create(item);
		else if (s == "lap")
			widget = GPX2VideoLapWidget::create(item);
		else if (s == "track")
			widget = GPX2VideoTrackWidget::create(item);
		else if (s == "map")
			widget = GPX2VideoMapWidget::create(item);
		else {
			log_error("Widget loading error, '%s' type unknown", s.c_str());
			continue;
		}
//		widget->setRate(telemetrySettings().telemetryRate());
//		widget->setTelemetry(source_);

		widgets_.push_back(widget);
	}
}


void GPX2VideoRenderer::reset(void) {
	log_call();

	// Drop widgets list
	while (!widgets_.empty()) {
		GPX2VideoWidget *widget = widgets_.front();

		widgets_.pop_front();

		delete widget;
	}

	// Drop widgets parent list
	Renderer::drop();
}


void GPX2VideoRenderer::draw(void) {
	log_call();

	uint64_t timestamp;

	int rate = telemetrySettings().telemetryRate();

	if (widgets_.empty())
		return;

	if (source_) {
		TelemetrySource::Data type = TelemetrySource::DataUnknown;

		if (data_.type() == TelemetryData::TypeUnknown) {
			timestamp = timestamp_;
			timestamp -= (timestamp_ % rate);

			// Retrieve first point
			source_->retrieveFrom(data_);
			data_.setDatetime(timestamp_);
		}
		else {
			// Continue from the last datetime
			timestamp = data_.datetime() + rate;
		}

		while ((type != TelemetrySource::DataEof) && !full_buffers()) {
			type = source_->retrieveNext(data_, timestamp);
			data_.setDatetime(timestamp);

			write_buffers();

			timestamp += rate;
		}
	}
	else {
		data_ = TelemetryData();

		write_buffers();
	}
}


void GPX2VideoRenderer::clear(GPX2VideoWidget *widget) {
	log_call();

	for (GPX2VideoWidget *item : widgets_) {
		if ((widget != NULL) && (widget != item))
			continue;

		item->clear();
	}
}


void GPX2VideoRenderer::compute(void) {
	log_call();

	// Update widgets position
	Renderer::computeWidgetsPosition();
}


void GPX2VideoRenderer::refresh(GPX2VideoWidget *widget) {
	log_call();

	// Widget drawing...
	is_ready_ = false;

	// Force widgets draw
	clear(widget);

	// Buffering
	schedule();
}


void GPX2VideoRenderer::init_buffers(void) {
	log_call();

	// Create texture buffers
	for (GPX2VideoWidget *item : widgets_)
		item->init_buffers();

	// Force widget drawing
	refresh();
}


bool GPX2VideoRenderer::full_buffers(void) {
	log_call();

	for (GPX2VideoWidget *item : widgets_)
		if (item->full())
			return true;

	return false;
}


void GPX2VideoRenderer::write_buffers(void) {
	log_call();

	for (GPX2VideoWidget *item : widgets_)
		item->write_buffers(data_);
}


/**
 * Load OpenGL texture
 *
 * Called from GTK main thread
 */
void GPX2VideoRenderer::load_texture(void) {
	log_call();

	for (GPX2VideoWidget *item : widgets_)
		item->load_texture();
}


/**
 * Render OpenGL texture
 *
 * Called from GTK main thread in OpenGL context
 */
void GPX2VideoRenderer::render(GPX2VideoShader *shader) {
	log_call();

	for (GPX2VideoWidget *item : widgets_)
		item->render(shader);
}


void GPX2VideoRenderer::update(uint64_t timestamp) {
	log_call();

	// Update each widget
	set_timestamp(timestamp);

	schedule();
}


bool GPX2VideoRenderer::run(void) {
	log_call();

	// Seek
	if (seek_req_) {
		is_ready_ = false;

		// Reset telemetry data
		data_ = TelemetryData();

		// Move to new timestamp
		set_timestamp(seek_pos_);

		seek_req_ = false;
	}

	// Draw each widget
	draw();

	if (!is_ready_) {
		is_ready_ = true;

		dispatcher_.emit();
	}

	return true;
}

