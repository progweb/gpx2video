#include <iostream>
#include <memory>

#include <OpenImageIO/imageio.h>
#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>

#include "macros.h"
#include "oiioutils.h"
#include "imagerenderer.h"


ImageRenderer::ImageRenderer(GPXApplication &app, 
		RendererSettings &renderer_settings, TelemetrySettings &telemetry_settings)
	: Renderer(app, renderer_settings, telemetry_settings)
	, timecode_(0)
	, started_at_(0) {
}


ImageRenderer::~ImageRenderer() {
}


ImageRenderer * ImageRenderer::create(GPXApplication &app, 
		RendererSettings &renderer_settings, TelemetrySettings &telemetry_settings,
		MediaContainer *container) {
	ImageRenderer *renderer = new ImageRenderer(app, renderer_settings, telemetry_settings);

	renderer->init(container);
	renderer->load();
	renderer->computeWidgetsPosition();

	return renderer;
}


bool ImageRenderer::init(MediaContainer *container) {
	Renderer::init(container);

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

	// Compute duration
	duration_ms_ = video_stream->duration() * av_q2d(video_stream->timeBase()) * 1000;

	// Hack - if maxDuration == -1, render whole gpx
	if (app_.settings().maxDuration() == (unsigned int) -1) {
		source_->retrieveLast(data_);

		duration_ms_ = data_.time() * 1000;

		source_->retrieveFirst(data_);

		duration_ms_ -= data_.time() * 1000;
	}
	// If maxDuration set by the user
	else if (app_.settings().maxDuration() > 0) 
		duration_ms_ = MIN(duration_ms_, app_.settings().maxDuration());

	snprintf(duration_, sizeof(duration_), "%02d:%02d:%02d.%03d", 
		(unsigned int) (duration_ms_ / 3600000), (unsigned int) ((duration_ms_ / 60000) % 60), (unsigned int) ((duration_ms_ / 1000) % 60), (unsigned int) (duration_ms_ % 1000));
	duration_[sizeof(duration_) - 1] = '\0';

	return true;
}


bool ImageRenderer::start(void) {
	bool is_update = false;

	time_t now = time(NULL);

//	time_t start_time;

	VideoStreamPtr video_stream = container_->getVideoStream();

	log_call();

	log_notice("Rendering...");

//	// Compute start time
//	start_time = container_->startTime() + container_->timeOffset();
//
//	// Update start time in GPX stream (start_time can change after sync step)
//	if (gpx_) {
//		gpx_->setStartTime(start_time);
////		data_.init();
//	}

	started_at_ = now;

	// Start
	timecode_ = 0;

	// Create overlay buffer
	overlay_ = new OIIO::ImageBuf(OIIO::ImageSpec(layout_width_, layout_height_,
		4, OIIOUtils::getOIIOBaseTypeFromFormat(video_stream->format())));

	// Prepare each widget, map...
	for (VideoWidget *widget : widgets_) {
		OIIO::ImageBuf *buf = NULL;

		uint64_t begin = widget->atBeginTime();
		uint64_t end = widget->atEndTime();

		if ((begin != 0) || (end != 0))
			continue;

		buf = widget->prepare(is_update);

		if (buf == NULL)
			continue;

		// Image over
		buf->specmod().x = widget->x();
		buf->specmod().y = widget->y();
		OIIO::ImageBufAlgo::over(*overlay_, *buf, *overlay_, buf->roi());
	}

	return true;
}


bool ImageRenderer::run(void) {
	char buf[128];
	int pos = -1;
	size_t len = 0;

	time_t start_time;

	uint64_t timecode_ms;

	double time_factor;

	enum TelemetrySource::Data type;

	bool is_update = false;

	std::string filename = app_.settings().outputfile();

	// Create image buffer
	OIIO::ImageBuf image_buffer(overlay_->spec()); 

	// Compute start time
	time_factor = rendererSettings().timeFactor();

	start_time = container_->startTime() + container_->timeOffset();

	// Render image each 1 second
	timecode_ms = timecode_ * 1000;

	if (timecode_ms >= duration_ms_)
		goto done;

	// Build filename from template
	snprintf(buf, sizeof(buf), "%ld", timecode_);
	buf[STRLEN(buf)] = '\0';

	// Search & replace 'XXXXXX' in the filename
	for (int i=filename.length()-1; i>= 0; i--) {
		if (filename.at(i) == '/')
			break;

		if ((len == 0) && (filename.at(i) != 'X'))
			continue;

		if ((len > 0) && (filename.at(i) != 'X'))
			break;

		pos = i;
		filename.at(pos) = '0';

		len++;
	}

	if (pos != -1) {
		if (len > strlen(buf))
			pos += len - strlen(buf);

		filename.replace(pos, MIN(len, strlen(buf)), buf);
	}

	// Compute video time
	app_.setTime(start_time + ((time_factor * timecode_ms) / 1000));

	if (source_) {
		// Save image
		std::unique_ptr<OIIO::ImageOutput> out = OIIO::ImageOutput::create(filename);

		// Read GPX data
		type = source_->retrieveNext(data_, (start_time * 1000) + (time_factor * timecode_ms));

		if (type == TelemetrySource::DataEof)
			goto done;

		// Draw overlay
		OIIO::ImageBufAlgo::over(image_buffer, *overlay_, image_buffer, OIIO::ROI());

		// Draw each widget, map...
		for (VideoWidget *widget : widgets_) {
			OIIO::ImageBuf *buf = NULL;

			uint64_t begin = widget->atBeginTime();
			uint64_t end = widget->atEndTime();

			if ((begin != 0) && (timecode_ms < begin))
				continue;

			if ((end != 0) && (end < timecode_ms))
				continue;

			if ((begin != 0) || (end != 0)) {
				buf = widget->prepare(is_update);

				if (buf != NULL) {
					// Image over
					buf->specmod().x = widget->x();
					buf->specmod().y = widget->y();
					OIIO::ImageBufAlgo::over(image_buffer, *buf, image_buffer, buf->roi());
				}
			}

			// Render dynamic widget
			buf = widget->render(data_, is_update);

			if (buf == NULL)
				continue;

			// Image over
			buf->specmod().x = widget->x();
			buf->specmod().y = widget->y();
			OIIO::ImageBufAlgo::over(image_buffer, *buf, image_buffer, buf->roi());
		}

		// Write image file
		if (out->open(filename, image_buffer.spec()) == false) {
			log_error("Draw track failure, can't open '%s' file", filename.c_str());
			goto error;
		}

		out->write_image(image_buffer.spec().format, image_buffer.localpixels());
		out->close();
	}

error:
//	// Max rendering duration
//	if (app_.settings().maxDuration() > 0) {
//		if (timecode_ms >= app_.settings().maxDuration())
//			goto done;
//	}

	// Dump frame info
	{
		char s[128];
		struct tm time;

		time_t now = ::time(NULL);

		localtime_r(&app_.time(), &time);

		strftime(s, sizeof(s), "%Y-%m-%d %H:%M:%S", &time);

		if (app_.progressInfo()) {
			printf("FRAME: %ld - TIMESTAMP: %ld ms - TIME: %s\n", 
				timecode_, timecode_ms, s);
		}
		else {
			int percent = 100 * timecode_ms / duration_ms_;
			int remaining = (timecode_ms > 0) ? (now - started_at_) * (duration_ms_ - timecode_ms) / timecode_ms : -1;

			printf("\r[FRAME %5ld] %02d:%02d:%02d.%03d / %s | %3d%% - Remaining time: %02d:%02d:%02d", 
				timecode_, 
				(int) (timecode_ms / 3600000), (int) ((timecode_ms / 60000) % 60), (int) ((timecode_ms / 1000) % 60), (int) (timecode_ms % 1000),
				duration_,
				percent,
				(remaining / 3600), (remaining / 60) % 60, (remaining) % 60
				); //label, buf, percent,
			fflush(stdout);
		}
	}

	// Dump telemetry data
	if (source_ && app_.progressInfo())
		data_.dump();

	timecode_++;

	schedule();

	return true;

done:
	complete();

	return true;
}


bool ImageRenderer::stop(void) {
	int working;

	time_t now = ::time(NULL);

	if (!app_.progressInfo())
		printf("\n");

	// Retrieve video streams
	VideoStreamPtr video_stream = container_->getVideoStream();

	// Sum-up
	working = now - started_at_;

	if (started_at_ > 0) 
		printf("%ld frames %dx%d proceed in %02d:%02d:%02d\n",
			timecode_,
			video_stream->width(), video_stream->height(),
			(working / 3600), (working / 60) % 60, (working) % 60);
	else
		printf("None frame proceed\n");

	if (overlay_)
		delete overlay_;

	overlay_ = NULL;

	return true;
}

