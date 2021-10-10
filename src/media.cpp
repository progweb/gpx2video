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


time_t MediaContainer::startTime(void) const {
	return start_time_;
}


/**
 * GoPro creation_time metadata format:
 *   2021-10-03T19:12:01.000000Z
 * creation_time is in local time
 */
void MediaContainer::setStartTime(const std::string &start_time) {
	struct tm time;

	const char *s = start_time.c_str();

	// creation_time = 2020-12-13T09:56:27.000000Z
	memset(&time, 0, sizeof(time));
	strptime(s, "%Y-%m-%dT%H:%M:%S.", &time);

	// Convert GoPro time in UTC time
	start_time_ = timelocal(&time);
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


