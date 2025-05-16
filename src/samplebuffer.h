#ifndef __GPX2VIDEO__SAMPLEBUFFER_H__
#define __GPX2VIDEO__SAMPLEBUFFER_H__

#include <memory>
#include <vector>


class SampleBuffer;

using SampleBufferPtr = std::shared_ptr<SampleBuffer>;

class SampleBuffer {
public:
	SampleBuffer();
	virtual ~SampleBuffer();

	static SampleBufferPtr create(void);

	const AudioParams& audioParams() const;
	void setAudioParams(const AudioParams &audio_params);

	uint64_t time(void) const;

	const uint64_t& timestamp() const;
	void setTimestamp(const uint64_t &timestamp);

	const int& sampleCount(void) const;
	void setSampleCount(const int &sample_count);

//	float * data(int channel) {
//		return data_[channel].data();
//	}
//
//	const float * data(int channel) const {
//		return data_.at(channel).data();
//	}

	std::vector<float *> toRawPtrs(void) {
		raw_data_.resize(data_.size());

		for (size_t i=0; i<raw_data_.size(); i++)
			raw_data_[i] = data_[i].data();

		return raw_data_;
	}

	int channelCount(void) const;

	bool isPlanar(void) const;
	bool isAllocated(void) const;

	void allocate(void);
	void destroy(void);

	size_t size(void) const;

//	void set(const float **data);

	void save(void);

private:
	AudioParams audio_params_;

	uint64_t pts_;

	int sample_count_per_channel_;

	// Channel vector
	std::vector<float *> raw_data_;
	// Sample vector per channel
	std::vector<std::vector<float>> data_;
};

#endif

