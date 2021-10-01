#ifndef __GPX2VIDEO__MEDIA_H__
#define __GPX2VIDEO__MEDIA_H__

#include <string>
#include <vector>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include "stream.h"


class MediaContainer {
public:
	virtual ~MediaContainer();

	const std::string& filename(void) const;
	void setFilename(const std::string &filename);

	void addStream(StreamPtr stream);

	StreamPtr getFirstStreamOfType(const AVMediaType &type) const;

	AudioStreamPtr getAudioStream(void);
	VideoStreamPtr getVideoStream(void);

private:
	std::string filename_;

	std::vector<StreamPtr> streams_;
};


#endif

