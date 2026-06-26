#include <iostream>
#include <memory>
#include <ranges>

#include "log_i.h"
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

	rate_ = 1;

	seek_pos_ = 0.0;
	seek_req_ = false;

	timestamp_ = 0;
	player_timestamp_ = 0;

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
	for (GPX2VideoWidget *item : widgets_) {
		if (!item->widget()->visible())
			continue;

		app_.append(item->widget());
	}

	// Finaly append renderer
	app_.append(this);
}


void GPX2VideoRenderer::setMediaContainer(MediaContainer *container) {
	log_call();

	if (init(container) == false)
		return;

	compute_telemetry_rate();

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

	// Update settings
	update_telemetry_settings();

	// Reset telemetry data
	data_ = TelemetryData();

	// TMP. Assign source to widgets
	for (GPX2VideoWidget *item : widgets_)
		item->widget()->setTelemetrySource(source);

	// Restart each widget & request buffering
	refresh(NULL, true);
}


void GPX2VideoRenderer::compute_telemetry_rate(void) {
	log_call();

	int video_framerate;

	if ((container_ == NULL) || (source_ == NULL))
		return;
	
	// Retrieve video streams
	VideoStreamPtr video_stream = container_->getVideoStream();

	// Video framerate
	video_framerate = round(1000.0 * av_q2d(av_inv_q(video_stream->frameRate())));

	// Telemetry rate can be less video framerate
	rate_ = std::max(video_framerate, telemetrySettings().telemetryRate());

	if (rate_ > (uint64_t) telemetrySettings().telemetryRate())
		log_info("Telemetry rate set to match video framerate %ld ms", rate_);
	else
		log_info("Telemetry rate set to %ld ms", rate_);
}


void GPX2VideoRenderer::update_telemetry_settings(void) {
	log_call();

	telemetrySettings().copy(source_->settings());

	computeTelemetryRange();

	compute_telemetry_rate();
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

	// Save current timestamp
	timestamp_ = timestamp;

//	// Set timestamp for each widget
//	timestamp -= (timestamp % rate_);

	for (GPX2VideoWidget *item : widgets_)
		item->set_timestamp(timestamp);
}


/**
 * Called as the video starttime is changed
 */
void GPX2VideoRenderer::reset_timestamp(void) {
	log_call();

	// Update each with the new timestamp
	if (container_->startTime() != 0)
		set_timestamp(container_->startTime() + player_timestamp_);
	else 
		set_timestamp(0);

	// Schedule each widget & request buffering
	refresh(NULL, true);
}


const std::list<GPX2VideoWidget *>& GPX2VideoRenderer::widgets(void) {
	log_call();

	return widgets_;
}


void GPX2VideoRenderer::append(VideoWidget::Widget type) {
	log_call();

	VideoWidget *widget;
	GPX2VideoWidget *item;

	// Widget drawing...
	app_.remove(this);

	// Create widget from type
	widget = Renderer::create(type, source_);

	// Create gtk widget widget item
	item = GPX2VideoWidget::create(widget);

	// Center
	widget->setPosition(
		(layout_width_ - widget->theme().width()) / 2, 
		(layout_height_ - widget->theme().height()) / 2
	);

	// Initialize
	item->init_buffers();
	item->set_timestamp(timestamp_);
	item->setLayoutSize(layout_width_, layout_height_);

	// Notice
	log_notice("Append new widget '%s' at %dx%d [size: %dx%d]", 
			VideoWidget::widget2string(widget->type()).c_str(),
			widget->x(), widget->y(),
			widget->theme().width(), widget->theme().height());

	// Append the new item
	widgets_.push_back(item);

	// Register tasks
	app_.append(this);

	// Broadcast event
	signal_widget_appened_.emit(item);

	// Force widget drawing
	refresh();
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


GPX2VideoWidget * GPX2VideoRenderer::get_at(const double &x, const double &y) {
	log_call();

	GPX2VideoWidget *item;

// C++20
//	for (GPX2VideoWidget *item : widgets_ | std::views::reverse) {
	for (auto it = widgets_.rbegin(); it != widgets_.rend(); ++it) {
		item = *it;

		if (!item->widget()->visible())
			continue;

		if (!item->is_over(x, y))
			continue;

		return item;
	}

	return NULL;
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

	if (widgets_.empty())
		return;

	if (source_) {
//		uint64_t clock = get_system_clock();

		for (GPX2VideoWidget *item : widgets_) {
			int max = 3;

			bool loop = true;

			TelemetrySource::Data type = TelemetrySource::DataUnknown;

			if (!item->widget()->visible())
				continue;

			while (loop && !item->full() && (type != TelemetrySource::DataEof) && (max-- > 0)) {
				TelemetryData data = item->data();

				if (!item->ready() || (data.type() == TelemetryData::TypeUnknown)) {
					timestamp = timestamp_;
//					timestamp -= (timestamp_ % rate_);

					// Retrieve first point
					source_->retrieveFrom(data);

					// Set timestamp requested
					data.setDatetime(timestamp_);
				}
				else {
					// Continue from the previous point 
					timestamp = data.timestamp() + rate_;
				}

				type = source_->retrieveNext(data, timestamp);

				// Optimize timestamp
				timestamp = data.timestamp();
//				timestamp -= (timestamp_ % rate_);

				// Save timestamp requested
				data.setDatetime(timestamp);

				item->write_buffers(data, loop);
			}
		}

//		printf("RENDERING DURATION: %ld us\n", get_system_clock() - clock);
	}
	else {
		bool loop;

		TelemetryData data;

		// Set timestamp requested
		data.setDatetime(timestamp_);

		for (GPX2VideoWidget *item : widgets_) {
			if (!item->widget()->visible())
				continue;

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
	signal_widget_position_changed_.emit(NULL);
}


void GPX2VideoRenderer::refresh(GPX2VideoWidget *widget, bool schedule) {
	log_call();

	// Widget drawing...
	is_ready_ = false;

	// Widget settings has changed & need to be scheduled
	if (schedule) {
		Task::reset();

		if (widget == NULL) {
			for (GPX2VideoWidget *item : widgets_) {
				if (!item->widget()->visible())
					continue;

				app_.insert(item->widget(), this);
			}
		}
		else
			app_.insert(widget->widget(), this);
	}

	// Force widgets draw
	clear(widget);

	// Buffering
	go();
}


void GPX2VideoRenderer::stats(void) {
	log_call();

	for (GPX2VideoWidget *item : widgets_) {
		if (!item->widget()->visible())
			continue;

		item->stats();
	}
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

	uint64_t timestamp;

	for (GPX2VideoWidget *item : widgets_) {
		if (!item->widget()->visible())
			continue;

		timestamp = item->load_texture();

		if ((timestamp > 0) && (timestamp_ > (timestamp + rate_)))
//			log_warn("Widget '%s' texture delayed: %ld ms (rate: %ld ms)!", 
//					item->name().c_str(),
//					timestamp_ - timestamp, rate_);

			item->texture_delayed();
	}
}


/**
 * Render OpenGL texture
 *
 * Called from GTK main thread in OpenGL context
 */
void GPX2VideoRenderer::render(GPX2VideoShader *shader) {
	log_call();

	for (GPX2VideoWidget *item : widgets_) {
		if (!item->widget()->visible())
			continue;

		item->render(shader);
	}
}


/**
 * Called from video player.
 * timestamp is the progress time in the video stream (in ms)
 */
void GPX2VideoRenderer::update(uint64_t timestamp) {
	log_call();

	// Save video player timestamp
	player_timestamp_ = timestamp;

	// Update each widget with the new timestamp
	if (container_->startTime() != 0) {
		timestamp += container_->startTime();
	
		set_timestamp(timestamp);

		schedule();
	}
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

