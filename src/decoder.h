#ifndef __GPX2VIDEO__DECODER_H__
#define __GPX2VIDEO__DECODER_H__

#include <string>
#include <vector>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

#include "frame.h"
#include "stream.h"
#include "media.h"


class Decoder {
public:
	virtual ~Decoder();

	static MediaContainer * probe(const std::string &filename);

	static Decoder * create(void);

	bool open(StreamPtr stream);
	int getFrame(AVPacket *packet, AVFrame *frame);
	void close(void);

	const AVCodecID& codec(void) const;

	FramePtr retrieveAudio(const AudioParams &params, AVRational timecode, int duration);
	uint8_t * retrieveAudioFrameData(const AudioParams &params, const int64_t& target_ts, const int& duration);

	FramePtr retrieveVideo(AVRational timecode);
	uint8_t * retrieveVideoFrameData(const int64_t& target_ts);

protected:
	StreamPtr stream(void) const {
		return stream_;
	}

	bool open(const std::string &filename, const int &index);

private:
	double getRotation(AVStream* stream);

	static uint64_t validateChannelLayout(AVStream* stream);
	static VideoParams::Format getNativePixelFormat(AVPixelFormat pix_fmt);
	static int getNativeNbChannels(AVPixelFormat pix_fmt);

	Decoder();

	StreamPtr stream_;

	AVFormatContext *fmt_ctx_;

	AVStream *avstream_;
	AVCodecContext *codec_ctx_;

	AVPixelFormat ideal_pix_fmt_;
	VideoParams::Format native_pix_fmt_;
	int native_nb_channels_;

	SwsContext *sws_ctx_;

	int64_t pts_;
};

#endif

