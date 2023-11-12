#ifndef __GPX2VIDEO__ENCODER_H__
#define __GPX2VIDEO__ENCODER_H__

#include <iostream>
#include <memory>
#include <string>
#include <list>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#include "exportcodec.h"
#include "audioparams.h"
#include "videoparams.h"
#include "frame.h"


class EncoderSettings {
public:
	class Option {
	public:
		Option(std::string name, std::string value) 
			: name_(name)
			, value_(value) {
		}

		const std::string& name(void) const {
			return name_;
		}

		const std::string& value(void) const {
			return value_;
		}

	private:
		std::string name_;
		std::string value_;
	};

	EncoderSettings();
	virtual ~EncoderSettings();

	std::string filename(void) const;
	void setFilename(const std::string &filename);

	const VideoParams& videoParams(void) const;
	void setVideoParams(const VideoParams &video_params, ExportCodec::Codec codec);

	const AudioParams& audioParams(void) const;
	void setAudioParams(const AudioParams &audio_params, ExportCodec::Codec codec);

	// Video
	bool isVideoEnabled(void) const;

	const ExportCodec::Codec& videoCodec(void) const;

	const std::list<Option>& videoOptions(void) const;
	void setVideoOption(std::string name, std::string value);

	const int64_t& videoBitrate(void) const;
	void setVideoBitrate(const int64_t rate);

	const int64_t& videoMinBitrate(void) const;
	void setVideoMinBitrate(const int64_t rate);

	const int64_t& videoMaxBitrate(void) const;
	void setVideoMaxBitrate(const int64_t rate);

	const int64_t& videoBufferSize(void) const;
	void setVideoBufferSize(const int64_t size);

	// Audio
	bool isAudioEnabled(void) const;

	const ExportCodec::Codec& audioCodec(void) const;

	void setAudioBitrate(const int64_t rate);

private:
	std::string filename_;

	bool video_enabled_;
	VideoParams video_params_;
	ExportCodec::Codec video_codec_;
	std::list<Option> video_options_;
	int64_t video_bit_rate_;
	int64_t video_min_bit_rate_;
	int64_t video_max_bit_rate_;
	int64_t video_buffer_size_;

	bool audio_enabled_;
	AudioParams audio_params_;
	ExportCodec::Codec audio_codec_;
	int64_t audio_bit_rate_;
};


class Encoder {
public:
	virtual ~Encoder();

	static Encoder * create(const EncoderSettings &settings);

	const EncoderSettings& settings() const;

	bool open(void);
	void close(void);

	bool writeAudio(FramePtr frame, AVRational time);
	bool writeFrame(FramePtr frame, AVRational time);

private:
	Encoder(const EncoderSettings &settings);

	void flush(void);
	void flush(AVCodecContext *codec_ctx, AVStream *stream);

	bool initializeStream(AVMediaType type, AVStream **stream_ptr, AVCodecContext **codec_context_ptr, const ExportCodec::Codec &codec);

	void setRotation(double theta);

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

