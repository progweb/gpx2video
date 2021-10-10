#include "ffmpegutils.h"


AVSampleFormat FFmpegUtils::getFFmpegSampleFormat(const AudioParams::Format &format)
{
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

