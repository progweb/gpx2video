#include "ffmpegutils.h"


const AVCodec * FFmpegUtils::getEncoder(ExportCodec::Codec codec) {
	switch (codec) {
	case ExportCodec::CodecH264:
		return avcodec_find_encoder_by_name("libx264");
	case ExportCodec::CodecNVEncH264:
		return avcodec_find_encoder_by_name("h264_nvenc");
	case ExportCodec::CodecVAAPIH264:
		return avcodec_find_encoder_by_name("h264_vaapi");
	case ExportCodec::CodecQSVH264:
		return avcodec_find_encoder_by_name("h264_qsv");
	case ExportCodec::CodecHEVC:
		return avcodec_find_encoder(AV_CODEC_ID_HEVC);
	case ExportCodec::CodecNVEncHEVC:
		return avcodec_find_encoder_by_name("hevc_nvenc");
	case ExportCodec::CodecQSVHEVC:
		return avcodec_find_encoder_by_name("hevc_qsv");
	
	case ExportCodec::CodecAAC:
		return avcodec_find_encoder(AV_CODEC_ID_AAC);

	case ExportCodec::CodecCount:
	default:
		break;
	}

	return NULL;
}


AVSampleFormat FFmpegUtils::getFFmpegSampleFormat(const AudioParams::Format &format) {
	switch (format) {
	case AudioParams::FormatUnsigned8:
		return AV_SAMPLE_FMT_U8;
	case AudioParams::FormatSigned16:
		return AV_SAMPLE_FMT_S16;
	case AudioParams::FormatSigned32:
		return AV_SAMPLE_FMT_S32;
	case AudioParams::FormatSigned64:
		return AV_SAMPLE_FMT_S64;
	case AudioParams::FormatFloat32:
		return AV_SAMPLE_FMT_FLT;
	case AudioParams::FormatFloat64:
		return AV_SAMPLE_FMT_DBL;
	case AudioParams::FormatInvalid:
	case AudioParams::FormatCount:
		break;
	} 

	return AV_SAMPLE_FMT_NONE;
}


AVPixelFormat FFmpegUtils::getCompatiblePixelFormat(const AVPixelFormat &pix_fmt) {
	enum AVPixelFormat possible_pix_fmts[] = {
//		AV_PIX_FMT_RGB24,
		AV_PIX_FMT_RGBA,
//		AV_PIX_FMT_RGB48,
		AV_PIX_FMT_RGBA64,
		AV_PIX_FMT_NONE
	};

	return avcodec_find_best_pix_fmt_of_list(possible_pix_fmts, pix_fmt, 1, NULL);
}


VideoParams::Format FFmpegUtils::getCompatiblePixelFormat(const VideoParams::Format &format) {
	switch (format) {
	case VideoParams::FormatUnsigned8:
		return VideoParams::FormatUnsigned8;
	case VideoParams::FormatUnsigned16:
	case VideoParams::FormatFloat16:
	case VideoParams::FormatFloat32:
		return VideoParams::FormatUnsigned16;
	case VideoParams::FormatInvalid:
	case VideoParams::FormatCount:
	default:
		break;
	}

	return VideoParams::FormatInvalid;
}


AVPixelFormat FFmpegUtils::getFFmpegPixelFormat(const VideoParams::Format &format, int nb_channels) {
	if (nb_channels == VideoParams::RGBChannelCount) {
		switch (format) {
		case VideoParams::FormatUnsigned8:
			return AV_PIX_FMT_RGB24;
		case VideoParams::FormatUnsigned16:
			return AV_PIX_FMT_RGB48;
		case VideoParams::FormatFloat16:
		case VideoParams::FormatFloat32:
		case VideoParams::FormatInvalid:
		case VideoParams::FormatCount:
		default:
			break;
		}
	}
	else if (nb_channels == VideoParams::RGBAChannelCount) {
		switch (format) {
		case VideoParams::FormatUnsigned8:
			return AV_PIX_FMT_RGBA;
		case VideoParams::FormatUnsigned16:
			return AV_PIX_FMT_RGBA64;
		case VideoParams::FormatFloat16:
		case VideoParams::FormatFloat32:
		case VideoParams::FormatInvalid:
		case VideoParams::FormatCount:
		default:
			break;
		}
	}

	return AV_PIX_FMT_NONE;
}

AVPixelFormat FFmpegUtils::overrideFFmpegDeprecatedPixelFormat(const AVPixelFormat &pix_fmt) {
	switch (pix_fmt) {
	case AV_PIX_FMT_YUVJ420P:
		return AV_PIX_FMT_YUV420P;
	case AV_PIX_FMT_YUVJ411P:
		return AV_PIX_FMT_YUV411P;
	case AV_PIX_FMT_YUVJ422P:
		return AV_PIX_FMT_YUV422P;
	case AV_PIX_FMT_YUVJ444P:
		return AV_PIX_FMT_YUV444P;
	case AV_PIX_FMT_YUVJ440P:
		return AV_PIX_FMT_YUV440P;
	default:
		break;
	}

	return pix_fmt;
}


uint8_t *FFmpegUtils::newSideData(AVStream* stream, enum AVPacketSideDataType type, size_t size) {
#ifdef HAVE_FFMPEG_API_SIDE_DATA
	const AVPacketSideData *sd = NULL;

	AVCodecParameters *codecpar = stream->codecpar;

	sd = av_packet_side_data_new(&codecpar->coded_side_data, &codecpar->nb_coded_side_data,
			type, size, 0);

	return sd->data;
#else
	return av_stream_new_side_data(stream, type, size);
#endif
}


uint8_t *FFmpegUtils::getSideData(AVStream* stream, enum AVPacketSideDataType type) {
#ifdef HAVE_FFMPEG_API_SIDE_DATA
	const AVPacketSideData *sd = NULL;

	AVCodecParameters *codecpar = stream->codecpar;

	sd = av_packet_side_data_get(codecpar->coded_side_data, codecpar->nb_coded_side_data, type);

	if (sd)
		return sd->data;

	return NULL;
#else
	uint8_t *data;

	data = av_stream_get_side_data(stream, type, NULL);

	return data;
#endif
}

