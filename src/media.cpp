#include <iostream>
#include <memory>

#include "media.h"


MediaContainer::~MediaContainer() {
}


const std::string& MediaContainer::filename(void) const {
	return filename_;
}


void MediaContainer::setFilename(const std::string &filename) {
	filename_ = filename;
}


void MediaContainer::addStream(StreamPtr stream) {
	stream->setContainer(this);

	streams_.push_back(stream);
}


StreamPtr MediaContainer::getFirstStreamOfType(const AVMediaType &type) const {
	for (StreamPtr stream : streams_) {
		if (stream->type() == type)
			return stream;
	}

	return NULL;
}


AudioStreamPtr MediaContainer::getAudioStream(void) {
	AudioStreamPtr stream = std::static_pointer_cast<AudioStream>(getFirstStreamOfType(AVMEDIA_TYPE_AUDIO));

	return stream;
}


VideoStreamPtr MediaContainer::getVideoStream(void) {
	VideoStreamPtr stream = std::static_pointer_cast<VideoStream>(getFirstStreamOfType(AVMEDIA_TYPE_VIDEO));

	return stream;
}


