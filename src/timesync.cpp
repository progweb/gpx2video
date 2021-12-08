#include <byteswap.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

#include "log.h"
#include "timesync.h"


TimeSync::TimeSync(GPX2Video &app) 
	: Task(app) 
	, app_(app) {
	container_ = NULL;
	offset_ = 0;
}


TimeSync::~TimeSync() {
}


TimeSync * TimeSync::create(GPX2Video &app) {
	TimeSync *timesync = new TimeSync(app);

	timesync->init();

	return timesync;
}


void TimeSync::init(void) {
	log_call();

	// Media
	container_ = app_.media();
}


void TimeSync::run(void) {
	int result;

	int n = 0;
	int offset = 0;
	int start_time = 0;

	bool eof = false;

    AVPacket *packet = NULL;
	AVStream *avstream = NULL;
	AVFormatContext *fmt_ctx = NULL;

	log_call();

	log_notice("Time synchronization...");

	StreamPtr stream = container_->getDataStream("GoPro MET");

	if (stream == NULL) {
		log_error("No GPS data stream found");
		goto done;
	};

	// Try to open
	if (avformat_open_input(&fmt_ctx, stream->container()->filename().c_str(), NULL, NULL) < 0) {
		av_log(NULL, AV_LOG_ERROR, "Cannot open input file '%s'", stream->container()->filename().c_str());
		goto done;
	}

	// Get stream information from format
	if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
		av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
		goto done;
	}

	// Get reference to correct AVStream
	avstream = fmt_ctx->streams[stream->index()];

	if (!(packet = av_packet_alloc()))
		goto done;

	// Get start time from metadata
	start_time = container_->startTime();

	while (!eof) {
		GPMD gpmd;

		do {
			// Free buffer in packet if there is one
			av_packet_unref(packet);
		
			result = av_read_frame(fmt_ctx, packet);
		} while (packet->stream_index != avstream->index && result >= 0);

		if (result == AVERROR_EOF) {
			// Don't break so that receive gets called again, but don't try to read again
			eof = true;
		}
		else if (result < 0) {
			// Handle other error by breaking loop and returning the code we received
			break;
		}
		else {
        	av_log(NULL, AV_LOG_DEBUG, "Extract GPX from stream %u\n", packet->stream_index);

			// Stream timestamp
			int64_t timecode = packet->pts;
			int64_t timecode_ms = timecode * av_q2d(avstream->time_base) * 1000;

			// Parsing stream packet
			parse(gpmd, packet->data, packet->size);

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
			printf("PACKET: %d - PTS: %ld - TIMESTAMP: %ld ms - TIME: %s - GPS TIME: %s - OFFSET: %d\n", 
				n, timecode, timecode_ms, s, gpmd.date.c_str(), offset);

			// We don't needd the packet anymore, so free it
			av_packet_unref(packet);

			n++;

			if (result < 0)
				break;
		}
	}

	offset_ = offset;

	container_->setTimeOffset(offset);

done:
	if (fmt_ctx)
		avformat_close_input(&fmt_ctx);

	complete();
}


void TimeSync::parse(GPMD &gpmd, uint8_t *buffer, size_t size) {
	int i;

	size_t n;
	size_t len;

	int remain;

	char string[128];

	struct gpmd_data *data;

	for (n=0; n<size; n=n+sizeof(data->header)) {
		data = (struct gpmd_data *) (buffer + n);

		data->header.count = bswap_16(data->header.count);

//		printf("%c%c%c%c %c 0x%X %u %u\n", 
//				data->header.label[0], data->header.label[1], data->header.label[2], data->header.label[3], 
//				data->header.type, data->header.type,
//				data->header.size, data->header.count);

		len = data->header.count * data->header.size;

		switch (data->header.type) {
		case 0x00:
			continue;
			break;

		case 'c': // 0x63
			memcpy(string, data->value.string, len);
			string[len] = '\0';
//			printf("  value: %s\n", string);
			break;

		case 'L': // 0x4c
			for (i=0; i<data->header.count; i++) {
				data->value.u32[i] = __bswap_32(data->value.u32[i]);
//				printf("  value[%d]: %u\n", i, data->value.u32[i]);
			}
			break;

		case 'S': // 0x53
			for (i=0; i<data->header.count; i++) {
				data->value.u16[i] = bswap_16(data->value.u16[i]);
//				printf("  value[%d]: %u\n", i, data->value.u16[i]);
			}
			break;

		case 's': // 0x73
			for (i=0; i<data->header.count; i++) {
				data->value.s16[i] = bswap_16(data->value.s16[i]);
//				printf("  value: %d\n", data->value.s16[i]);
			}
			break;

		case 'f': // 0x66
			for (i=0; i<data->header.count; i++) {
				data->value.real[i] = __bswap_32(data->value.real[i]);
//				printf("  value: %f\n", data->value.real[i]);
			}
			break;

		case 'U': { // 0x55 16 bytes
				char *bytes = data->value.string;

				// 2020-12-13T09:56:27.000000Z
				// buffer contains: 201213085548.215
				// so format to : YY:MM:DD HH:MM:SS.
				sprintf(string, "20%c%c-%c%c-%c%c %c%c:%c%c:%c%c.%c%c%c",
						bytes[0], bytes[1], // YY
						bytes[2], bytes[3], // MM 
						bytes[4], bytes[5], // DD
						bytes[6], bytes[7], // HH
						bytes[8], bytes[9], // MM
						bytes[10], bytes[11], // SS
						bytes[13], bytes[14], bytes[15] // SS
					);

				gpmd.date = string;
//				printf("  value: %s\n", string);
			}
			break;

		default:
			break;
		}

		n += len;
		remain = len % 4;
		if (remain > 0)
			n += 4 - remain;
	}
}

