#include <iostream>
#include <memory>
#include <string>
#include <algorithm>

#include "log.h"
#include "ffmpegutils.h"
#include "samplebuffer.h"


SampleBuffer::SampleBuffer()
	: sample_count_per_channel_(0) {
}


SampleBuffer::~SampleBuffer() {
}


SampleBufferPtr SampleBuffer::create(void) {
	return std::make_shared<SampleBuffer>();
}


const AudioParams& SampleBuffer::audioParams(void) const {
	return audio_params_;
}


void SampleBuffer::setAudioParams(const AudioParams &audio_params) {
	audio_params_ = audio_params;
}


uint64_t SampleBuffer::time(void) const {
	return timestamp() * av_q2d(audioParams().timeBase()) * 1000.0;
}


const uint64_t& SampleBuffer::timestamp() const {
	return pts_;
}


void SampleBuffer::setTimestamp(const uint64_t &timestamp) {
	pts_ = timestamp;
}


const int& SampleBuffer::sampleCount(void) const {
	return sample_count_per_channel_;
}


void SampleBuffer::setSampleCount(const int &sample_count) {
	sample_count_per_channel_ = sample_count;
}


int SampleBuffer::channelCount(void) const {
	return audio_params_.channelCount();
}


bool SampleBuffer::isPlanar(void) const {
	return false;
}


bool SampleBuffer::isAllocated(void) const {
	return !data_.empty();
}


void SampleBuffer::allocate(void) {
	int nb_samples;
	int nb_channels;

	if (!sample_count_per_channel_) {
		log_warn("Tried to allocate sample buffer with zero sample count");
		return;
	}

	if (isAllocated()) {
		log_warn("Tried to allocate already allocated sample buffer");
		return;
	}

	nb_channels = !isPlanar() ? audio_params_.channelCount() : 1;
	nb_samples = audio_params_.channelCount() * sample_count_per_channel_;

	data_.resize(nb_channels);
	for (int i=0; i<nb_channels; i++)
		data_[i].resize(nb_samples);
}


void SampleBuffer::destroy(void) {
	data_.clear();
}


//void SampleBuffer::set(const float **data) {
//	int nb_samples;
//	int nb_channels;
//
//	nb_channels = 1;
//	nb_samples = audio_params_.channelCount() * sample_count_per_channel_;
//
//	if (!isAllocated()) {
//		log_warn("Tried to fill an unallocated sample buffer");
//		return;
//	}
//
//	for (int i=0; i<nb_channels; i++)
//		memcpy(data_[i].data(), data[i], sizeof(float) * nb_samples);
//}


size_t SampleBuffer::size(void) const {
	int nb_samples;
	int nb_channels;

	nb_channels = !isPlanar() ? audio_params_.channelCount() : 1;
	nb_samples = audio_params_.channelCount() * sample_count_per_channel_;

	return audioParams().samplesToBytes(nb_samples) / nb_channels;
}


void SampleBuffer::save(void) {
	int nb_samples;
	int nb_channels;

	nb_channels = !isPlanar() ? audio_params_.channelCount() : 1;
	nb_samples = audio_params_.channelCount() * sample_count_per_channel_;

	// TO DEBUG
	FILE *fp = fopen("audio.wav", "a+");

	for (int i=0; i<nb_channels; i++)
		fwrite(data_[i].data(), 1, 2 *  nb_samples, fp);

	fclose(fp);
}


