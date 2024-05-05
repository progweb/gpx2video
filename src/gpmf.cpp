#include <iostream>
#include <memory>
#include <string>
#include <algorithm>

#include <byteswap.h>

#include "log.h"
#include "gpmf.h"


GPMFDecoder::GPMFDecoder()
	: fmt_ctx_(NULL) {
	n_ = -1;
	pts_ = 0;
}


GPMFDecoder::~GPMFDecoder() {
	close();
}


GPMFDecoder * GPMFDecoder::create(void) {
	GPMFDecoder *decoder = new GPMFDecoder();

	return decoder;
}


bool GPMFDecoder::open(StreamPtr stream) {
	bool result;

	// Set stream
	stream_ = stream;

	// Try to open
	if ((result = open(stream->container()->filename(), stream->index())) == false)
		return false;

	return true;
}


bool GPMFDecoder::open(const std::string &filename, const int &index) {
	int result;

	if ((result = avformat_open_input(&fmt_ctx_, filename.c_str(), NULL, NULL)) < 0) {
		av_log(NULL, AV_LOG_ERROR, "Cannot open input file '%s'", filename.c_str());
		return false;
	}

	// Get stream information from format
	if ((result = avformat_find_stream_info(fmt_ctx_, NULL)) < 0) {
		av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
		return false;
	}

	// Get reference to correct AVStream
	avstream_ = fmt_ctx_->streams[index];

	// Get first packet
	AVRational null = av_make_q(0, 1);

	retrieveData(next_data_, null);

	return true;
}


int GPMFDecoder::getPacket(AVPacket *packet) {
	int result = -1;

	bool eof = false;

	while (!eof) {
		do {
			// Free buffer in packet if there is one
			av_packet_unref(packet);
		
			result = av_read_frame(fmt_ctx_, packet);
		} while (packet->stream_index != avstream_->index && result >= 0);

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

			break;
		}
	}

	return result;
}


void GPMFDecoder::close(void) {
	if (fmt_ctx_) {
		avformat_close_input(&fmt_ctx_);
		fmt_ctx_ = NULL;
	}
}


bool GPMFDecoder::retrieveData(GPMFData &data, AVRational timecode) {
	bool eof;

	GPMFData last;

	uint64_t difftime;
	uint64_t timeelapsed;

    AVPacket *packet = NULL;
 
	int64_t target_ts = stream_->getTimeInTimeBaseUnits(timecode);

	// Retrieve packet data
	packet = retrievePacketData(target_ts, eof);

	if (!eof && (packet == NULL))
		return false;

	// Return previous data
	data = next_data_;

	if (packet == NULL)
		goto done;

	// Parse GPMF next data
	parseData(next_data_, packet->data, packet->size);

	// Add packet info
	next_data_.id = n_;
	next_data_.pts = packet->pts;
	next_data_.timecode = packet->pts * av_q2d(avstream_->time_base) * 1000;

	difftime = next_data_.timecode - data.timecode;
	timeelapsed = next_data_.timestamp - data.timestamp;

	// Compute timelapse value
	data.timelapse = (difftime != 0) ? (double) timeelapsed / (1000.0 * difftime) : 1.0;

	// Assume the time factor is constant (will be computed the next call)
	next_data_.timelapse = data.timelapse;

	// We don't needd the packet anymore, so free it
	av_packet_unref(packet);

done:
	return true;
}


AVPacket * GPMFDecoder::retrievePacketData(const int64_t& target_ts, bool &eof) {
	int result;

	AVPacket *packet = NULL;

	eof = false;

	// Check PTS
	if ((1000 * pts_) > target_ts)
		return NULL;

	packet = av_packet_alloc();

	while (true) {
		// Read raw data
		result = getPacket(packet);

		// Handle any errors that aren't EOF (EOF is handled later on)
		if ((result < 0) && (result != AVERROR_EOF)) {
			goto abort;
		}

		if (result == AVERROR_EOF) {
			eof = true;
			goto abort;
		}

		// Store data
//		data = (uint8_t *) frame;

		n_++;
		pts_ = packet->pts;

		break;
	}

	return packet;

abort:
	if (packet != NULL)
		av_packet_unref(packet);

	return NULL;
}


bool GPMFDecoder::parseData(GPMFData &gpmd, uint8_t *buffer, size_t size) {
	int i, j, k;
	int nstream = 0;
	int inputtypesize;

	size_t n;
	size_t len;

	int remain;

	uint32_t key;
	char string[128];

	uint32_t scal[5] = { 1, 1, 1, 1, 1 };

	struct gpmd_data *data;

#define MAKEKEY(label)		((uint32_t *) &label)[0]
#define STR2FOURCC(s)		((s[0]<<0)|(s[1]<<8)|(s[2]<<16)|(s[3]<<24))

	for (n=0; n<size; n=n+sizeof(data->header)) {
		data = (struct gpmd_data *) (buffer + n);

		data->header.count = bswap_16(data->header.count);

		inputtypesize = 4;
		len = data->header.count * data->header.size;

		key = MAKEKEY(data->header);

		if (key == STR2FOURCC("STRM"))
			nstream = 1;

		// Parse data
		switch (data->header.type) {
		case 0x00:
			break;

		case GPMFDecoder::GPMF_TYPE_STRING_ASCII: // 0x63
			memcpy(string, data->value.string, len);
			string[len] = '\0';
			break;

		case GPMFDecoder::GPMF_TYPE_SIGNED_SHORT: // 0x73
			for (i=0; i<data->header.count; i++)
				data->value.s16[i] = bswap_16(data->value.s16[i]);
			break;

		case GPMFDecoder::GPMF_TYPE_UNSIGNED_SHORT: // 0x53
			for (i=0; i<data->header.count; i++)
				data->value.u16[i] = bswap_16(data->value.u16[i]);
			break;

		case GPMFDecoder::GPMF_TYPE_FLOAT: // 0x66
			for (i=0; i<data->header.count; i++)
				data->value.real[i] = __bswap_32(data->value.real[i]);
			break;

		case GPMFDecoder::GPMF_TYPE_SIGNED_LONG: // 0x6c
			inputtypesize = 4;
			for (i=0, k=0; i<data->header.count; i++) {
				for (j=0; j<data->header.size / inputtypesize; j++)
					data->value.s32[k+j] = __bswap_32(data->value.s32[k+j]);
				k += data->header.size / inputtypesize;
			}
			break;

		case GPMFDecoder::GPMF_TYPE_UNSIGNED_LONG: // 0x4c
			inputtypesize = 4;
			for (i=0, k=0; i<data->header.count; i++) {
				for (j=0; j<data->header.size / inputtypesize; j++)
					data->value.u32[k+j] = __bswap_32(data->value.u32[k+j]);
				k += data->header.size / inputtypesize;
			}
			break;

		case GPMFDecoder::GPMF_TYPE_UNSIGNED_64BIT_INT: // 0x4a
			inputtypesize = 8;
			for (i=0; i<data->header.count; i++)
				data->value.u64[i] = __bswap_64(data->value.u64[i]);
			break;

		case GPMFDecoder::GPMF_TYPE_UTC_DATE_TIME: { // 0x55 16 bytes
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
						bytes[13], bytes[14], bytes[15] // MS
					);

				// GPS time - format = 2021-12-08 08:55:46.039
				memset(&gpmd.utc_time, 0, sizeof(gpmd.utc_time));
				strptime(string, "%Y-%m-%d %H:%M:%S.", &gpmd.utc_time);
				gpmd.utc_ms = (bytes[13] - '0') * 100 + (bytes[14] - '0') * 10 + (bytes[15] - '0');
			}
			break;

		case GPMFDecoder::GPMF_TYPE_DOUBLE:
			for (i=0; i<data->header.count; i++)
				data->value.u64[i] = __bswap_64(data->value.u64[i]);
			break;

		default:
			break;
		}

		// Save data
		if (key == STR2FOURCC("STRM"))
			nstream++;
		else if (key == STR2FOURCC("DVNM")) {
			gpmd.device_name = string;
		}
		else if (key == STR2FOURCC("STMP")) {
			if ((data->header.type == 'J') && data->header.count) {
				gpmd.timestamp = data->value.u64[0];
			}
		}
		else if (key == STR2FOURCC("GPSF")) {
			if ((data->header.type == 'L') && data->header.count) {
				gpmd.fix = data->value.u32[0];
			}
		}
		else if (key == STR2FOURCC("GPSU")) {
			gpmd.date = string;
		}
		else if (key == STR2FOURCC("SCAL")) {
			for (i=0, k=0; i<data->header.count; i++) {
				if (k < (int) sizeof(scal))
					scal[k] = data->value.u32[k];

				k += data->header.size / inputtypesize;
			}
		}
		else if (key == STR2FOURCC("GPS5")) {
			for (i=0, k=0; i<data->header.count; i++) {
				// GPS point freq: 18 Hz
				// 1st point match on the GPSU value
				if (k == 0) {
					gpmd.lat = (double) data->value.s32[k] / (double) scal[0];
					gpmd.lon = (double) data->value.s32[k+1] / (double) scal[1];
					gpmd.ele = (double) data->value.s32[k+2] / (double) scal[2];
				}

				k += data->header.size / inputtypesize;
			}
		}

		if (data->header.type != 0x00) { 
			n += len;
			remain = len % 4;
			if (remain > 0)
				n += 4 - remain;
		}
	}

	return true;
}

