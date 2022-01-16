#include <iostream>
#include <memory>
#include <string>
#include <algorithm>

#include "log.h"
#include "ffmpegutils.h"
#include "decoder.h"


Decoder::Decoder()
	: fmt_ctx_(NULL)
	, codec_ctx_(NULL)
	, sws_ctx_(NULL) {
}


Decoder::~Decoder() {
	close();
}


MediaContainer * Decoder::probe(const std::string &filename) {
	int result;
	std::string name;
	std::string start_time;

	unsigned int i;

	AVFormatContext *fmt_ctx = NULL;

	MediaContainer *container;

	// Open file in a format context
	if ((result = avformat_open_input(&fmt_ctx, filename.c_str(), NULL, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
        return NULL;
    }
	
	// Read metadata
	// creation_time = 2020-12-13T09:56:27.000000Z
	const AVDictionaryEntry *entry = av_dict_get(fmt_ctx->metadata, "creation_time", NULL, 0);
	
	if ((entry != NULL) && (entry->value != NULL)) {
		av_log(NULL, AV_LOG_INFO, "%s = %s\n", entry->key, entry->value);
		
		start_time = entry->value;
	}

	// Get stream information from format
    if ((result = avformat_find_stream_info(fmt_ctx, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
        return NULL;
    }

	container = new MediaContainer();

	container->setStartTime(start_time);
	container->setFilename(filename);

	// For each stream
	for (i=0; i<fmt_ctx->nb_streams; i++) {
		StreamPtr stream;

		// Get reference to correct AVStream
		AVStream *avstream = fmt_ctx->streams[i];
		
		// Get stream name
		const AVDictionaryEntry *entry = av_dict_get(avstream->metadata, "handler_name", NULL, 0);
	
		if ((entry != NULL) && (entry->value != NULL))
			name = entry->value;
		else
			name = "";

		name.erase(std::remove(name.begin(), name.end(), 0xb), name.end());

		// Find decoder for this stream
		const AVCodec *decoder = avcodec_find_decoder(avstream->codecpar->codec_id);

		if (decoder
			&& ((avstream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
				|| (avstream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO))) {
			if (avstream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
				AVRational frame_rate;
				AVRational pixel_aspect_ratio;
				VideoParams::Interlacing interlacing = VideoParams::InterlaceNone;

				// Read at least two frames to get more information about this video stream
				{
					AVPacket *packet = av_packet_alloc();
					AVFrame *frame = av_frame_alloc();

					Decoder instance;
					instance.open(filename, avstream->index);

					// Read fist frame and retrieve some metadata
					if (instance.getFrame(packet, frame) >= 0) {
						// Check if video is interlaced and what field dominance it has if so
						if (frame->interlaced_frame) {
							if (frame->top_field_first)
								interlacing = VideoParams::InterlacedTopFirst;
							else
								interlacing = VideoParams::InterlacedBottomFirst;
						}

						pixel_aspect_ratio = av_guess_sample_aspect_ratio(fmt_ctx, avstream, frame);

						frame_rate = av_guess_frame_rate(fmt_ctx, avstream, frame);
					}

					// Read second frame
					// TODO

					instance.close();

					av_frame_free(&frame);
					av_packet_free(&packet);
				}

				AVPixelFormat compatible_pix_fmt = FFmpegUtils::getCompatiblePixelFormat(static_cast<AVPixelFormat>(avstream->codecpar->format));

				VideoStreamPtr video_stream = std::make_shared<VideoStream>();

				video_stream->setWidth(avstream->codecpar->width);
				video_stream->setHeight(avstream->codecpar->height);
				video_stream->setFrameRate(frame_rate);
				// video_stream->setStartTime(avstream->start_stime);
				video_stream->setInterlacing(interlacing);
				video_stream->setFormat(getNativePixelFormat(compatible_pix_fmt));
				video_stream->setPixelFormat(static_cast<AVPixelFormat>(avstream->codecpar->format));
				video_stream->setPixelAspectRatio(pixel_aspect_ratio);
				video_stream->setNbChannels(getNativeNbChannels(compatible_pix_fmt));

				stream = video_stream;
			}
			else if (avstream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
				AudioStreamPtr audio_stream = std::make_shared<AudioStream>();

				uint64_t channel_layout = avstream->codecpar->channel_layout;
				if (!channel_layout)
					channel_layout = static_cast<uint64_t>(av_get_default_channel_layout(avstream->codecpar->channels));

				audio_stream->setChannelLayout(channel_layout);
				audio_stream->setNbChannels(avstream->codecpar->channels);
				audio_stream->setSampleRate(avstream->codecpar->sample_rate);

				stream = audio_stream;
			}
		}
		else {
            av_log(NULL, AV_LOG_ERROR, "Failed to find decoder for stream #%u\n", i);
			
			stream = std::make_shared<Stream>();

			stream->setType(avstream->codecpar->codec_type);
		}

		stream->setName(name);
		stream->setIndex(avstream->index);
		stream->setTimeBase(avstream->time_base);
		stream->setDuration(avstream->duration);

		container->addStream(stream);
	}

	// Dump input info
    av_dump_format(fmt_ctx, 0, filename.c_str(), 0);

	// Close input
	avformat_close_input(&fmt_ctx);

	return container;
}


Decoder * Decoder::create(void) {
	Decoder *decoder = new Decoder();

	return decoder;
}


bool Decoder::open(StreamPtr stream) {
	bool result;

	// Set stream
	stream_ = stream;

	// Try to open
	if ((result = open(stream->container()->filename(), stream->index())) == false)
		return false;

	if (stream->type() == AVMEDIA_TYPE_VIDEO) {
		// Get a compatible AVPixelFormat
		ideal_pix_fmt_ = FFmpegUtils::getCompatiblePixelFormat(static_cast<AVPixelFormat>(avstream_->codecpar->format));

		// Determine which native pixel 
		native_pix_fmt_ = getNativePixelFormat(ideal_pix_fmt_);
		native_nb_channels_ = getNativeNbChannels(ideal_pix_fmt_);

		if ((native_pix_fmt_ == VideoParams::FormatInvalid)
			|| (native_nb_channels_ == 0)) {
			av_log(NULL, AV_LOG_ERROR, "Failed to find valid native pixel format for %d\n", ideal_pix_fmt_);
		}

		// Init scaler
		sws_ctx_ = sws_getContext(avstream_->codecpar->width, avstream_->codecpar->height, 
				static_cast<AVPixelFormat>(avstream_->codecpar->format),
				avstream_->codecpar->width, avstream_->codecpar->height, 
				ideal_pix_fmt_,
				SWS_FAST_BILINEAR, NULL, NULL, NULL);

		if (sws_ctx_ == NULL) {
			log_error("Decoder fails to create scale context");
			return false;
		}
	}

	return true;
}


bool Decoder::open(const std::string &filename, const int &index) {
	int result;

	if ((result = avformat_open_input(&fmt_ctx_, filename.c_str(), NULL, NULL)) < 0) {
		av_log(NULL, AV_LOG_ERROR, "Cannot open input file '%s'", filename.c_str());
		return false;
	}

	// Get stream information from format
	if ((result = avformat_find_stream_info(fmt_ctx_, NULL)) < 0) {
		av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
		return false;
	}

	// Get reference to correct AVStream
	avstream_ = fmt_ctx_->streams[index];

	// Find decoder
	AVCodec *decoder = avcodec_find_decoder(avstream_->codecpar->codec_id);

	// Handle failure to find decoder
	if (decoder == NULL) {
		av_log(NULL, AV_LOG_ERROR, "Failed to find appropriate decoder for this codec: %d",
			avstream_->codecpar->codec_id);
		return false;
	}

	// Allocate context for the decoder
	codec_ctx_ = avcodec_alloc_context3(decoder);
	if (!codec_ctx_) {
		av_log(NULL, AV_LOG_ERROR, "Failed to allocate the decoder context for stream #%u\n", index);
		return false;
	}

	// Copy parameters from the AVStream to the AVCodecContext
	if ((result = avcodec_parameters_to_context(codec_ctx_, avstream_->codecpar)) < 0) {
		av_log(NULL, AV_LOG_ERROR, "Failed to copy decoder parameters to input decoder context"
				"for stream #%u\n", index);
		return false;
	}

	// Open decoder
	result = avcodec_open2(codec_ctx_, decoder, NULL);

	if (result < 0) {
		char buf[64];
		av_strerror(result, buf, sizeof(buf));
		av_log(NULL, AV_LOG_ERROR, "Failed to open decoder for stream #%u, %s\n", index, buf);
		return false;
	}

	return true;
}


int Decoder::getFrame(AVPacket *packet, AVFrame *frame) {
	int result;

	bool eof = false;

	// Clear any previous frame
	av_frame_unref(frame);

	while ((result = avcodec_receive_frame(codec_ctx_, frame)) == AVERROR(EAGAIN) && !eof) {
		// Find next packet in the correct stream index
		do {
			// Free buffer in packet if there is one
			av_packet_unref(packet);

			// Read packet from file
			result = av_read_frame(fmt_ctx_, packet);
		} while (packet->stream_index != avstream_->index && result >= 0);

		if (result == AVERROR_EOF) {
			// Don't break so that receive gets called again, but don't try to read again
			eof = true;

			// Send a null packet to signal end of 
			avcodec_send_packet(codec_ctx_, NULL);
		}
		else if (result < 0) {
			// Handle other error by breaking loop and returning the code we received
			break;
		}
		else {
			// Successful read, send the packet
			result = avcodec_send_packet(codec_ctx_, packet);

			// We don't needd the packet anymore, so free it
			av_packet_unref(packet);

			if (result < 0)
				break;
		}
	}

	return result;
}


void Decoder::close(void) {
	if (sws_ctx_) {
		sws_freeContext(sws_ctx_);
		sws_ctx_ = NULL;
	}

	if (codec_ctx_) {
		avcodec_free_context(&codec_ctx_);
		codec_ctx_ = NULL;
	}

	if (fmt_ctx_) {
		avformat_close_input(&fmt_ctx_);
		fmt_ctx_ = NULL;
	}
}


FramePtr Decoder::retrieveAudio(const AudioParams &params, AVRational timecode) {
	uint8_t *data;

	AudioStreamPtr as = std::static_pointer_cast<AudioStream>(stream());

	int64_t target_ts = as->getTimeInTimeBaseUnits(timecode);

	// Retrieve frame data
	if ((data = retrieveAudioFrameData(params, target_ts)) == NULL)
		return NULL;

	// Return the frame
	FramePtr frame = Frame::create();

	// TODO : do better !!!
//	frame->setAudioParams();
	frame->setTimestamp(pts_);
	frame->setData(data);
	
	return frame;
}


uint8_t * Decoder::retrieveAudioFrameData(const AudioParams &params, const int64_t& target_ts) {
	int result;

	uint8_t *data = NULL;

	AVPacket *packet = av_packet_alloc();
	AVFrame *frame = av_frame_alloc();

	(void) params;
	(void) target_ts;

//	printf("RETRIEVE: %ld\n", target_ts);

	// Handle NULL channel layout
	uint64_t channel_layout = validateChannelLayout(avstream_);
	if (!channel_layout) {                     
		fprintf(stderr, "Failed to determine channel layout of audio file, could not conform\n");
//		return NULL;
	}                                          

//	// Create resampling context
//	SwrContext* resampler = swr_alloc_set_opts(NULL,
//			params.channelLayout(),                        
//			FFmpegUtils::getFFmpegSampleFormat(params.format()),
//			params.sampleRate(),                           
//			channel_layout,                                 
//			static_cast<AVSampleFormat>(avstream_->codecpar->format),
//			avstream_->codecpar->sample_rate,    
//			0,
//			NULL);                                       
//  
//	swr_init(resampler);   

	while (true) {
		// Pull from decoder
		result = getFrame(packet, frame);

		// Handle any errors that aren't EOF (EOF is handled later on)
		if ((result < 0) && (result != AVERROR_EOF)) {
			break;
		}

		if (result == AVERROR_EOF) {
			break;
		}

//		// Allocate buffers
//		int nb_samples = swr_get_out_samples(resampler, frame->nb_samples);
//		size_t size = params.samplesToBytes(nb_samples);
//		data = (uint8_t *) malloc(size * sizeof(uint8_t));

		// Store data (TOOD...)
		data = (uint8_t *) frame;

		pts_ = frame->pts;

		break;
	}

//	swr_free(&resampler);

//	printf("  PTS: %ld\n", pts_);

//	av_frame_free(&frame);
	av_packet_free(&packet);

	return data;
}


FramePtr Decoder::retrieveVideo(AVRational timecode) {
	uint8_t *data;

	VideoStreamPtr vs = std::static_pointer_cast<VideoStream>(stream());

	int64_t target_ts = vs->getTimeInTimeBaseUnits(timecode);

	// Retrieve frame data
	if ((data = retrieveVideoFrameData(target_ts)) == NULL)
		return NULL;

	// Return the frame
	FramePtr frame = Frame::create();

	frame->setVideoParams(VideoParams(vs->width(), vs->height(),
		native_pix_fmt_,
		native_nb_channels_,
		std::static_pointer_cast<VideoStream>(stream())->pixelAspectRatio(),
		std::static_pointer_cast<VideoStream>(stream())->interlacing()));
	// TODO : do better !!!
	frame->setTimestamp(pts_);
	frame->setData(data);
	
	return frame;
}

uint8_t * Decoder::retrieveVideoFrameData(const int64_t& target_ts) {
	int result;

	uint8_t *data = NULL;

	AVPacket *packet = av_packet_alloc();
	AVFrame *frame = av_frame_alloc();

	(void) target_ts;

//	printf("RETRIEVE: %ld\n", target_ts);

	while (true) {
		// Pull from decoder
		result = getFrame(packet, frame);

		// Handle any errors that aren't EOF (EOF is handled later on)
		if ((result < 0) && (result != AVERROR_EOF)) {
			break;
		}

		if (result == AVERROR_EOF) {
			break;
		}

		// Store data
		int linesize = Frame::generateLinesizeBytes(frame->width, native_pix_fmt_, native_nb_channels_);
		size_t size = VideoParams::getBufferSize(linesize, frame->height, native_pix_fmt_, native_nb_channels_);
//printf("linesize = [%d,%d,%d] / dst_linesize = %d / height = %d\n", 
//		frame->linesize[0], frame->linesize[1], frame->linesize[2], linesize, frame->height);
//printf("buffsize = %ld\n", size);
		data = (uint8_t *) malloc(size * sizeof(uint8_t));

		sws_scale(sws_ctx_,
			(const uint8_t * const *) frame->data,
			frame->linesize,
			0,
			frame->height,
			&data,
			&linesize);

		pts_ = frame->pts;

		break;
	}

//	printf("  PTS: %ld\n", pts_);

	av_frame_free(&frame);
	av_packet_free(&packet);

	return data;
}


uint64_t Decoder::validateChannelLayout(AVStream* stream) {
	if (stream->codecpar->channel_layout)
		return stream->codecpar->channel_layout;

	return av_get_default_channel_layout(stream->codecpar->channels);
} 


VideoParams::Format Decoder::getNativePixelFormat(AVPixelFormat pix_fmt) {
	switch (pix_fmt) {
	case AV_PIX_FMT_RGB24:
	case AV_PIX_FMT_RGBA:
		return VideoParams::FormatUnsigned8;
	case AV_PIX_FMT_RGB48:
	case AV_PIX_FMT_RGBA64:
		return VideoParams::FormatUnsigned16;
	default:
		return VideoParams::FormatInvalid;
	}
}


int Decoder::getNativeNbChannels(AVPixelFormat pix_fmt) {
	switch (pix_fmt) {
	case AV_PIX_FMT_RGB24:
	case AV_PIX_FMT_RGB48:
		return VideoParams::RGBChannelCount;
	case AV_PIX_FMT_RGBA:
	case AV_PIX_FMT_RGBA64:
		return VideoParams::RGBAChannelCount;
	default:
		return 0;
	}
}

