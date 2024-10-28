#include <iostream>
#include <memory>

#include <OpenImageIO/imageio.h>
#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>

#include "oiioutils.h"
#include "ffmpegutils.h"
#include "videorenderer.h"


VideoRenderer::VideoRenderer(GPXApplication &app, 
		RendererSettings &renderer_settings, TelemetrySettings &telemetry_settings)
	: Renderer(app, renderer_settings, telemetry_settings)
	, started_at_(0) {
	decoder_audio_ = NULL;
	decoder_video_ = NULL;
	decoder_gpmf_ = NULL;
	encoder_ = NULL;

	frame_time_ = 0;
	duration_ms_ = 0;
	real_duration_ms_ = 0;
}


VideoRenderer::~VideoRenderer() {
	if (encoder_)
		delete encoder_;
	if (decoder_audio_)
		delete decoder_audio_;
	if (decoder_video_)
		delete decoder_video_;
	if (decoder_gpmf_)
		delete decoder_gpmf_;
}


VideoRenderer * VideoRenderer::create(GPXApplication &app, 
		RendererSettings &renderer_settings, TelemetrySettings &telemetry_settings,
		MediaContainer *container) {
	VideoRenderer *renderer = new VideoRenderer(app, renderer_settings, telemetry_settings);

	if (renderer->init(container) == false)
		goto abort;

	renderer->load();
	renderer->computeWidgetsPosition();

	return renderer;

abort:
	delete renderer;

	return NULL;
}


bool VideoRenderer::init(MediaContainer *container) {
	if (!Renderer::init(container))
		return false;

	// Codec
	ExportCodec::Codec video_codec = rendererSettings().videoCodec();

	// Retrieve audio & video streams
	VideoStreamPtr video_stream = container_->getVideoStream();
	AudioStreamPtr audio_stream = container_->getAudioStream();

	// Retrieve GoPro MET stream
	StreamPtr gpmf_stream = container_->getDataStream("GoPro MET");

	// Audio & Video encoder settings
	VideoParams video_params(video_stream->width(), video_stream->height(),
		av_inv_q(video_stream->frameRate()),
		video_stream->format(),
		video_stream->nbChannels(),
		video_stream->orientation(),
		video_stream->pixelAspectRatio(),
		video_stream->interlacing());

	// Set video output framerate (fps)
	//video_params.setFrameRate(av_make_q(30, 1));

	// By default, use pixel format from input video execpt if deprecated
	video_params.setPixelFormat(FFmpegUtils::overrideFFmpegDeprecatedPixelFormat(video_stream->pixelFormat()));

	// Use pixel format supported by encoder
	switch (video_codec) {
	case ExportCodec::CodecVAAPIH264:
	case ExportCodec::CodecQSVH264:
	case ExportCodec::CodecQSVHEVC:
		video_params.setPixelFormat(AV_PIX_FMT_NV12);
		break;

	default:
		break;
	}

	// Encoder settings
	EncoderSettings encoderSettings;
	encoderSettings.setFilename(app_.settings().outputfile());
	encoderSettings.setVideoParams(video_params, video_codec);
	encoderSettings.setVideoHardwareDevice(rendererSettings().videoHardwareDevice());

	switch (video_codec) {
	case ExportCodec::CodecH264:
	case ExportCodec::CodecHEVC:
		if (rendererSettings().videoCRF() != -1) {
			// Set crf value
			encoderSettings.setVideoOption("crf", std::to_string(rendererSettings().videoCRF()));
		}
		else {
			// Disable crf as target bitrate compression method is used
			encoderSettings.setVideoOption("crf", "-1");

			// Set target bitrate
			encoderSettings.setVideoBitrate(rendererSettings().videoBitrate()); // 2 * 1000 * 1000 * 8 // 16
			encoderSettings.setVideoMinBitrate(rendererSettings().videoMinBitrate()); // 0 // 8 * 1000 * 1000
			encoderSettings.setVideoMaxBitrate(rendererSettings().videoMaxBitrate()); // 2 * 1000 * 1000 * 16 // 32
			encoderSettings.setVideoBufferSize(4 * 1000 * 1000 / 2);
		}

		// Preset: ultrafast, fast, medium...
		if (!rendererSettings().videoPreset().empty())
			encoderSettings.setVideoOption("preset", rendererSettings().videoPreset());

		break;

	case ExportCodec::CodecNVEncH264:
	case ExportCodec::CodecNVEncHEVC:
		encoderSettings.setVideoOption("rc", "vbr");
		encoderSettings.setVideoOption("cq", "19");
		encoderSettings.setVideoOption("profile", "main");
		encoderSettings.setVideoOption("level", "auto");

		// Preset: ultrafast, fast, medium...
		if (!rendererSettings().videoPreset().empty())
			encoderSettings.setVideoOption("preset", rendererSettings().videoPreset());
		break;

	case ExportCodec::CodecQSVH264:
	case ExportCodec::CodecQSVHEVC:
		// Preset: ultrafast, fast, medium...
		if (!rendererSettings().videoPreset().empty())
			encoderSettings.setVideoOption("preset", rendererSettings().videoPreset());

		break;

	default:
		break;
	}

	if (audio_stream) {
		AudioParams audio_params(audio_stream->sampleRate(),
			audio_stream->channelLayout(),
			audio_stream->format());

		encoderSettings.setAudioParams(audio_params, ExportCodec::CodecAAC);
		encoderSettings.setAudioBitrate(44 * 1000);
	}

	// Compute layout size from width & height and DAR
	//   DAR = width / height * SAR
	//   SAR = video_stream->pixelAspectRatio()
	switch (video_params.orientation()) {
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

	// If maxDuration set by the user
	if (app_.settings().maxDuration() > 0) 
		duration_ms_ = MIN(duration_ms_, app_.settings().maxDuration());

	snprintf(duration_, sizeof(duration_), "%02d:%02d:%02d.%03d", 
		(unsigned int) (duration_ms_ / 3600000), (unsigned int) ((duration_ms_ / 60000) % 60), (unsigned int) ((duration_ms_ / 1000) % 60), (unsigned int) (duration_ms_ % 1000));
	duration_[sizeof(duration_) - 1] = '\0';

	// Open & decode input media
	decoder_video_ = Decoder::create();
	decoder_video_->open(video_stream);

	if (audio_stream) {
		decoder_audio_ = Decoder::create();
		decoder_audio_->open(audio_stream);
	}

	// Open & decoder gpmf stream
	if (gpmf_stream) {
		decoder_gpmf_ = GPMFDecoder::create();
		decoder_gpmf_->open(gpmf_stream);
	}

	// Open & encode output video
	encoder_ = Encoder::create(encoderSettings);
	return encoder_->open();
}


void VideoRenderer::computeWidgetsPosition(void) {
	int x, y;

	int orientation = encoder_->settings().videoParams().orientation();

	double sar = av_q2d(encoder_->settings().videoParams().pixelAspectRatio());

	log_call();

	Renderer::computeWidgetsPosition();

	// At last, apply layout rotation
	for (VideoWidget *widget : widgets_) {
		switch (orientation) {
		case 180:
		case -180:
			x = layout_width_ - widget->x() - widget->width();
			y = layout_height_ - widget->y() - widget->height();
			break;

		case 90:
		case -270:
			x = widget->y();
			y = layout_width_ - widget->x() - widget->width();
			break;

		case -90:
		case 270:
			x = layout_height_ - widget->y() - widget->height();
			y = widget->x();
			break;

		default:
			x = widget->x();
			y = widget->y();
			break;
		} 

		x = round((double) x / sar);

		widget->setPosition(x, y);
	}
}


bool VideoRenderer::start(void) {
	bool is_update = false;

	time_t now = time(NULL);

//	time_t start_time;

	double sar;
	int orientation;

	VideoStreamPtr video_stream = container_->getVideoStream();

	log_call();

	log_notice("Rendering...");

	// SAR & orientation video
	sar = av_q2d(encoder_->settings().videoParams().pixelAspectRatio());
	orientation = encoder_->settings().videoParams().orientation();

//	// Compute start time
//	start_time = container_->startTime() + container_->timeOffset();
//
//	app_.setTime(start_time);
//
//	// Update start time in GPX stream (start_time can change after sync step)
//	if (gpx_) {
//		gpx_->setStartTime(start_time);
////		data_.init();
//	}

	started_at_ = now;

	// Create overlay buffer
	overlay_ = new OIIO::ImageBuf(OIIO::ImageSpec(video_stream->width(), video_stream->height(), 
		video_stream->nbChannels(), OIIOUtils::getOIIOBaseTypeFromFormat(video_stream->format())));

	// Prepare each widget, map...
	for (VideoWidget *widget : widgets_) {
		OIIO::ImageBuf *buf = NULL;

		uint64_t begin = widget->atBeginTime();
		uint64_t end = widget->atEndTime();

		if ((begin != 0) || (end != 0))
			continue;

		// Render static widget
		buf = widget->prepare(is_update);

		if (buf == NULL)
			continue;

		// Rotate & rescale
		this->resize(buf, round((double) widget->width() / sar), widget->height());
		this->rotate(buf, orientation);

		// Image over
		buf->specmod().x = widget->x();
		buf->specmod().y = widget->y();
		OIIO::ImageBufAlgo::over(*overlay_, *buf, *overlay_, buf->roi());
	}

	return true;
}


bool VideoRenderer::run(void) {
	FramePtr frame;

	double sar;
	int orientation;

	time_t start_time;

	int64_t timecode;
	uint64_t timecode_ms;

	double time_factor;

	int duration;
	unsigned int real_duration_ms;

	AVRational video_time;

	bool is_update = false;

	VideoStreamPtr video_stream = container_->getVideoStream();
//	AudioStreamPtr audio_stream = container_->getAudioStream();

	time_factor = rendererSettings().timeFactor();

	start_time = container_->startTime() + container_->timeOffset();

	video_time = av_div_q(av_make_q(1000 * frame_time_, 1), encoder_->settings().videoParams().frameRate());

	// SAR & orientation video
	sar = av_q2d(encoder_->settings().videoParams().pixelAspectRatio());
	orientation = encoder_->settings().videoParams().orientation();

	// Read GPMF data
	if (decoder_gpmf_) {
		decoder_gpmf_->retrieveData(gpmf_data_, video_time);

		if (rendererSettings().isTimeFactorAuto())
			time_factor = gpmf_data_.timelapse;
	}
	
	// Read audio data
	if (decoder_audio_) {
		duration = round(av_q2d(av_div_q(av_make_q(1000 * (frame_time_ + 1), 1), encoder_->settings().videoParams().frameRate())));
		duration -= round(av_q2d(video_time));

		do {
			frame = decoder_audio_->retrieveAudio(encoder_->settings().audioParams(), video_time, duration);

			if (frame != NULL)
				encoder_->writeAudio(frame, video_time);
		} while (frame != NULL);
	}

	// Read video data
	frame = decoder_video_->retrieveVideo(video_time);

	if (frame == NULL)
		goto done;

	timecode = frame->timestamp();
	timecode_ms = timecode * av_q2d(video_stream->timeBase()) * 1000;

	// Update video real time 
	app_.setTime(start_time + real_duration_ms_ / 1000);

	if (source_) {
		uint64_t timestamp = (start_time * 1000) + real_duration_ms_;

		OIIO::ImageBuf frame_buffer = frame->toImageBuf();

		// Read GPX data
		timestamp -= (timestamp % telemetrySettings().telemetryRate());
		source_->retrieveNext(data_, timestamp);

		// Draw overlay
		OIIO::ImageBufAlgo::over(frame_buffer, *overlay_, frame_buffer, OIIO::ROI());

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
					// Rotate & resize
					if (is_update) {
						this->resize(buf, round((double) widget->width() / sar), widget->height());
						this->rotate(buf, orientation);
					}

					// Image over
					buf->specmod().x = widget->x();
					buf->specmod().y = widget->y();
					OIIO::ImageBufAlgo::over(frame_buffer, *buf, frame_buffer, buf->roi());
				}
			}

			// Render dynamic widget
			buf = widget->render(data_, is_update);

			if (buf == NULL)
				continue;

			// Rotate & resize
			if (is_update) {
				this->resize(buf, round((double) widget->width() / sar), widget->height());
				this->rotate(buf, orientation);
			}

			// Image over
			buf->specmod().x = widget->x();
			buf->specmod().y = widget->y();
			OIIO::ImageBufAlgo::over(frame_buffer, *buf, frame_buffer, buf->roi());
		}

		frame->fromImageBuf(frame_buffer);
	}

	// Max rendering duration
	if (app_.settings().maxDuration() > 0) {
		if (timecode_ms > app_.settings().maxDuration())
			goto done;
	}

	// Dump frame info
	{
		char s[128];
		struct tm time;

		time_t now = ::time(NULL);

		localtime_r(&app_.time(), &time);

		strftime(s, sizeof(s), "%Y-%m-%d %H:%M:%S", &time);

		if (app_.progressInfo()) {
			printf("FRAME: %ld - PTS: %ld - TIMESTAMP: %ld ms - TIME: %s (x %.01f)\n", 
				frame_time_, timecode, timecode_ms, s, time_factor);
		}
		else {
			int percent = 100 * timecode_ms / duration_ms_;
			int remaining = (timecode_ms > 0) ? (now - started_at_) * (duration_ms_ - timecode_ms) / timecode_ms : -1;

			printf("\r[FRAME %5ld] %02d:%02d:%02d.%03d / %s | %3d%% - Remaining time: %02d:%02d:%02d", 
				frame_time_, 
				(int) (timecode_ms / 3600000), (int) ((timecode_ms / 60000) % 60), (int) ((timecode_ms / 1000) % 60), (int) (timecode_ms % 1000),
				duration_,
				percent,
				(remaining / 3600), (remaining / 60) % 60, (remaining) % 60
				); //label, buf, percent,
			fflush(stdout);
		}
	}

	// Compute real time by step, since time_factor is variable
	real_duration_ms = round(av_q2d(av_div_q(av_make_q(1000, 1), encoder_->settings().videoParams().frameRate())));
	real_duration_ms_ += time_factor * real_duration_ms;

	// Dump GPMF data
	if (decoder_gpmf_ && app_.progressInfo())
		gpmf_data_.dump();

	// Dump telemetry data
	if (source_ && app_.progressInfo())
		data_.dump();

	video_time = av_mul_q(av_make_q(timecode, 1), video_stream->timeBase());

	encoder_->writeFrame(frame, video_time);

	frame_time_++;


	schedule();

	return true;

done:
	complete();

	return true;
}


bool VideoRenderer::stop(void) {
	int working;

	time_t now = ::time(NULL);

	if (!app_.progressInfo())
		printf("\n");

	// Retrieve video streams
	VideoStreamPtr video_stream = container_->getVideoStream();

	// Sum-up
	working = now - started_at_;

	if (started_at_ > 0) 
		printf("%ld frames %dx%d to %dx%d proceed in %02d:%02d:%02d\n",
			frame_time_,
			video_stream->width(), video_stream->height(),
			encoder_->settings().videoParams().width(), encoder_->settings().videoParams().height(),
			(working / 3600), (working / 60) % 60, (working) % 60);
	else
		printf("None frame proceed\n");

	encoder_->close();
	if (decoder_audio_)
		decoder_audio_->close();
	decoder_video_->close();

	if (overlay_)
		delete overlay_;

	decoder_audio_ = NULL;
	decoder_video_ = NULL;
	overlay_ = NULL;

	return true;
}


