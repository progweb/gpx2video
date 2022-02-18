#include <byteswap.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

#include "log.h"
#include "timesync.h"



TimeSync::TimeSync(GPX2Video &app, const ExtractorSettings &settings) 
	: Extractor(app, settings) {
}


TimeSync::~TimeSync() {
}


TimeSync * TimeSync::create(GPX2Video &app) {
	ExtractorSettings extractorSettings;
	extractorSettings.setFormat(ExtractorSettings::FormatNone);

	TimeSync *timesync = new TimeSync(app, extractorSettings);

	timesync->init();

	return timesync;
}


void TimeSync::run(void) {
	int result;

	int n = 0;
	int offset = 0;
	int start_time = 0;

    AVPacket *packet = NULL;

	log_call();

	log_notice("Time synchronization...");

	// Open output stream
    std::ofstream out = std::ofstream("/dev/null");
       
	if (!out.is_open()) {
		log_error("Open '/dev/null' failure");
		goto done;
	}

	// Open GoPro MET stream
	if (open() != true) {
		log_warn("Time synchronization failure!");
		goto done;
	}

	if (!(packet = av_packet_alloc()))
		goto done;

	// Get start time from metadata
	start_time = container_->startTime();

	while (true) {
		TimeSync::GPMD gpmd;

		// Pull from stream
		result = getPacket(packet);

		// Handle any errors that aren't EOF (EOF is handled later on)
		if ((result < 0) && (result != AVERROR_EOF)) {
			break;
		}

		if (result == AVERROR_EOF) {
			break;
		}

		// Stream timestamp
		int64_t timecode = packet->pts;
		int64_t timecode_ms = timecode * av_q2d(avstream_->time_base) * 1000;

		// Parsing stream packet
		parse(gpmd, packet->data, packet->size, out);

		// Camera time
		char s[128];

		struct tm camera_time;

		const time_t camera_t = start_time + (timecode_ms / 1000);

		gmtime_r(&camera_t, &camera_time);

		strftime(s, sizeof(s), "%Y-%m-%d %H:%M:%S", &camera_time);

		// GPS time - format = 2021-12-08 08:55:46.039
		struct tm gps_time;

		const char *str = gpmd.date.c_str();

		memset(&gps_time, 0, sizeof(gps_time));
		strptime(str, "%Y-%m-%d %H:%M:%S.", &gps_time);

		const time_t gps_t = timegm(&gps_time);

		// Offset in seconds
		offset = gps_t - camera_t;

		// Dump
		printf("PACKET: %d - PTS: %ld - TIMESTAMP: %ld ms - TIME: %s - GPS FIX: %d - GPS TIME: %s - OFFSET: %d\n", 
			n, timecode, timecode_ms, s, gpmd.fix, gpmd.date.c_str(), offset);

		// We don't needd the packet anymore, so free it
		av_packet_unref(packet);

		n++;

		// Fix ?
		if (gpmd.fix > 1) {
			log_notice("Video stream synchronized with success");
			break;
		}

		if (result < 0)
			break;
	}

done:
	// Apply offset
	container_->setTimeOffset(offset);

	// Save offset value
	offset_ = offset;

	close();

	complete();
}

