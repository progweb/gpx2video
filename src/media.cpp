#include <iostream>
#include <memory>

#include "log.h"
#include "media.h"
#include "datetime.h"


MediaContainer::MediaContainer() 
	: offset_(0) 
	, start_time_(0) {
}


MediaContainer::~MediaContainer() {
}


const std::string& MediaContainer::filename(void) const {
	return filename_;
}


void MediaContainer::setFilename(const std::string &filename) {
	filename_ = filename;
}


uint64_t MediaContainer::startTime(void) const {
	return start_time_;
}


void MediaContainer::setStartTime(const uint64_t &start_time) {
	start_time_ = start_time;
}


/**
 * GoPro creation_time metadata format:
 *   2021-10-03T19:12:01.000000Z
 * creation_time is in local time
 */
void MediaContainer::setStartTime(const std::string &start_time) {
	start_time_ = ::string2timestamp(start_time);
}


int MediaContainer::timeOffset(void) const {
	return offset_;
}


void MediaContainer::setTimeOffset(const int& offset) {
	offset_ = offset;
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


StreamPtr MediaContainer::getDataStream(const std::string &name) const {
	(void) name;

	for (StreamPtr stream : streams_) {
		if (stream->name().find(name) != std::string::npos)
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


