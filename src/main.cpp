#include <iostream>
#include <string>

#include "decoder.h"
#include "encoder.h"
#include "renderer.h"


int main(int argc, char *argv[], char *envp[]) {
	(void) argc;
	(void) argv;
	(void) envp;

	av_log_set_level(AV_LOG_INFO);

	av_register_all();
	avcodec_register_all();


	MediaContainer *container = Decoder::probe("../video/clip.mp4");
//	MediaContainer *container = Decoder::probe("../video/GOPR1860.MP4");

	VideoStreamPtr video_stream = container->getVideoStream();
	AudioStreamPtr audio_stream = container->getAudioStream();

	// av_inv_q
	VideoParams video_params(video_stream->width(), video_stream->height(),
		// av_make_q(1,  50), 
		av_inv_q(video_stream->frameRate()),
		video_stream->format(),
		video_stream->nbChannels(), //VideoParams::InternalChannelCount,
		video_stream->pixelAspectRatio(),
		video_stream->interlacing());
//	AudioParams audio_params();

//	video_params.setTimeBase(video_stream->timeBase());
	video_params.setPixelFormat(video_stream->pixelFormat());

	EncoderSettings settings;
	settings.setFilename("video.mp4");
	settings.setVideoParams(video_params, AV_CODEC_ID_H264);
	settings.setVideoBitrate(4 * 1000 * 1000 * 8);
	settings.setVideoMaxBitrate(2 * 1000 * 1000 * 16);
	settings.setVideoBufferSize(4 * 1000 * 1000 / 2);
//	settings.setAudioParams(audio_params, AV_CODEC_ID_AAC);


//	Decoder *decoder;
//
//	foreach () {
//		decoder = Decoder::create();
//		decoder->open(video_stream);
//	}

	Decoder *decoder = Decoder::create();
	decoder->open(video_stream);

	Encoder *encoder = Encoder::create(settings);
	encoder->open();

	Renderer *renderer = Renderer::create();

	/**
	 * Choose fps from command line (or from input stream
	 * for each output frame, we can set easily the PTS value of output frame:
	 *  pts = timestamp (in second) / av_q2d(timebase of output video stream)
	 * We have to retrieve input frame in providing timestamp (in second).
	 * At last, we can extract telemetry data at timestamp (in second).
	 */
	int64_t frame_time = 0;

//	for (int i=0; i<10; i++) {
	for (;;) {
		AVRational real_time;

//		printf("FRAME_TIME: %ld\n", frame_time);

		real_time = av_mul_q(av_make_q(frame_time, 1), encoder->settings().videoParams().timeBase()); // timestamp_to_time(frame_time, time_base);

		FramePtr frame;

		frame = decoder->retrieveVideo(real_time);
//		frame = decoder->retrieveAudio(range);
		
		if (frame == NULL)
			break;

		renderer->draw(frame);

		int64_t timecode = frame->timestamp();

		printf("FRAME: %ld - PTS: %ld - TIMESTAMP: %ld ms\n", frame_time, timecode, 
			(int64_t) (timecode * av_q2d(video_stream->timeBase()) * 1000));

//		real_time = av_make_q(timecode, 1);
		real_time = av_mul_q(av_make_q(timecode, 1), video_stream->timeBase());

		encoder->writeFrame(frame, real_time);

		frame_time++;
	}

	encoder->close();
	decoder->close();

	exit(EXIT_SUCCESS);
}

