#include "ffmpegutils.h"
#include "audioparams.h"


AudioParams::AudioParams() 
	: sample_rate_(0)
	, format_(AudioParams::FormatInvalid)
	, channel_layout_(0) {
	time_base_ = av_make_q(1, sampleRate());
}


AudioParams::AudioParams(const int &sample_rate, 
	const uint64_t &channel_layout, 
	const AudioParams::Format &format)
	: sample_rate_(sample_rate)
	, format_(format)
	, channel_layout_(channel_layout) {
	time_base_ = av_make_q(1, sampleRate());
}


AudioParams::~AudioParams() {
}


const int& AudioParams::sampleRate(void) const {
	return sample_rate_;
}


const AudioParams::Format& AudioParams::format(void) const {
	return format_;
}


const uint64_t& AudioParams::channelLayout(void) const {
	return channel_layout_;
}


const AVRational& AudioParams::timeBase(void) const {
	return time_base_;
}


void AudioParams::setTimeBase(const AVRational &time_base) {
	time_base_ = time_base;
}


int AudioParams::channelCount() const { 
#ifdef HAVE_FFMPEG_CH_LAYOUT
	AVChannelLayout channel_layout = {};

	av_channel_layout_from_mask(&channel_layout, channelLayout());

	return channel_layout.nb_channels;
#else
	return av_get_channel_layout_nb_channels(channelLayout());
#endif
}   


int AudioParams::bytesPerSecond(void) const {
	return av_samples_get_buffer_size(NULL,
			channelCount(),
			sample_rate_,
			FFmpegUtils::getFFmpegSampleFormat(format_),
			1);
}


int AudioParams::bytesPerSamplePerChannel() const
{ 
	switch (format_) {
	case AudioParams::FormatUnsigned8:
		return 1;
	case AudioParams::FormatSigned16:
		return 2;
	case AudioParams::FormatSigned32:
	case AudioParams::FormatFloat32:
		return 4;
	case AudioParams::FormatSigned64:
	case AudioParams::FormatFloat64:
		return 8;
	case AudioParams::FormatInvalid:
	case AudioParams::FormatCount:
		break;
	}

	return 0;
}


int64_t AudioParams::samplesToBytes(const int64_t &samples) const {
	return samples * channelCount() * bytesPerSamplePerChannel();
} 

