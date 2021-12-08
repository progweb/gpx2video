#include <iostream>
#include <memory>

#include <OpenImageIO/imageio.h>
#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>

#include "oiioutils.h"
#include "decoder.h"
#include "audioparams.h"
#include "videoparams.h"
#include "encoder.h"
#include "renderer.h"


Renderer::Renderer(GPX2Video &app, Map *map) 
	: Task(app) 
	, app_(app) 
	, map_(map) {
	container_ = NULL;
	decoder_audio_ = NULL;
	decoder_video_ = NULL;
	encoder_ = NULL;

	frame_time_ = 0;
}


Renderer::~Renderer() {
	if (encoder_)
		delete encoder_;
	if (decoder_audio_)
		delete decoder_audio_;
	if (decoder_video_)
		delete decoder_video_;
}


Renderer * Renderer::create(GPX2Video &app, Map *map) {
	Renderer *renderer = new Renderer(app, map);

	renderer->init();

	return renderer;
}


void Renderer::init(void) {
	time_t start_time;

	std::string mediafile = app_.settings().mediafile();

	gpx_ = GPX::open(app_.settings().gpxfile());

	// Probe input media
	container_ = Decoder::probe(mediafile);

	// Set start time in GPX stream
	start_time = container_->startTime();
	gpx_->setStartTime(start_time);

	// Retrieve audio & video streams
	VideoStreamPtr video_stream = container_->getVideoStream();
	AudioStreamPtr audio_stream = container_->getAudioStream();

	// Audio & Video encoder settings
	VideoParams video_params(video_stream->width(), video_stream->height(),
		// av_make_q(1,  50), 
		av_inv_q(video_stream->frameRate()),
		video_stream->format(),
		video_stream->nbChannels(),
		video_stream->pixelAspectRatio(),
		video_stream->interlacing());
	AudioParams audio_params(audio_stream->sampleRate(),
		audio_stream->channelLayout(),
		audio_stream->format());

	video_params.setPixelFormat(video_stream->pixelFormat());

	EncoderSettings settings;
	settings.setFilename(app_.settings().outputfile());
	settings.setVideoParams(video_params, AV_CODEC_ID_H264);
	settings.setVideoBitrate(4 * 1000 * 1000 * 8);
	settings.setVideoMaxBitrate(2 * 1000 * 1000 * 16);
	settings.setVideoBufferSize(4 * 1000 * 1000 / 2);
	settings.setAudioParams(audio_params, AV_CODEC_ID_AAC);
	settings.setAudioBitrate(44 * 1000);

	// Open & decode input media
	decoder_video_ = Decoder::create();
	decoder_video_->open(video_stream);

	decoder_audio_ = Decoder::create();
	decoder_audio_->open(audio_stream);

	// Open & encode output video
	encoder_ = Encoder::create(settings);
	encoder_->open();
}


void Renderer::run(void) {
	GPXData data;

	FramePtr frame;

	time_t start_time;

	int64_t timecode;
	int64_t timecode_ms;

	AVRational real_time;

	VideoStreamPtr video_stream = container_->getVideoStream();
//	AudioStreamPtr audio_stream = container_->getAudioStream();

	start_time = container_->startTime();

	real_time = av_mul_q(av_make_q(frame_time_, 1), encoder_->settings().videoParams().timeBase());

	// Read audio data
	frame = decoder_audio_->retrieveAudio(encoder_->settings().audioParams(), real_time);

	if (frame != NULL)
		encoder_->writeAudio(frame, real_time);

	// Read video data
	frame = decoder_video_->retrieveVideo(real_time);

	if (frame == NULL)
		goto done;

	timecode = frame->timestamp();
	timecode_ms = timecode * av_q2d(video_stream->timeBase()) * 1000;

	// Read GPX data
	data = gpx_->retrieveData(timecode_ms);

	// Draw
	this->draw(frame, data);

	// Max 5 secondes
	if (app_.settings().maxDuration() > 0) {
		if (timecode_ms > app_.settings().maxDuration())
			goto done;
	}

	// Dump frame info
	{
		char s[128];
		const time_t t = start_time + (timecode_ms / 1000);
		struct tm time;

		localtime_r(&t, &time);

		strftime(s, sizeof(s), "%Y-%m-%d %H:%M:%S", &time);

		printf("FRAME: %ld - PTS: %ld - TIMESTAMP: %ld ms - TIME: %s\n", 
			frame_time_, timecode, timecode_ms, s);
	}

	// Dump GPX data
	data.dump();

	real_time = av_mul_q(av_make_q(timecode, 1), video_stream->timeBase());

	encoder_->writeFrame(frame, real_time);

	frame_time_++;

	schedule();

	return;

done:
	encoder_->close();
	decoder_audio_->close();
	decoder_video_->close();

	complete();
}


void Renderer::draw(FramePtr frame, const GPXData &data) {
	char s[128];

	OIIO::ImageBuf frame_buffer = frame->toImageBuf();

	// Draw gauges
	sprintf(s, "%.0f%%", data.grade());
	this->add(&frame_buffer, 50, 500, "./assets/picto/DataOverlay_icn_grade.png", "PENTE", s, 2.5 * 64.0 / 150.0);

	sprintf(s, "%.0f m", data.elevation());
	this->add(&frame_buffer, 50, 700, "./assets/picto/DataOverlay_icn_elevation.png", "ALTITUDE", s, 2.5);

	sprintf(s, "%.0f km/h", data.speed());
	this->add(&frame_buffer, 50, 900, "./assets/picto/DataOverlay_icn_speed.png", "VITESSE", s, 2.5);

	// Draw map
	if (map_ != NULL)
		map_->render(&frame_buffer, data); // x:1700, y:900, w:800, h:500

	frame->fromImageBuf(frame_buffer);
}


void Renderer::add(OIIO::ImageBuf *frame, int x, int y, const char *picto, const char *label, const char *value, double divider) {
	int w, h;

	// Open picto
	auto img = OIIO::ImageInput::open(picto);
	const OIIO::ImageSpec& spec = img->spec();
	VideoParams::Format img_fmt = OIIOUtils::getFormatFromOIIOBaseType((OIIO::TypeDesc::BASETYPE) spec.format.basetype);
	OIIO::TypeDesc::BASETYPE type = OIIOUtils::getOIIOBaseTypeFromFormat(img_fmt);

	OIIO::ImageBuf *buf = new OIIO::ImageBuf(OIIO::ImageSpec(spec.width, spec.height, spec.nchannels, type)); //, OIIO::InitializePixels::No);
	img->read_image(type, buf->localpixels());

	// Resize picto
	OIIO::ImageBuf dst(OIIO::ImageSpec(spec.width * divider, spec.height * divider, spec.nchannels, type)); //, OIIO::InitializePixels::No);
	OIIO::ImageBufAlgo::resize(dst, *buf);

//	x = 50;
//	y = 900;
	w = spec.width * divider;
	h = spec.height * divider;

	// Image over
	dst.specmod().x = x;
	dst.specmod().y = y;
	OIIO::ImageBufAlgo::over(*frame, dst, *frame, OIIO::ROI());

	delete buf;


	// Add label
	int pt;
	int space = h / 10;
	pt = (h - 3 * space) / 3;
	float white[] = { 1.0, 1.0, 1.0, 1.0 };
	if (OIIO::ImageBufAlgo::render_text(*frame, x + w + (w/10), y + space + pt, label, pt, "./assets/fonts/Helvetica.ttf", white) == false)
		fprintf(stderr, "render text error\n");
	pt *= 2;
	if (OIIO::ImageBufAlgo::render_text(*frame, x + w + (w/10), y + h - (pt/2), value, pt, "./assets/fonts/Helvetica.ttf", white) == false)
		fprintf(stderr, "render text error\n");
}


//void Renderer::drawMap(OIIO::ImageBuf *frame, int x, int y, int width, int height, double divider) {
//	int w, h;
//
//	// Open map
//	auto img = OIIO::ImageInput::open("map.png");
//	const OIIO::ImageSpec& spec = img->spec();
//	VideoParams::Format img_fmt = OIIOUtils::getFormatFromOIIOBaseType((OIIO::TypeDesc::BASETYPE) spec.format.basetype);
//	OIIO::TypeDesc::BASETYPE type = OIIOUtils::getOIIOBaseTypeFromFormat(img_fmt);
//
//	OIIO::ImageBuf *buf = new OIIO::ImageBuf(OIIO::ImageSpec(spec.width, spec.height, spec.nchannels, type)); //, OIIO::InitializePixels::No);
//	img->read_image(type, buf->localpixels());
//
//	// Resize map
//	OIIO::ImageBuf dst(OIIO::ImageSpec(spec.width * divider, spec.height * divider, spec.nchannels, type)); //, OIIO::InitializePixels::No);
//	OIIO::ImageBufAlgo::resize(dst, *buf);
//
//	// Draw track
//
//	// Image over
//	dst.specmod().x = x;
//	dst.specmod().y = y;
//	OIIO::ImageBufAlgo::over(*frame, dst, *frame, OIIO::ROI(x, x + width, y, y + height));
//
//	delete buf;
//}

