#include <iostream>
#include <string>

#include <string.h>

//#include "gpx.h"
#include "decoder.h"
#include "encoder.h"
#include "renderer.h"


void init(void) {
	av_register_all();
	avcodec_register_all();
}


int main(int argc, char *argv[], char *envp[]) {
	// Video start time
	time_t start_time;

	(void) argc;
	(void) argv;
	(void) envp;

	av_log_set_level(AV_LOG_INFO);

	init();

//	// GPX input file
//	GPXData data;
//
//	GPX *gpx = GPX::open(argv[2]);
//
//	gpx->dump();
//
////	gpx->setStartTime(&start_time);
//////	gpx->setStartTime(s);
////	data = gpx->retrieveData(0);
////	data.dump();


	// Probe input media
	MediaContainer *container = Decoder::probe(argv[1]);
//	MediaContainer *container = Decoder::probe("../video/clip.mp4");
//	MediaContainer *container = Decoder::probe("../video/GOPR1860.MP4");

	// Set start time in GPX stream
	start_time = container->startTime();
//	gpx->setStartTime(start_time);

	// Retrieve audio & video streams
	VideoStreamPtr video_stream = container->getVideoStream();
	AudioStreamPtr audio_stream = container->getAudioStream();

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

//	video_params.setTimeBase(video_stream->timeBase());
	video_params.setPixelFormat(video_stream->pixelFormat());

	EncoderSettings settings;
	settings.setFilename("video.mp4");
	settings.setVideoParams(video_params, AV_CODEC_ID_H264);
	settings.setVideoBitrate(4 * 1000 * 1000 * 8);
	settings.setVideoMaxBitrate(2 * 1000 * 1000 * 16);
	settings.setVideoBufferSize(4 * 1000 * 1000 / 2);
	settings.setAudioParams(audio_params, AV_CODEC_ID_AAC);
	settings.setAudioBitrate(44 * 1000);

	// Open & decode input media
	Decoder *decoder_video = Decoder::create();
	decoder_video->open(video_stream);

	Decoder *decoder_audio = Decoder::create();
	decoder_audio->open(audio_stream);

	// Open & encode output video
	Encoder *encoder = Encoder::create(settings);
	encoder->open();

	// Renderer
	Renderer *renderer = Renderer::create();

	/**
	 * Choose fps from command line (or from input stream
	 * for each output frame, we can set easily the PTS value of output frame:
	 *  pts = timestamp (in second) / av_q2d(timebase of output video stream)
	 * We have to retrieve input frame in providing timestamp (in second).
	 * At last, we can extract telemetry data at timestamp (in second).
	 */
	int64_t timecode;
	int64_t timecode_ms;
	int64_t frame_time = 0;

	for (;;) {
		FramePtr frame;

		AVRational real_time;

		real_time = av_mul_q(av_make_q(frame_time, 1), encoder->settings().videoParams().timeBase()); // timestamp_to_time(frame_time, time_base);

		// Audio
		frame = decoder_audio->retrieveAudio(audio_params, real_time);
		
		if (frame != NULL)
			encoder->writeAudio(frame, real_time);

		// Video
		frame = decoder_video->retrieveVideo(real_time);

		if (frame == NULL)
			break;

		renderer->draw(frame);

		timecode = frame->timestamp();
		timecode_ms = timecode * av_q2d(video_stream->timeBase()) * 1000;

		// Max 5 secondes
		if (timecode_ms > 5000)
			break;

		char s[128];
		const time_t t = start_time + (timecode_ms / 1000);
		struct tm time;

		localtime_r(&t, &time);

		strftime(s, sizeof(s), "%Y-%m-%d %H:%M:%S", &time);

		printf("FRAME: %ld - PTS: %ld - TIMESTAMP: %ld ms - TIME: %s\n", 
			frame_time, timecode, timecode_ms, s);

//		data = gpx->retrieveData(timecode_ms);
//		data.dump();

		real_time = av_mul_q(av_make_q(timecode, 1), video_stream->timeBase());

		encoder->writeFrame(frame, real_time);

		frame_time++;
	}

	encoder->close();
	decoder_audio->close();
	decoder_video->close();

	exit(EXIT_SUCCESS);
}

