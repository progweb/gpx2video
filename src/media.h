#ifndef __GPX2VIDEO__MEDIA_H__
#define __GPX2VIDEO__MEDIA_H__

#include <time.h>

#include <string>
#include <vector>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include "stream.h"


class MediaContainer {
public:
	MediaContainer();
	virtual ~MediaContainer();

	const std::string& filename(void) const;
	void setFilename(const std::string &filename);

	uint64_t creationTime(void) const;
	void setCreationTime(const uint64_t &creation_time);
	void setCreationTime(const std::string &creation_time);

	uint64_t startTime(void) const;
	void setStartTime(const uint64_t &start_time);
	void setStartTime(const std::string &start_time);

	int timeOffset(void) const;
	void setTimeOffset(const int& offset);

	double maxFrameDuration(void) const;
	void setMaxFrameDuration(double duration);

	void addStream(StreamPtr stream);

	StreamPtr getFirstStreamOfType(const AVMediaType &type) const;
	StreamPtr getDataStream(const std::string &name) const;

	AudioStreamPtr getAudioStream(void);
	VideoStreamPtr getVideoStream(void);

private:
	int offset_;

	uint64_t start_time_;
	uint64_t creation_time_;

	double max_frame_duration_;

	std::string filename_;

	std::vector<StreamPtr> streams_;
};


#endif

