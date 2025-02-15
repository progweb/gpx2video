#include <iostream>
#include <memory>

#include "log.h"
#include "renderer.h"


GPX2VideoRenderer::GPX2VideoRenderer(GPXApplication &app, 
		RendererSettings &renderer_settings, TelemetrySettings &telemetry_settings)
	: Renderer(app, renderer_settings, telemetry_settings) {
}


GPX2VideoRenderer::~GPX2VideoRenderer() {
}


GPX2VideoRenderer * GPX2VideoRenderer::create(GPXApplication &app, 
		RendererSettings &renderer_settings, TelemetrySettings &telemetry_settings,
		MediaContainer *container) {
	GPX2VideoRenderer *renderer = new GPX2VideoRenderer(app, renderer_settings, telemetry_settings);

	if (renderer->init(container) == false)
		goto skip;

	renderer->load();
	renderer->computeWidgetsPosition();

skip:
	return renderer;
}


bool GPX2VideoRenderer::init(MediaContainer *container) {
	log_call();

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
}


void GPX2VideoRenderer::setLayoutFile(const Glib::ustring &layout_file) {
	log_call();

	// Load layout
	renderer_settings_.setLayoutfile(layout_file);

	load();
	computeWidgetsPosition();

	for (VideoWidget *item : Renderer::widgets_) {
		GPX2VideoWidget *widget = GPX2VideoWidget::create(item);

		widgets_.push_back(widget);
	}
}

