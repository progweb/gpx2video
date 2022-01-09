#include <string>

#include <byteswap.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

#include "log.h"
#include "extractor.h"


ExtractorSettings::ExtractorSettings() {
}


ExtractorSettings::~ExtractorSettings() {
}


const ExtractorSettings::Format& ExtractorSettings::format(void) const {
	return format_;
}


void ExtractorSettings::setFormat(const ExtractorSettings::Format &format) {
	format_ = format;
}


const std::string ExtractorSettings::getFriendlyName(const ExtractorSettings::Format &format) {
	switch (format) {
	case FormatDump:
		return "Dump GPMD data to text";
	case FormatRAW:
		return "Dump GoPro GPMD stream to binary data";
	case FormatGPX:
		return "Extract GPS data from GoPro GPMD stream";
	case FormatCount:
	default:
		return "";
	}

	return "";
}


Extractor::Extractor(GPX2Video &app, const ExtractorSettings &settings) 
	: Task(app) 
	, app_(app) 
	, settings_(settings) {
	container_ = NULL;
}


Extractor::~Extractor() {
}


const ExtractorSettings& Extractor::settings() const {
	log_call();

	return settings_;
}


Extractor * Extractor::create(GPX2Video &app, const ExtractorSettings &settings) {
	Extractor *extractor = new Extractor(app, settings);

	extractor->init();

	return extractor;
}


void Extractor::init(void) {
	log_call();

	// Media
	container_ = app_.media();
}


void Extractor::run(void) {
	int result;

	int n = 0;
	int start_time = 0;

	bool eof = false;

    AVPacket *packet = NULL;
	AVStream *avstream = NULL;
	AVFormatContext *fmt_ctx = NULL;

	std::string filename = app_.settings().outputfile();

	StreamPtr stream;

	log_call();

	// Open output stream
    std::ofstream out = std::ofstream(filename);
       
	if (!out.is_open()) {
		log_error("Open '%s' failure", filename.c_str());
		goto done;
	}

	// Open GPMD stream
	log_notice("Extract GPMD data...");

	stream = container_->getDataStream("GoPro MET");

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

	// Write GPX header
	if (settings().format() == ExtractorSettings::FormatGPX) {
		out << "<gpx version=\"1.0\"" << std::endl;
		out << "  xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"" << std::endl;
		out << "  xmlns=\"http://www.topografix.com/GPX/1/0\"" << std::endl;
		out << "  xsi:schemaLocation=\"http://www.topografix.com/GPX/1/0 http://www.topografix.com/GPX/1/0/gpx.xsd\">" << std::endl;
		out << "  <trk>" << std::endl;
		out << "    <number>1</number>" << std::endl;
		out << "    <trkseg>" << std::endl;
	}

	while (!eof) {
		Extractor::GPMD gpmd;

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

			// Camera time
			char s[128];

			struct tm camera_time;

			const time_t camera_t = start_time + (timecode_ms / 1000);

			gmtime_r(&camera_t, &camera_time);

			strftime(s, sizeof(s), "%Y-%m-%d %H:%M:%S", &camera_time);

			// Dump
			printf("PACKET: %d - PTS: %ld - TIMESTAMP: %ld ms - TIME: %s\n", 
				n, timecode, timecode_ms, s);

			if (settings().format() == ExtractorSettings::FormatRAW) {
				out.write(reinterpret_cast<char*>(packet->data), packet->size); //reinterpret_cast<char*>(&myuint), sizeof(myuint));
			}
			else {
				// Parsing stream packet
				parse(gpmd, packet->data, packet->size, out); //(settings().format() == ExtractorSettings::FormatDump));

				// <trkpt lat="42.4586662211" lon="2.017777">
				//	 <ele>31.4122</ele>
				//   <time>2021-10-10T06:14:52.000Z</time>
				// </trkpt>
				if (settings().format() == ExtractorSettings::FormatGPX) {
					if (gpmd.fix > 0) {
						strftime(s, sizeof(s), "%Y-%m-%dT%H:%M:%S.000Z", &gpmd.utc_time);

						out << std::setprecision(8);
						out << "      <trkpt lat=\"" << gpmd.lat << "\" lon=\"" << gpmd.lon << "\">" << std::endl;
						out << "        <ele>" << gpmd.ele << "</ele>" << std::endl;
						out << "        <time>" << s << "</time>" << std::endl;
						out << "      </trkpt>" << std::endl;
					}
				}
			}

			// We don't needd the packet anymore, so free it
			av_packet_unref(packet);

			n++;

			if (result < 0)
				break;
		}
	}

	// Write GPX footer
	if (settings().format() == ExtractorSettings::FormatGPX) {
		out << "    </trkseg>" << std::endl;
		out << "  </trk>" << std::endl;
		out << "</gpx>" << std::endl;
	}

done:
	if (fmt_ctx)
		avformat_close_input(&fmt_ctx);

	if (out.is_open())
		out.close();

	complete();
}


void Extractor::parse(Extractor::GPMD &gpmd, uint8_t *buffer, size_t size, std::ofstream &out) {
	int i, j, k;
	int inputtypesize;

	size_t n;
	size_t len;

	int remain;

	uint32_t key;
	char string[128];

	uint32_t scal[5] = { 1, 1, 1, 1, 1 };

	struct gpmd_data *data;

	bool dump = (settings().format() == ExtractorSettings::FormatDump);

#define MAKEKEY(label)		((uint32_t *) &label)[0]
#define STR2FOURCC(s)		((s[0]<<0)|(s[1]<<8)|(s[2]<<16)|(s[3]<<24))

	for (n=0; n<size; n=n+sizeof(data->header)) {
		data = (struct gpmd_data *) (buffer + n);

		data->header.count = bswap_16(data->header.count);

		if (dump) {
			snprintf(string, sizeof(string), "%c%c%c%c %c 0x%X %u %u", 
					data->header.label[0], data->header.label[1], data->header.label[2], data->header.label[3], 
					data->header.type, data->header.type,
					data->header.size, data->header.count);
			out << string << std::endl;
		}

		len = data->header.count * data->header.size;

		key = MAKEKEY(data->header);

		// Parse data
		switch (data->header.type) {
		case 0x00:
			continue;
			break;

		case Extractor::GPMF_TYPE_STRING_ASCII: // 0x63
			memcpy(string, data->value.string, len);
			string[len] = '\0';

			if (dump)
				out << "  value: " << string << std::endl;
			break;

		case Extractor::GPMF_TYPE_SIGNED_SHORT: // 0x73
			for (i=0; i<data->header.count; i++) {
				data->value.s16[i] = bswap_16(data->value.s16[i]);

				if (dump)
					out << "  value: " << data->value.s16[i] << std::endl;
			}
			break;

		case Extractor::GPMF_TYPE_UNSIGNED_SHORT: // 0x53
			for (i=0; i<data->header.count; i++) {
				data->value.u16[i] = bswap_16(data->value.u16[i]);

				if (dump)
					out << "  value[" << i << "]: " << data->value.u16[i] << std::endl;
			}
			break;

		case Extractor::GPMF_TYPE_FLOAT: // 0x66
			for (i=0; i<data->header.count; i++) {
				data->value.real[i] = __bswap_32(data->value.real[i]);

				if (dump)
					out << "  value: " << data->value.real[i] << std::endl;
			}
			break;

		case Extractor::GPMF_TYPE_SIGNED_LONG: // 0x4c
			inputtypesize = 4;
			for (i=0, k=0; i<data->header.count; i++) {
				for (j=0; j<data->header.size / inputtypesize; j++) {
					data->value.s32[k+j] = __bswap_32(data->value.s32[k+j]);

					if (dump)
						out << "  value[" << k+j << "]: " << data->value.s32[k+j] << std::endl;
				}
				k += data->header.size / inputtypesize;
			}
			break;

		case Extractor::GPMF_TYPE_UNSIGNED_LONG: // 0x4c
			inputtypesize = 4;
			for (i=0, k=0; i<data->header.count; i++) {
				for (j=0; j<data->header.size / inputtypesize; j++) {
					data->value.u32[k+j] = __bswap_32(data->value.u32[k+j]);

					if (dump)
						out << "  value[" << k+j << "]: " << data->value.u32[k+j] << std::endl;
				}
				k += data->header.size / inputtypesize;
			}
			break;

		case Extractor::GPMF_TYPE_UTC_DATE_TIME: { // 0x55 16 bytes
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

				// GPS time - format = 2021-12-08 08:55:46.039
				memset(&gpmd.utc_time, 0, sizeof(gpmd.utc_time));
				strptime(string, "%Y-%m-%d %H:%M:%S.", &gpmd.utc_time);

				if (dump)
					out << "  value: " << string << std::endl;
			}
			break;

		case Extractor::GPMF_TYPE_DOUBLE:
			for (i=0; i<data->header.count; i++) {
				data->value.u64[i] = __bswap_64(data->value.u64[i]);

				if (dump)
					out << "  value: " << (double) data->value.u64[i] << std::endl;
			}
			break;

		default:
			break;
		}

		// Save data
		if (key == STR2FOURCC("GPSF")) {
			if ((data->header.type == 'L') && data->header.count) {
				gpmd.fix = __bswap_32(data->value.u32[0]);
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
				gpmd.lat = (double) data->value.u32[k] / (double) scal[0];
				gpmd.lon = (double) data->value.u32[k+1] / (double) scal[1];
				gpmd.ele = (double) data->value.u32[k+2] / (double) scal[2];

				k += data->header.size / inputtypesize;
			}
		}

		n += len;
		remain = len % 4;
		if (remain > 0)
			n += 4 - remain;
	}
}

