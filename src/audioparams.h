#ifndef __GPX2VIDEO__AUDIOPARAMS_H__
#define __GPX2VIDEO__AUDIOPARAMS_H__

#include <iostream>
#include <memory>
#include <string>

extern "C" {
#include <libavutil/channel_layout.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}


class AudioParams {
public:
	enum Format {
		FormatInvalid = -1,
		FormatUnsigned8,
		FormatSigned16,
		FormatSigned32,
		FormatSigned64,
		FormatFloat32,
		FormatFloat64,
		FormatCount
	};

	AudioParams();
	AudioParams(const int &sample_rate, 
		const uint64_t &channel_layout, 
		const Format &format);

	virtual ~AudioParams();

	const int& sampleRate(void) const;

	const Format& format(void) const;

	const uint64_t& channelLayout(void) const;

	const AVRational& timeBase(void) const;
	void setTimeBase(const AVRational &time_base);

	int channelCount(void) const;
	int bytesPerSecond(void) const;
	int bytesPerSamplePerChannel(void) const;
	int64_t samplesToBytes(const int64_t &samples) const;

private:
	int sample_rate_;
	Format format_;
	uint64_t channel_layout_;

	AVRational time_base_;
};

#endif

