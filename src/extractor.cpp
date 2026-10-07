#include <string>
#include <iomanip>

#include <byteswap.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

#include "log_i.h"
#include "utils.h"
#include "datetime.h"
#include "extractor.h"


struct Formatter {
	int width;
	friend std::ostream& operator<<(std::ostream& os, const Formatter& fmt) {
		os << std::setfill(' ') << std::setw(fmt.width);
		return os;
	}
};


// Extractor settings
//--------------------

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
	case FormatNone:
		return "None";
	case FormatRAW:
		return "Dump GoPro GPMD stream to binary data";
	case FormatDump:
		return "Dump GPMD data to text";
	case FormatText:
		return "Dump GPMD data and interpret the content";
	case FormatGPX:
		return "Extract GPS data from GoPro GPMD stream";
	case FormatCount:
	default:
		return "";
	}

	return "";
}


// Extractor API
//---------------

Extractor::Extractor(GPXApplication &app, const ExtractorSettings &settings) 
	: Task(app, "extractor") 
	, app_(app) 
	, settings_(settings) {
	container_ = NULL;

	fmt_ctx_ = NULL;
	avstream_ = NULL;
}


Extractor::~Extractor() {
}


const ExtractorSettings& Extractor::settings() const {
	log_call();

	return settings_;
}


Extractor * Extractor::create(GPXApplication &app, const ExtractorSettings &settings,
		MediaContainer *container) {
	Extractor *extractor = new Extractor(app, settings);

	extractor->init(container);

	return extractor;
}


void Extractor::init(MediaContainer *container) {
	log_call();

	container_ = container;
}


bool Extractor::start(void) {
	bool result = true;

	std::string filename = app_.settings().outputfile();

	log_call();

	// Register task status
	Task::start();

	n_ = 0;
	ok_ = false;

	// Open GPMD stream
	log_notice("Extract GPMD data...");

	// Open output stream
    out_ = std::ofstream(filename);
       
	if (!out_.is_open()) {
		log_error("Open '%s' failure", filename.c_str());
		result = false;
		goto done;
	}

	// Open GoPro MET stream
	if (open() != true) {
		log_warn("Time synchronization failure!");
		result = false;
		goto done;
	}

	// Write GPX header
	if (settings().format() == ExtractorSettings::FormatGPX) {
		out_ << "<?xml version=\"1.0\" encoding=\"utf-8\"?>" << std::endl;
		out_ << "<gpx version=\"1.0\"" << std::endl;
		out_ << "  creator=\"gpx2video\"" << std::endl;
		out_ << "  xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"" << std::endl;
		out_ << "  xmlns=\"http://www.topografix.com/GPX/1/0\"" << std::endl;
		out_ << "  xsi:schemaLocation=\"http://www.topografix.com/GPX/1/0 http://www.topografix.com/GPX/1/0/gpx.xsd\">" << std::endl;
		out_ << "  <trk>" << std::endl;
		out_ << "    <number>1</number>" << std::endl;
		out_ << "    <trkseg>" << std::endl;
	}

done:
	return result;
}


bool Extractor::run(void) {
	int result;

	uint64_t start_time = 0;

	uint64_t camera_time;

	int64_t timecode, timecode_ms;

    AVPacket *packet = NULL;

	Extractor::GPMD gpmd;

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

	if (result == AVERROR_EOF) {
		ok_ = true;
		goto done;
	}

	// Stream timestamp
	timecode = packet->pts;
	timecode_ms = timecode * av_q2d(avstream_->time_base) * 1000;

	// Camera time
	camera_time = start_time + timecode_ms;

	// Dump
	if (app_.progressInfo()) {
		printf("PACKET: %d - PTS: %ld - TIMESTAMP: %ld ms - TIME: %sZ\n", 
			n_, timecode, timecode_ms, Datetime::timestamp2string(camera_time, Datetime::FormatDatetime, true).c_str());
	}

	if (settings().format() == ExtractorSettings::FormatRAW) {
		out_.write(reinterpret_cast<char*>(packet->data), packet->size); //reinterpret_cast<char*>(&myuint), sizeof(myuint));
	}
	else {
		// Append MP4 stream info
		if ((settings().format() == ExtractorSettings::FormatDump) || (settings().format() == ExtractorSettings::FormatText))
			out_ << "PACKET: " << n_ << " - PTS: " << timecode << " TIMESTAMP: " << timecode_ms << " ms" << std::endl;

		// Parsing stream packet
		parse(gpmd, packet->data, packet->size, out_); //(settings().format() == ExtractorSettings::FormatDump));

		// <trkpt lat="42.4586662211" lon="2.017777">
		//	 <ele>31.4122</ele>
		//   <time>2021-10-10T06:14:52.000Z</time>
		// </trkpt>
		if (settings().format() == ExtractorSettings::FormatGPX) {
			if (gpmd.fix > 0) {
				out_ << std::setprecision(9);
				out_ << "      <trkpt lat=\"" << gpmd.lat << "\" lon=\"" << gpmd.lon << "\">" << std::endl;
				out_ << "        <ele>" << gpmd.ele << "</ele>" << std::endl;
				out_ << "        <time>" << gpmd.date << "</time>" << std::endl;
				out_ << "      </trkpt>" << std::endl;
			}
		}
		else if ((settings().format() == ExtractorSettings::FormatDump) || (settings().format() == ExtractorSettings::FormatText))
			out_ << "###############################################" << std::endl;
	}

	n_++;

	// We don't needd the packet anymore, so free it
	av_packet_unref(packet);

	packet = NULL;

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


bool Extractor::stop(void) {
	// Write GPX footer
	if (settings().format() == ExtractorSettings::FormatGPX) {
		out_ << "    </trkseg>" << std::endl;
		out_ << "  </trk>" << std::endl;
		out_ << "</gpx>" << std::endl;
	}

	close();

	if (out_.is_open())
		out_.close();

	if (ok_)
		log_notice("GPMD data extracted with success");
	else
		log_error("GPMD data extraction failure");

	// Register task status
	Task::stop();

	return true;
}


bool Extractor::open(void) {
	bool result = false;

	StreamPtr stream = NULL;

	log_call();

	// Open GoPro MET stream
	stream = container_->getDataStream("GoPro MET");

	if (stream == NULL) {
		log_error("No GPS data stream found");
		goto done;
	};

	// Try to open
	if (avformat_open_input(&fmt_ctx_, stream->container()->filename().c_str(), NULL, NULL) < 0) {
		av_log(NULL, AV_LOG_ERROR, "Cannot open input file '%s'", stream->container()->filename().c_str());
		goto done;
	}

	// Get stream information from format
	if (avformat_find_stream_info(fmt_ctx_, NULL) < 0) {
		av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
		goto done;
	}

	// Get reference to correct AVStream
	avstream_ = fmt_ctx_->streams[stream->index()];

	result = true;

done:
	return result;
}


void Extractor::close(void) {
	log_call();

	if (fmt_ctx_)
		avformat_close_input(&fmt_ctx_);
}


int Extractor::getPacket(AVPacket *packet) {
	int result = -1;

	bool eof = false;

	log_call();

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


double Extractor::compute(struct gpmd_data *data, int index, double scal) {
	switch (data->header.type) {
	case Extractor::GPMF_TYPE_SIGNED_SHORT: // 0x73
		return (double) data->value.s16[index] / (double) scal;
		break;

	case Extractor::GPMF_TYPE_UNSIGNED_SHORT: // 0x53
		return (double) data->value.u16[index] / (double) scal;
		break;

	case Extractor::GPMF_TYPE_SIGNED_LONG: // 0x6c
		return (double) data->value.s32[index] / (double) scal;
		break;

	case Extractor::GPMF_TYPE_UNSIGNED_LONG: // 0x4c
		return (double) data->value.u32[index] / (double) scal;
		break;

	case Extractor::GPMF_TYPE_FLOAT: // 0x66
		return (double) data->value.real[index] / (double) scal;
		break;

	default:
		return 0.0;
	}
}

void Extractor::parse(Extractor::GPMD &gpmd, uint8_t *buffer, size_t size, std::ofstream &out) {
	int i, j, k;
	int nstream = 0;
	int inputtypesize;

	size_t n;
	size_t len;

	int remain;

	uint32_t key;
	char string[128];

	std::vector<std::string> titles;
	std::vector<uint32_t> scales;
	std::vector<std::string> units;

	struct gpmd_data *data;

	bool dumpvalue;

	bool dumpinfo = (settings().format() == ExtractorSettings::FormatText);

	bool dump = (settings().format() == ExtractorSettings::FormatDump) || (settings().format() == ExtractorSettings::FormatText);

#define MAKEKEY(label)		((uint32_t *) &label)[0]
#define STR2FOURCC(s)		((s[0]<<0)|(s[1]<<8)|(s[2]<<16)|(s[3]<<24))

	auto format = [](const int& width) {
		return Formatter{width}; 
	};

	auto title = [&titles](const size_t& index) {
		if (index < titles.size())
			return titles[index] + ": ";
		else
			return std::string();
	};

	auto scale = [&scales](const size_t& index) {
		if (index < scales.size())
			return scales[index];
		else if (!scales.empty())
			return scales.back();
		else
			return (uint32_t) 1;
	};

	auto unit = [&units](const size_t& index) {
		if (index < units.size())
			return std::string(" ") + units[index];
		else if (!units.empty())
			return std::string(" ") + units.back();
		else
			return std::string("");
	};

	for (n=0; n<size; n=n+sizeof(data->header)) {
		data = (struct gpmd_data *) (buffer + n);

		data->header.count = bswap_16(data->header.count);

		inputtypesize = 4;
		len = data->header.count * data->header.size;

		key = MAKEKEY(data->header);

		if (key == STR2FOURCC("STRM")) {
			// No title by default
			titles.clear();

			// No scale by default
			scales.clear();

			// No unit by default
			units.clear();

			if (nstream > 1) {
				Utils::IndentingOStreambuf indent(out, 4 * (nstream - 1));
 
				if (dump)
					out << "]" << std::endl;
			}

			nstream = 1;
		}

		// Indent ?
		Utils::IndentingOStreambuf indent(out, 4 * nstream);

		if (dump) {
			snprintf(string, sizeof(string), "%c%c%c%c %s %c 0x%X %u %u", 
					data->header.label[0], data->header.label[1], data->header.label[2], data->header.label[3], 
					(key == STR2FOURCC("STRM")) ? "[" : "",
					data->header.type, data->header.type,
					data->header.size, data->header.count);
			out << string << std::endl;
		}

		// Doesn't dump values for parsed & known sections
		if (dumpinfo) {
			if (key == STR2FOURCC("GPS5"))
				dumpvalue = false;
			else if ((key == STR2FOURCC("ACCL")) || (key == STR2FOURCC("GYRO")) || (key == STR2FOURCC("MAGN")) || (key == STR2FOURCC("GRAV")))
				dumpvalue = false;
			else if ((key == STR2FOURCC("CORI")) || (key == STR2FOURCC("IORI")))
				dumpvalue = false;
			else if (key == STR2FOURCC("ISOE"))
				dumpvalue = false;
			else if (key == STR2FOURCC("SHUT"))
				dumpvalue = false;
			else
				dumpvalue = dump;
		}
		else
			dumpvalue = dump;

		// Parse data
		switch (data->header.type) {
		case 0x00:
			break;

		case Extractor::GPMF_TYPE_STRING_ASCII: // 0x63
			inputtypesize = 1;
			for (i=0, k=0; i<data->header.count; i++) {
				const char *token = &(data->value.string[k]);

				memcpy(string, token, data->header.size / inputtypesize);
				string[data->header.size / inputtypesize] = '\0';

				if (dumpvalue)
					out << "  value[" << i << "]: " << string << std::endl;

				k += data->header.size / inputtypesize;
			}
			break;

		case Extractor::GPMF_TYPE_SIGNED_SHORT: // 0x73
			inputtypesize = 2;
			for (i=0, k=0; i<data->header.count; i++) {
				for (j=0; j<data->header.size / inputtypesize; j++) {
					data->value.s16[k+j] = __bswap_16(data->value.s16[k+j]);

					if (dumpvalue)
						out << "  value[" << k+j << "]: " << data->value.s16[k+j] << std::endl;
				}
				k += data->header.size / inputtypesize;
			}
			break;

		case Extractor::GPMF_TYPE_UNSIGNED_SHORT: // 0x53
			inputtypesize = 2;
			for (i=0, k=0; i<data->header.count; i++) {
				for (j=0; j<data->header.size / inputtypesize; j++) {
					data->value.u16[k+j] = __bswap_16(data->value.u16[k+j]);

					if (dumpvalue)
						out << "  value[" << k+j << "]: " << data->value.u16[k+j] << std::endl;
				}
				k += data->header.size / inputtypesize;
			}
			break;

		case Extractor::GPMF_TYPE_SIGNED_LONG: // 0x6c
			inputtypesize = 4;
			for (i=0, k=0; i<data->header.count; i++) {
				for (j=0; j<data->header.size / inputtypesize; j++) {
					data->value.s32[k+j] = __bswap_32(data->value.s32[k+j]);

					if (dumpvalue)
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

					if (dumpvalue)
						out << "  value[" << k+j << "]: " << data->value.u32[k+j] << std::endl;
				}
				k += data->header.size / inputtypesize;
			}
			break;

		case Extractor::GPMF_TYPE_UNSIGNED_64BIT_INT: // 0x4a
			inputtypesize = 8;
			for (i=0, k=0; i<data->header.count; i++) {
				for (j=0; j<data->header.size / inputtypesize; j++) {
					data->value.u64[k+j] = __bswap_64(data->value.u64[k+j]);

					if (dumpvalue)
						out << "  value: " << (uint64_t) data->value.u64[k+j] << std::endl;
				}
				k += data->header.size / inputtypesize;
			}
			break;

		case Extractor::GPMF_TYPE_FLOAT: // 0x66
			inputtypesize = 4;
			for (i=0, k=0; i<data->header.count; i++) {
				for (j=0; j<data->header.size / inputtypesize; j++) {
					float *result;

					uint32_t value = __bswap_32(data->value.u32[k+j]);

					result = (float *) &value;

					data->value.real[k+j] = *result;

					if (dumpvalue)
						out << "  value[" << k+j << "]: " << data->value.real[k+j] << std::endl;
				}
				k += data->header.size / inputtypesize;
			}
			break;

		case Extractor::GPMF_TYPE_DOUBLE:
			inputtypesize = 8;
			for (i=0, k=0; i<data->header.count; i++) {
				for (j=0; j<data->header.size / inputtypesize; j++) {
					data->value.u64[k+j] = __bswap_64(data->value.u64[k+j]);

					if (dumpvalue)
						out << "  value: " << (double) data->value.u64[k+j] << std::endl;
				}
				k += data->header.size / inputtypesize;
			}
			break;

		case Extractor::GPMF_TYPE_UTC_DATE_TIME: { // 0x55 16 bytes
				char *bytes = data->value.string;

				// 2020-12-13T09:56:27.000000Z
				// buffer contains: 201213085548.215
				// so format to : YY:MM:DD HH:MM:SS.mmmZ
				sprintf(string, "20%c%c-%c%c-%c%c %c%c:%c%c:%c%c.%c%c%cZ",
						bytes[0], bytes[1], // YY
						bytes[2], bytes[3], // MM 
						bytes[4], bytes[5], // DD
						bytes[6], bytes[7], // HH
						bytes[8], bytes[9], // MM
						bytes[10], bytes[11], // SS
						bytes[13], bytes[14], bytes[15] // MS
					);

				if (dumpvalue)
					out << "  value: " << string << std::endl;
			}
			break;

		default:
			break;
		}

		// Save data
		switch (key) {
		case STR2FOURCC("STRM"):
			nstream++;
			break;

		case STR2FOURCC("DVNM"):
			gpmd.device_name = string;
			break;

		case STR2FOURCC("GPSF"):
			if ((data->header.type == 'L') && data->header.count) {
				gpmd.fix = data->value.u32[0];
			}
			break;

		case STR2FOURCC("GPSU"):
			gpmd.date = string;
			break;

		case STR2FOURCC("STNM"): {
				// Ex:
				//   GPS (Lat., Long., Alt., 2D speed, 3D speed)
				//   Wind Processing[wind_enable, meter_value(0 - 100)]
				//   Microphone Wet[mic_wet, all_mics, confidence]
				//   AGC audio level[rms_level ,peak_level]
				char sep;

				std::string title(&(data->value.string[0]));

				if (title.back() == ']')
					sep = '[';
				else if (title.back() == ')')
					sep = '(';
				else
					sep = 0;

				if (sep != 0) {
					size_t pos = title.find(sep);

					title.pop_back();

					if (pos != std::string::npos) {
						size_t start = 0, end;

						title = title.substr(pos + 1);

						// Split string
						while ((end = title.find(',', start)) != std::string::npos) {
							std::string token = title.substr(start, end - start);

							token = Utils::ltrim(token);
							token = Utils::rtrim(token);

							titles.push_back(token);

							start = end + 1;
						}

						titles.push_back(title.substr(start));
					}
				}
			}
			break;

		case STR2FOURCC("SCAL"):
			for (i=0, k=0; i<data->header.count; i++) {
				const uint32_t value = data->value.u32[k];

				scales.push_back(value);

				k += data->header.size / inputtypesize;
			}
			break;
		
		case STR2FOURCC("UNIT"):
		case STR2FOURCC("SIUN"):
			for (i=0, k=0; i<data->header.count; i++) {
				const char *token = &(data->value.string[k]);

				memcpy(string, token, data->header.size / inputtypesize);
				string[data->header.size / inputtypesize] = '\0';

				units.push_back(string);

				k += data->header.size / inputtypesize;
			}
			break;

		case STR2FOURCC("GPS5"):
			for (i=0; i<data->header.count; i++) {
				// GPS point freq: 18 Hz
				// 1st point match on the GPSU value
				gpmd.lat = (double) data->value.s32[i] / (double) scale(0);
				gpmd.lon = (double) data->value.s32[i+1] / (double) scale(1);
				gpmd.ele = (double) data->value.s32[i+2] / (double) scale(2);

				// Process only the fist point
				break;
			}

		default:
			break;
		};

		// Dump info
		if (dumpinfo) {
			switch (key) {
			case STR2FOURCC("STRM"):
			case STR2FOURCC("DVNM"):
			case STR2FOURCC("GPSF"):
			case STR2FOURCC("GPSU"):
			case STR2FOURCC("SCAL"):
			case STR2FOURCC("UNIT"):
			case STR2FOURCC("SIUN"):
				// Don't print info
				break;

			case STR2FOURCC("ACCL"):
			case STR2FOURCC("GYRO"):
			case STR2FOURCC("MAGN"): {
					out << "  " << "TODO: Apply orientation matrix in/out (ORIO & ORIN)" << std::endl;
				}
				// fallthrough
				[[fallthrough]];

			case STR2FOURCC("GPS5"):
			case STR2FOURCC("GRAV"):
			case STR2FOURCC("CORI"):
			case STR2FOURCC("IORI"):
			case STR2FOURCC("ISOE"):
			case STR2FOURCC("SHUT"): {
					out << std::setprecision(12);

					for (i=0, k=0; i<data->header.count; i++) {
						out << "  " << format(3) << i   << ". ";

						for (j=0; j<data->header.size / inputtypesize; j++) {
							if (j > 0)
								out << ", ";

							out << title(j) << format(18) << (double) compute(data, k+j, (double) scale(j)) << unit(j);
						}

						out << std::endl;

						k += data->header.size / inputtypesize;
					}
				}
				break;

			default:
				break;
			};
		}

		if (data->header.type != 0x00) { 
			n += len;
			remain = len % 4;
			if (remain > 0)
				n += 4 - remain;
		}
	}

	if (nstream > 1) {
		Utils::IndentingOStreambuf indent(out, 4 * (nstream - 1));

		if (dump)
			out << "]" << std::endl;
	}
}

