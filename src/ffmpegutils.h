#ifndef __GPX2VIDEO__FFMPEGUTILS_H__
#define __GPX2VIDEO__FFMPEGUTILS_H__

#include <string>
#include <vector>

extern "C" {
#include <libavutil/eval.h>
#include <libavutil/opt.h>
#include <libavutil/display.h>
#include <libavutil/channel_layout.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

}

#include "exportcodec.h"
#include "audioparams.h"
#include "videoparams.h"


class FFmpegUtils {
public:
	static const AVCodec * getEncoder(ExportCodec::Codec c);

	static AVSampleFormat getFFmpegSampleFormat(const AudioParams::Format &format);

	static AVPixelFormat getCompatiblePixelFormat(const AVPixelFormat &pix_fmt);
	static VideoParams::Format getCompatiblePixelFormat(const VideoParams::Format &format);
	static AVPixelFormat getFFmpegPixelFormat(const VideoParams::Format &format, int nb_channels);

	static AVPixelFormat overrideFFmpegDeprecatedPixelFormat(const AVPixelFormat &pix_fmt);
};

#endif

