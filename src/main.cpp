#include <string.h>
#include <getopt.h>

#include <iostream>
#include <string>

#include "gpx.h"
#include "decoder.h"
#include "encoder.h"
#include "renderer.h"


namespace gpx2video {


static struct option options[] = {
	{ "help",      no_argument,       0, 'h' },
	{ "verbose",   no_argument,       0, 'v' },
	{ "quiet",     no_argument,       0, 'q' },
	{ "duration",  required_argument, 0, 'd' },
	{ "media",     required_argument, 0, 'm' },
	{ "gpx",       required_argument, 0, 'g' },
	{ "output",    required_argument, 0, 'o' },
	{ 0,           0,                 0, 0 }
};


static void printUsage(const std::string &name) {
	std::cout << "Usage: " << name << "%s [-v] -m=media -g=gpx -o=output" << std::endl;
	std::cout << "       " << name << " -h" << std::endl;
	std::cout << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "\t- m, --media=file       : Input media file name" << std::endl;
	std::cout << "\t- g, --gpx=file         : GPX file name" << std::endl;
	std::cout << "\t- o, --output=file      : Output file name" << std::endl;
	std::cout << "\t- d, --duration         : Duration (in ms)" << std::endl;
	std::cout << "\t- v, --verbose          : Show trace" << std::endl;
	std::cout << "\t- q, --quiet            : Quiet mode" << std::endl;
	std::cout << "\t- h, --help             : Show this help screen" << std::endl;
	std::cout << std::endl;

	return;
}


static void init(void) {
	av_register_all();
	avcodec_register_all();
}


namespace log {

static void setLevel(int level) {
	av_log_set_level(level);
}


static void quiet(bool enable) {
	(void) enable;
}

}; // namespace log


static void process(const std::string &mediafile, const std::string &gpxfile, 
	const std::string &outputfile,
	int max_duration_ms) {
	// Video utc start time
	time_t start_time;

	// GPX input file
	GPXData data;

	GPX *gpx = GPX::open(gpxfile);

	gpx->dump();

//	gpx->setStartTime(&start_time);
////	gpx->setStartTime(s);
//	data = gpx->retrieveData(0);
//	data.dump();


	// Probe input media
	MediaContainer *container = Decoder::probe(mediafile);
//	MediaContainer *container = Decoder::probe("../video/clip.mp4");
//	MediaContainer *container = Decoder::probe("../video/GOPR1860.MP4");

	// Set start time in GPX stream
	start_time = container->startTime();
	gpx->setStartTime(start_time);

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
	settings.setFilename(outputfile);
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

		// Read audio data
		frame = decoder_audio->retrieveAudio(audio_params, real_time);
		
		if (frame != NULL)
			encoder->writeAudio(frame, real_time);

		// Read video data
		frame = decoder_video->retrieveVideo(real_time);

		if (frame == NULL)
			break;

		timecode = frame->timestamp();
		timecode_ms = timecode * av_q2d(video_stream->timeBase()) * 1000;

		// Read GPX data
		data = gpx->retrieveData(timecode_ms);

		// Draw
		renderer->draw(frame, data);

		// Max 5 secondes
		if (timecode_ms > max_duration_ms)
			break;

		// Dump frame info
		char s[128];
		const time_t t = start_time + (timecode_ms / 1000);
		struct tm time;

		localtime_r(&t, &time);

		strftime(s, sizeof(s), "%Y-%m-%d %H:%M:%S", &time);

		printf("FRAME: %ld - PTS: %ld - TIMESTAMP: %ld ms - TIME: %s\n", 
			frame_time, timecode, timecode_ms, s);

		// Dump GPX data
		data.dump();

		real_time = av_mul_q(av_make_q(timecode, 1), video_stream->timeBase());

		encoder->writeFrame(frame, real_time);

		frame_time++;
	}

	encoder->close();
	decoder_audio->close();
	decoder_video->close();
}

}; // namespace gpx2video


int main(int argc, char *argv[], char *envp[]) {
	int index;
	int option;

	int verbose = 0;
	int max_duration_ms = 5 * 1000; // By default 5 seconds

	char *gpxfile = NULL;
	char *mediafile = NULL;
	char *outputfile = NULL;

	const std::string name(argv[0]);

	(void) envp;

	for (;;) {
		index = 0;
		option = getopt_long(argc, argv, "hqvd:m:g:o:", gpx2video::options, &index);

		if (option == -1) 
			break;

		switch (option) {
		case 0:
			printf("option %s", gpx2video::options[index].name);
			if (optarg)
				printf(" with arg %s", optarg);
			printf("\n");
			break;
		case 'h':
			gpx2video::printUsage(name);
			exit(0);
			break;
		case 'q':
			gpx2video::log::quiet(true);
			break;
		case 'v':
			verbose++;
			break;
		case 'd':
			max_duration_ms = atoi(optarg);
			break;
		case 'm':
			if (mediafile != NULL) {
				printf("'media' option is already set!\n");
				gpx2video::printUsage(name);
				exit(0);
			}
			mediafile = strdup(optarg);
			break;
		case 'g':
			if (gpxfile != NULL) {
				printf("'gpx' option is already set!\n");
				gpx2video::printUsage(name);
				exit(0);
			}
			gpxfile = strdup(optarg);
			break;
		case 'o':
			if (outputfile != NULL) {
				printf("'output' option is already set!\n");
				gpx2video::printUsage(name);
				exit(0);
			}
			outputfile = strdup(optarg);
			break;
		default:
			gpx2video::printUsage(name);
			exit(0);
			break;
		}
	}

	// getopt has consumed
	argc -= optind;
	argv += optind;
	optind = 0;

	// Check required options
	if (mediafile == NULL) {
		std::cout << name << ": option '--media' is required" << std::endl;
		gpx2video::printUsage(name);
		goto exit;
	}

	if (gpxfile == NULL) {
		std::cout << name << ": option '--gpx' is required" << std::endl;
		gpx2video::printUsage(name);
		goto exit;
	}

	if (outputfile == NULL) {
		std::cout << name << ": option '--output' is required" << std::endl;
		gpx2video::printUsage(name);
		goto exit;
	}

	av_log_set_level(AV_LOG_INFO);

	// Init
	gpx2video::init();

	// Logs
	gpx2video::log::setLevel(AV_LOG_INFO);

	// Process
	gpx2video::process(mediafile, gpxfile, outputfile, max_duration_ms);

exit:
	if (mediafile != NULL)
		free(mediafile);
	if (gpxfile != NULL)
		free(gpxfile);
	if (outputfile != NULL)
		free(outputfile);

	exit(EXIT_SUCCESS);
}
