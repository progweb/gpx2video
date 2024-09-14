#include <byteswap.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

#include "log.h"
#include "timesync.h"



TimeSync::TimeSync(GPXApplication &app, const ExtractorSettings &settings) 
	: Extractor(app, settings) {
}


TimeSync::~TimeSync() {
}


TimeSync * TimeSync::create(GPXApplication &app, MediaContainer *container) {
	ExtractorSettings extractorSettings;
	extractorSettings.setFormat(ExtractorSettings::FormatNone);

	TimeSync *timesync = new TimeSync(app, extractorSettings);

	timesync->init(container);

	return timesync;
}


bool TimeSync::start(void) {
	bool result = false;

	log_call();

	n_ = 0;
	ok_ = false;
	offset_ = 0;

	log_notice("Time synchronization...");

	// Open output stream
    out_ = std::ofstream("/dev/null");
       
	if (!out_.is_open()) {
		log_error("Open '/dev/null' failure");
		goto done;
	}

	// Open GoPro MET stream
	if (open() != true) {
		log_warn("Time synchronization failure!");
		goto done;
	}

	result = true;

done:
	return result;
}


bool TimeSync::run(void) {
	char s[128];

	int result;

	int offset = 0;
	int start_time = 0;

	time_t gps_t;
	struct tm gps_time;

	time_t camera_t;
	struct tm camera_time;

	const char *str;

	int64_t timecode, timecode_ms;

    AVPacket *packet = NULL;

	TimeSync::GPMD gpmd;

	log_call();

	// Get start time from metadata
	start_time = container_->startTime();

	if (!(packet = av_packet_alloc()))
		goto done;

	// Pull from stream
	result = getPacket(packet);

	// Handle any errors that aren't EOF (EOF is handled later on)
	if ((result < 0) && (result != AVERROR_EOF))
		goto done;

	if (result == AVERROR_EOF)
		goto done;

	// Stream timestamp
	timecode = packet->pts;
	timecode_ms = timecode * av_q2d(avstream_->time_base) * 1000;

	// Parsing stream packet
	parse(gpmd, packet->data, packet->size, out_);

	// Camera time
	camera_t = start_time + (timecode_ms / 1000);

	gmtime_r(&camera_t, &camera_time);

	strftime(s, sizeof(s), "%Y-%m-%d %H:%M:%S", &camera_time);

	// GPS time - format = 2021-12-08 08:55:46.039
	str = gpmd.date.c_str();

	memset(&gps_time, 0, sizeof(gps_time));
	strptime(str, "%Y-%m-%d %H:%M:%S.", &gps_time);

	gps_t = timegm(&gps_time);

	// Offset in seconds
	offset = gps_t - camera_t;

	// Dump
	if (app_.progressInfo()) {
		printf("PACKET: %d - PTS: %ld - TIMESTAMP: %ld ms - TIME: %s - GPS FIX: %d - GPS TIME: %s - OFFSET: %d\n", 
			n_, timecode, timecode_ms, s, gpmd.fix, gpmd.date.c_str(), offset);
	}

	n_++;

	// We don't needd the packet anymore, so free it
	av_packet_unref(packet);

	packet = NULL;

	// Fix ?
	if (gpmd.fix > 1) {
		ok_ = true;
		offset_ = offset;
		goto done;
	}

	if (result < 0)
		goto done;

	schedule();

	return true;

done:
	if (packet != NULL)
		av_packet_unref(packet);

	complete();

	return true;
}


bool TimeSync::stop(void) {
	if (ok_) {
		log_notice("Video stream synchronized with success (offset: %d s)", offset_);
	
		// Apply offset
		container_->setTimeOffset(offset_);
	}

	close();

	return true;
}

