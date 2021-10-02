#ifndef __GPX2VIDEO__ENCODER_H__
#define __GPX2VIDEO__ENCODER_H__

#include <iostream>
#include <memory>
#include <string>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#include "audioparams.h"
#include "videoparams.h"
#include "frame.h"


class EncoderSettings {
public:
	EncoderSettings();
	virtual ~EncoderSettings();

	std::string filename(void) const;
	void setFilename(const std::string &filename);

	const VideoParams& videoParams(void) const;
	void setVideoParams(const VideoParams &video_params, AVCodecID codec_id);

	const AudioParams& audioParams(void) const;
	void setAudioParams(const AudioParams &audio_params, AVCodecID codec_id);

	bool isVideoEnabled(void) const;
	void setVideoBitrate(const int64_t rate);
	void setVideoMaxBitrate(const int64_t rate);
	void setVideoBufferSize(const int64_t size);

	bool isAudioEnabled(void) const;

private:
	std::string filename_;

	bool video_enabled_;
	VideoParams video_params_;
	AVCodecID video_codec_id_;
	int64_t video_bit_rate_;
	int64_t video_max_bit_rate_;
	int64_t video_buffer_size_;

	bool audio_enabled_;
	AudioParams audio_params_;
	AVCodecID audio_codec_id_;
};


class Encoder {
public:
	virtual ~Encoder();

	static Encoder * create(const EncoderSettings &settings);

	const EncoderSettings& settings() const;

	bool open(void);
	void close(void);

	bool writeFrame(FramePtr frame, AVRational time);

private:
	Encoder(const EncoderSettings &settings);

	void flush(void);
	void flush(AVCodecContext *codec_ctx, AVStream *stream);

	bool initializeStream(AVMediaType type, AVStream **stream_ptr, AVCodecContext **codec_context_ptr, AVCodecID codec_id);

	bool writeAVFrame(AVFrame *frame, AVCodecContext *codec_ctx, AVStream *stream);

	EncoderSettings settings_;

	bool open_;

	AVFormatContext *fmt_ctx_;

	AVStream *video_stream_;
	AVCodecContext *video_codec_;

	AVStream *audio_stream_;
	AVCodecContext *audio_codec_;

	SwsContext *sws_ctx_;
	SwsContext *alpha_sws_ctx_;
	SwsContext *noalpha_sws_ctx_;
	VideoParams::Format video_conversion_fmt_;
};

#endif

