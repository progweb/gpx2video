#include <iostream>
#include <memory>

#include "log.h"
#include "datetime.h"
#include "videowidget.h"
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


void GPX2VideoRenderer::restart(void) {
	log_call();

	// Purge old tasks
	app_.purge();

	// Append each widget task
	for (GPX2VideoWidget *item : widgets_)
		app_.append(item->widget());

	// Finaly append renderer
	app_.append(this);
}


void GPX2VideoRenderer::setMediaContainer(MediaContainer *container) {
	log_call();

	if (init(container) == false)
		return;

//	// TOREMOVE !!!
//	container_->setTimeOffset(-139000);
}


void GPX2VideoRenderer::setLayoutFile(const Glib::ustring &layout_file) {
	log_call();

	// Widget drawing...
	is_ready_ = false;

	app_.remove(this);

	// Load layout
	renderer_settings_.setLayoutfile(layout_file);

	// Reset & load each widgets
	reset();
	load();

	// Set last timestamp
	set_timestamp(timestamp_);

	// Register tasks
	app_.append(this);
}


void GPX2VideoRenderer::set_telemetry(TelemetrySource *source) {
	log_call();

	is_ready_ = false;

	// Save telemetry source
	source_ = source;

	// Reset telemetry data
	data_ = TelemetryData();

	// Need to start widget again
	restart();

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


void GPX2VideoRenderer::reset_timestamp(void) {
	log_call();

	// Widget refresh
	refresh();
}


const std::list<GPX2VideoWidget *>& GPX2VideoRenderer::widgets(void) {
	log_call();

	return widgets_;
}


void GPX2VideoRenderer::append(GPX2VideoWidget *widget) {
	log_call();
}


void GPX2VideoRenderer::remove(GPX2VideoWidget *widget) {
	log_call();

	// Remove widget
	widgets_.remove(widget);

	// Destroy widget
	delete widget;

	// Remove widget from core
	Renderer::remove(widget->widget());

	// Refresh
	compute();
	refresh();
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
		GPX2VideoWidget *widget;

		widget = GPX2VideoWidget::create(item);
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

		for (GPX2VideoWidget *item : widgets_) {
			bool loop = true;

			while (loop && !item->full()) {
				TelemetryData data = item->data();

				if (!item->ready() || (data.type() == TelemetryData::TypeUnknown)) {
					timestamp = timestamp_;
					timestamp -= (timestamp_ % rate);

					// Retrieve first point
					source_->retrieveFrom(data);

					// Set timestamp requested
					data.setDatetime(timestamp_);
				}
				else {
					// Continue from the last datetime requested
					timestamp = data.datetime() + rate;
				}

				type = source_->retrieveNext(data, timestamp);

				// Optimize timestamp
				timestamp = data.timestamp();
				timestamp -= (timestamp_ % rate);

				// Save timestamp requested
				data.setDatetime(timestamp);

				item->write_buffers(data, loop);
				
				(void) type;
			}
		}
	}
	else {
		bool loop;

		TelemetryData data;

		// Set timestamp requested
		data.setDatetime(timestamp_);

		for (GPX2VideoWidget *item : widgets_) {
			item->write_buffers(data, loop);
		}
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

	// Broadcast event
	widget_position_dispatcher_.emit();
}


void GPX2VideoRenderer::refresh(GPX2VideoWidget *widget) {
	log_call();

	// Widget drawing...
	is_ready_ = false;

	// Force widgets draw
	clear(widget);

	// Buffering
	go();
}


void GPX2VideoRenderer::init_buffers(void) {
	log_call();

	// Create texture buffers
	for (GPX2VideoWidget *item : widgets_)
		item->init_buffers();

	// Force widget drawing
	refresh();
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

