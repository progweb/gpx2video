#include "log.h"
#include "ffmpegutils.h"
#include "encoder.h"


EncoderSettings::EncoderSettings() :
	video_enabled_(false),
	video_bit_rate_(0),
	video_min_bit_rate_(0),
	video_max_bit_rate_(0),
	video_buffer_size_(0),
	audio_enabled_(false) {
}


EncoderSettings::~EncoderSettings() {
}


std::string EncoderSettings::filename(void) const {
	return filename_;
}


void EncoderSettings::setFilename(const std::string &filename) {
	filename_ = filename;
}


const VideoParams& EncoderSettings::videoParams(void) const {
	return video_params_;
}


void EncoderSettings::setVideoParams(const VideoParams &video_params, ExportCodec::Codec codec) {
	video_enabled_ = true;
	video_params_ = video_params;
	video_codec_ = codec;
}


const AudioParams& EncoderSettings::audioParams(void) const {
	return audio_params_;
}


void EncoderSettings::setAudioParams(const AudioParams &audio_params, ExportCodec::Codec codec) {
	audio_enabled_ = true;
	audio_params_ = audio_params;
	audio_codec_ = codec;
}


bool EncoderSettings::isVideoEnabled(void) const {
	return video_enabled_;
}


const ExportCodec::Codec& EncoderSettings::videoCodec(void) const {
	return video_codec_;
}


const std::string& EncoderSettings::videoHardwareDevice(void) const {
	return video_hw_device_;
}


void EncoderSettings::setVideoHardwareDevice(std::string hw_device) {
	video_hw_device_ = hw_device;
}


const std::list<EncoderSettings::Option>& EncoderSettings::videoOptions(void) const {
	return video_options_;
}


void EncoderSettings::setVideoOption(std::string name, std::string value) {
	video_options_.push_back(EncoderSettings::Option(name, value));
}


const int64_t& EncoderSettings::videoBitrate(void) const {
	return video_bit_rate_;
}


void EncoderSettings::setVideoBitrate(const int64_t rate) {
	video_bit_rate_ = rate;
}


const int64_t& EncoderSettings::videoMinBitrate(void) const {
	return video_min_bit_rate_;
}


void EncoderSettings::setVideoMinBitrate(const int64_t rate) {
	video_min_bit_rate_ = rate;
}


const int64_t& EncoderSettings::videoMaxBitrate(void) const {
	return video_max_bit_rate_;
}


void EncoderSettings::setVideoMaxBitrate(const int64_t rate) {
	video_max_bit_rate_ = rate;
}


const int64_t& EncoderSettings::videoBufferSize(void) const {
	return video_buffer_size_;
}


void EncoderSettings::setVideoBufferSize(const int64_t size) {
	video_buffer_size_ = size;
}


bool EncoderSettings::isAudioEnabled(void) const {
	return audio_enabled_;
}


const ExportCodec::Codec& EncoderSettings::audioCodec(void) const {
	return audio_codec_;
}


void EncoderSettings::setAudioBitrate(const int64_t rate) {
	audio_bit_rate_ = rate;
}


Encoder::Encoder(const EncoderSettings &settings) : 
	settings_(settings),
	open_(false),
	fmt_ctx_(NULL),
	video_stream_(NULL),
	video_codec_(NULL),
	audio_stream_(NULL),
	audio_codec_(NULL),
	hw_device_ctx_(NULL) {
	log_call();
}


Encoder::~Encoder() {
	close();
}


const EncoderSettings& Encoder::settings() const {
	return settings_;
}


Encoder * Encoder::create(const EncoderSettings &settings) {
	Encoder *encoder = new Encoder(settings);

	return encoder;
}


bool Encoder::open(void) {
	int result;

	// Create output context
    result = avformat_alloc_output_context2(&fmt_ctx_, NULL, NULL, settings_.filename().c_str());

	if (result < 0) {
		av_log(NULL, AV_LOG_ERROR, "Failed to allocate output context\n");
		return false;
	}

	// Initialize video stream
	if (settings().isVideoEnabled()) {
		if (!this->initializeStream(AVMEDIA_TYPE_VIDEO, &video_stream_, &video_codec_, settings_.videoCodec()))
			return false;

		// Decoder use a compatible AVPixelFormat
		AVPixelFormat ideal_pix_fmt = FFmpegUtils::getCompatiblePixelFormat(settings_.videoParams().pixelFormat());

		// Apply rotation
		this->setRotation(settings_.videoParams().orientation());

//		if (theta < -180)
//			theta += 360;
//		else if (theta >= 180.0)
//			theta -= 360;
//		theta = -theta;

//		// This is the format we will expect frames received in Write() to be in
//		VideoParams::Format native_pix_fmt = settings_.videoParams().format();
//
//		// This is the format we will need to convert the frame for swscale to understand it
//		video_conversion_fmt_ = FFmpegUtils::getCompatiblePixelFormat(native_pix_fmt);
//
//		// This is the equivalent pixel format above as an AVPixelFormat that swscale can understand
//		AVPixelFormat src_alpha_pix_fmt = FFmpegUtils::getFFmpegPixelFormat(video_conversion_fmt_, VideoParams::RGBAChannelCount);
//		AVPixelFormat src_noalpha_pix_fmt =  FFmpegUtils::getFFmpegPixelFormat(video_conversion_fmt_, VideoParams::RGBChannelCount);
//
//		// Set up a scaling context - if the native pixel format is not equal to the encoder's, we'll need to convert
//		// it before encoding. Even if we don't, this may be useful for converting between linesizes, etc.
//		alpha_sws_ctx_ = sws_getContext(settings_.videoParams().width(), settings_.videoParams().height(), 
//			src_alpha_pix_fmt,
//			settings_.videoParams().width(), settings_.videoParams().height(), 
//			(AVPixelFormat) video_codec_->pix_fmt,
//			0, NULL, NULL, NULL);
//
//		noalpha_sws_ctx_ = sws_getContext(settings_.videoParams().width(), settings_.videoParams().height(), 
//			src_noalpha_pix_fmt,
//			settings_.videoParams().width(), settings_.videoParams().height(), 
//			(AVPixelFormat) video_codec_->pix_fmt,
//			0, NULL, NULL, NULL);

		sws_ctx_ = sws_getContext(settings_.videoParams().width(), settings_.videoParams().height(), 
			ideal_pix_fmt,
			settings_.videoParams().width(), settings_.videoParams().height(), 
			settings().videoParams().pixelFormat(),
			0, NULL, NULL, NULL);

	}

	// Initialize audio stream
	if (settings().isAudioEnabled()) {
		if (!this->initializeStream(AVMEDIA_TYPE_AUDIO, &audio_stream_, &audio_codec_, settings_.audioCodec()))
			return false;
	}

	// Dump info
    av_dump_format(fmt_ctx_, 0, settings_.filename().c_str(), 1);

	// Open output file for writing
	if (!(fmt_ctx_->oformat->flags & AVFMT_NOFILE)) {
		result = avio_open(&fmt_ctx_->pb, settings_.filename().c_str(), AVIO_FLAG_WRITE);

		if (result < 0) {
			av_log(NULL, AV_LOG_ERROR, "Could not open output file '%s'\n", settings_.filename().c_str());
			return false;
		}
	}

    // Init muxer, write output file header
    result = avformat_write_header(fmt_ctx_, NULL);

	if (result < 0) {
        av_log(NULL, AV_LOG_ERROR, "Error occurred when opening output file\n");
        return false;
    }

	open_ = true;

	return true;
}


void Encoder::close(void) {
	log_call();

	if (open_) {
		this->flush();

		// Write trailer
		av_write_trailer(fmt_ctx_);

		if (!(fmt_ctx_->oformat->flags & AVFMT_NOFILE))
			avio_closep(&fmt_ctx_->pb);

		open_ = false;
	}

	if (sws_ctx_) {
		sws_freeContext(sws_ctx_);
		sws_ctx_ = NULL;
	}

	if (video_codec_) {
		avcodec_free_context(&video_codec_);
		video_codec_ = NULL;
	}

	if (audio_codec_) {
		avcodec_free_context(&audio_codec_);
		audio_codec_ = NULL;
	}

	if (hw_device_ctx_) {
		av_buffer_unref(&hw_device_ctx_);
		hw_device_ctx_ = NULL;
	}

	if (fmt_ctx_) {
		avformat_free_context(fmt_ctx_);
		fmt_ctx_ = NULL;
	}
}


void Encoder::flush(void) {
	if (video_codec_)
		flush(video_codec_, video_stream_);
	
	if (audio_codec_)
		flush(audio_codec_, audio_stream_);
}


void Encoder::flush(AVCodecContext *codec_ctx, AVStream *stream) {
	avcodec_send_frame(codec_ctx, NULL);

	AVPacket *packet = av_packet_alloc();

	int result;

	do {
		result = avcodec_receive_packet(codec_ctx, packet);

		if (result < 0)
			break;

		packet->stream_index = stream->index;

		av_packet_rescale_ts(packet, codec_ctx->time_base, stream->time_base);
		av_interleaved_write_frame(fmt_ctx_, packet);
		av_packet_unref(packet);
	} while (result >= 0);

	av_packet_free(&packet);
}


bool Encoder::initializeStream(AVMediaType type, AVStream **stream_ptr, AVCodecContext **codec_context_ptr, const ExportCodec::Codec &codec) {
	int result;

	AVStream *stream;
	AVCodecContext *codec_context;

	log_call();

	log_info("Initialize stream encoder in using '%s' codec", ExportCodec::getCodecName(codec).c_str());

	// Find encoder with this name
	const AVCodec *encoder = FFmpegUtils::getEncoder(codec);

	if (!encoder) {
		av_log(NULL, AV_LOG_FATAL, "Failed to find encoder for codec '%s'\n", ExportCodec::getCodecName(codec).c_str());
		return false;
	}

	if (type == AVMEDIA_TYPE_VIDEO) {
		if (codec == ExportCodec::CodecVAAPIH264)
			av_hwdevice_ctx_create(&hw_device_ctx_, AV_HWDEVICE_TYPE_VAAPI, settings().videoHardwareDevice().c_str(), NULL, 0);
	}

	// Create stream
	stream = avformat_new_stream(fmt_ctx_, NULL);

	if (!stream) {
		av_log(NULL, AV_LOG_ERROR, "Failed allocating output stream\n");
		return false;
	}

	codec_context = avcodec_alloc_context3(encoder);
	if (!codec_context) {
		av_log(NULL, AV_LOG_FATAL, "Failed to allocate the encoder context\n");
		return false;
	}

	// Codec parameters
	switch (type) {
	case AVMEDIA_TYPE_VIDEO:
		codec_context->width = settings().videoParams().width();
		codec_context->height = settings().videoParams().height();
		codec_context->sample_aspect_ratio = settings().videoParams().pixelAspectRatio();
		codec_context->pix_fmt = settings().videoParams().pixelFormat();
		codec_context->framerate = settings().videoParams().frameRate();
		codec_context->time_base = settings().videoParams().timeBase();

		if (codec == ExportCodec::CodecVAAPIH264)
			codec_context->pix_fmt = AV_PIX_FMT_VAAPI;

		if (settings().videoParams().interlacing() != VideoParams::InterlaceNone) {
//			codec/ffmpeg/ffmpegencoder.cpp:503
//			codec_context->flags |= AV_CODEC_FLAG_INTERLACED_DCT | AV_CODEC_FLAG_INTERLACED_ME;

			if (settings().videoParams().interlacing() == VideoParams::InterlacedTopFirst)
				codec_context->field_order = AV_FIELD_TT;
			else {
				codec_context->field_order = AV_FIELD_BB;

				// For some reason, FFmpeg doesn't set libx264's bff flag so we have to do it ourselves
				if (codec == ExportCodec::CodecH264)
					av_opt_set(codec_context->priv_data, "x264opts", "bff=1", AV_OPT_SEARCH_CHILDREN);
			}
		}

		if (codec == ExportCodec::CodecNVEncH264)
			codec_context->dct_algo = FF_DCT_FASTINT;

		// Custom options
		if (settings().videoBitrate() > 0)
			codec_context->bit_rate = settings().videoBitrate(); //4 * 1000 * 1000 * 8;
		if (settings().videoMinBitrate() > 0)
			codec_context->rc_min_rate = settings().videoMinBitrate(); // 0 // 8 * 1000 * 1000;
		if (settings().videoMaxBitrate() > 0)
			codec_context->rc_max_rate = settings().videoMaxBitrate(); //2 * 1000 * 1000 * 16;
		if (settings().videoBufferSize() > 0)
			codec_context->rc_buffer_size = settings().videoBufferSize(); //4 * 1000 * 1000 / 2;

		for (EncoderSettings::Option opt : settings().videoOptions()) {
			log_info("Set video encoder option: '%s' = '%s'", opt.name().c_str(), opt.value().c_str());

			av_opt_set(codec_context->priv_data, opt.name().c_str(), opt.value().c_str(), AV_OPT_SEARCH_CHILDREN);
		}
		break;

	case AVMEDIA_TYPE_AUDIO: {
			const enum AVSampleFormat *sample_fmts = NULL;

#ifdef HAVE_FFMPEG_SUPPORTED_CONFIG
			avcodec_get_supported_config(NULL, encoder,
					AV_CODEC_CONFIG_SAMPLE_FORMAT, 0, (const void **) &sample_fmts, NULL);
#else
			sample_fmts = encoder->sample_fmts;
#endif

			codec_context->sample_rate = settings().audioParams().sampleRate();
#ifdef HAVE_FFMPEG_CH_LAYOUT
			av_channel_layout_from_mask(&codec_context->ch_layout, settings().audioParams().channelLayout());
#else
			codec_context->channel_layout = settings().audioParams().channelLayout();
			codec_context->channels = av_get_channel_layout_nb_channels(codec_context->channel_layout);
#endif
			// take first format from list of supported formats
			if (sample_fmts != NULL)
				codec_context->sample_fmt = sample_fmts[0];

			codec_context->time_base = (AVRational) {1, codec_context->sample_rate};
		}
		break;

	default:
		break;
	}

	// At last
	if (fmt_ctx_->oformat->flags & AVFMT_GLOBALHEADER)
		codec_context->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

	// Use hardware context for VAAPI encoder
	if (type == AVMEDIA_TYPE_VIDEO) {
		if (codec == ExportCodec::CodecVAAPIH264) {
			AVBufferRef *hw_frames_ref;
			AVHWFramesContext *frames_ctx;

			hw_frames_ref = av_hwframe_ctx_alloc(hw_device_ctx_);

			frames_ctx = (AVHWFramesContext *) (hw_frames_ref->data);
			frames_ctx->format    = codec_context->pix_fmt;	 // AV_PIX_FMT_VAAPI
			frames_ctx->sw_format = settings().videoParams().pixelFormat(); // AV_PIX_FMT_NV12;
			frames_ctx->width     = settings().videoParams().width();
			frames_ctx->height    = settings().videoParams().height();
			frames_ctx->initial_pool_size = 20;

			av_hwframe_ctx_init(hw_frames_ref);

			codec_context->hw_frames_ctx = av_buffer_ref(hw_frames_ref);

			av_buffer_unref(&hw_frames_ref);
		}
	}

	// Try to open encoder (third parameter can be used to pass settings to encoder)
	result = avcodec_open2(codec_context, encoder, NULL);

	if (result < 0) {
		av_log(NULL, AV_LOG_ERROR, "Failed to open encoder\n");
		return false;
	}

	// Copy context settings to codecpar object
	result = avcodec_parameters_from_context(stream->codecpar, codec_context);

	if (result < 0) {
		av_log(NULL, AV_LOG_ERROR, "Failed to copy encoder parameters to stream\n");
		return false;
	}

	// FIXME ???
	stream->time_base = codec_context->time_base;
//	stream->codecpar->codec_tag = 0;

	if (type == AVMEDIA_TYPE_VIDEO) {
		stream->avg_frame_rate = codec_context->framerate;
	}

	*stream_ptr = stream;
	*codec_context_ptr = codec_context;

	return true;
}


void Encoder::setRotation(double theta) {
	uint8_t *displaymatrix;

	if (theta == 0)
		return;

	displaymatrix = FFmpegUtils::newSideData(video_stream_, AV_PKT_DATA_DISPLAYMATRIX, sizeof(int32_t) * 9);

	av_display_rotation_set((int32_t*) displaymatrix, theta);

	av_dict_set(&video_stream_->metadata, "rotate", NULL, 0);
}


bool Encoder::writeAudio(FramePtr frame, AVRational time) {
	bool success = false;

	AVFrame *encoded_frame = (AVFrame *) frame->data();

	(void) time;

	success = writeAVFrame(encoded_frame, audio_codec_, audio_stream_);

	av_frame_free(&encoded_frame);

	frame->setData(NULL);

	return success;
}


bool Encoder::writeFrame(FramePtr frame, AVRational time) {
	int result;

	bool success = false;

	int input_linesize;
	const uint8_t *input_data;

	AVFrame *encoded_frame = av_frame_alloc();

	// Frame must be video
	encoded_frame->width = frame->videoParams().width();
	encoded_frame->height = frame->videoParams().height();
	encoded_frame->format = settings().videoParams().pixelFormat();

//	// TODO / FIXME !!!
//	encoded_frame->linesize[0] = 2752;
//	encoded_frame->linesize[1] = 1376;
//	encoded_frame->linesize[2] = 1376;

//printf("width x height: %d x %d\n", encoded_frame->width, encoded_frame->height);
//printf("format = %d\n", encoded_frame->format);
	// Set interlacing
	if (frame->videoParams().interlacing() != VideoParams::InterlaceNone) {
#ifdef HAVE_FFMPEG_FLAGS_FRAME
		encoded_frame->flags |= AV_FRAME_FLAG_INTERLACED;
#else
		encoded_frame->interlaced_frame = 1;
#endif

		if (frame->videoParams().interlacing() == VideoParams::InterlacedTopFirst)
#ifdef HAVE_FFMPEG_FLAGS_FRAME
			encoded_frame->flags |= AV_FRAME_FLAG_TOP_FIELD_FIRST;
#else
			encoded_frame->top_field_first = 1;
		else
			encoded_frame->top_field_first = 0;
#endif
	}

	// Create encoded buffer
	result = av_frame_get_buffer(encoded_frame, 0);
	if (result < 0) {
		av_log(NULL, AV_LOG_ERROR, "Failed to create AVFrame buffer\n");
		goto fail;
	}

	// We may need to convert this frame to a frame that swscale will understand
	// TODO...
//	if (frame->videoParams().format() != video_conversion_fmt_) {
//		av_log(NULL, AV_LOG_ERROR, "NEED TO CONVERT THIS FRAME\n");
//	}

	// Use swscale context to convert formats/linesizes
	input_data = frame->constData();
	input_linesize = frame->linesizeBytes();

	result = sws_scale(sws_ctx_,
//	result = sws_scale((frame->videoParams().nbChannels() == VideoParams::RGBAChannelCount) ? alpha_sws_ctx_ : noalpha_sws_ctx_,
			reinterpret_cast<const uint8_t * const *>(&input_data),
			&input_linesize,
			0,
			frame->videoParams().height(),
			encoded_frame->data,
			encoded_frame->linesize);
//printf("linesize = [%d,%d,%d] / dst_linesize = %d / height = %d\n", 
//		encoded_frame->linesize[0], encoded_frame->linesize[1], encoded_frame->linesize[2], input_linesize, encoded_frame->height);

	if (result < 0) {
		av_log(NULL, AV_LOG_ERROR, "Failed to scale frame\n");
		goto fail;
	}

	// TODO	
	encoded_frame->pts = (uint64_t) round(av_q2d(time) / av_q2d(video_codec_->time_base));
//	encoded_frame->pts = (uint64_t) round(av_q2d(time));

	// If hardware acceleration using
	if (video_codec_->hw_frames_ctx != NULL) {
		AVFrame *hw_frame;

		hw_frame = av_frame_alloc();
		av_hwframe_get_buffer(video_codec_->hw_frames_ctx, hw_frame, 0);
		av_hwframe_transfer_data(hw_frame, encoded_frame, 0);
		av_frame_free(&encoded_frame);

		encoded_frame = hw_frame;
	}

	// Write to encoder
	success = writeAVFrame(encoded_frame, video_codec_, video_stream_);

fail:
	av_frame_free(&encoded_frame);

	return success;
}


bool Encoder::writeAVFrame(AVFrame *frame, AVCodecContext *codec_ctx, AVStream *stream) {
	int result;

	// Send raw frame to the encoder
	result = avcodec_send_frame(codec_ctx, frame);

	if (result < 0) {
		av_log(NULL, AV_LOG_ERROR, "Failed to send frame to encoder\n");
		return false;
	}

	AVPacket *packet = av_packet_alloc();

	while (result >= 0) {
		result = avcodec_receive_packet(codec_ctx, packet);

		// EAGAIN just means the encoder wants another frame before encoding
		if ((result == AVERROR(EAGAIN)) || (result == AVERROR_EOF))
			break;
		else if (result < 0) {
			av_log(NULL, AV_LOG_ERROR, "Failed to receive packet from decoder");
			break;
		}

		// Set packet stream index
		packet->stream_index = stream->index;

        av_packet_rescale_ts(packet, codec_ctx->time_base, stream->time_base);

		// Mux encoded frame
        av_interleaved_write_frame(fmt_ctx_, packet);

		// Unref packet in case we're getting another
		av_packet_unref(packet);
	}

	av_packet_free(&packet);

	return (result < 0) ? false : true;
}

