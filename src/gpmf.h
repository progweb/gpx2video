#ifndef __GPX2VIDEO__GPMF_H__
#define __GPX2VIDEO__GPMF_H__

#include <string>
#include <vector>

#include "stream.h"
#include "media.h"


class GPMFData {
public:
	GPMFData() {
		id = 0;
		pts = 0;
		timecode = 0;

		device_name = "";

		timestamp = 0;

		timelapse = 1;

		fix = 0;
		date = "";

		lat = lon = ele = 0.0;
	}

	void dump() {
		printf("  [%ld] GPMF Timestamp: %ld ms - Timelapse factor: %d\n",
				id,
				timestamp / 1000, 
				timelapse);
	}

	uint64_t id;
	uint64_t pts;
	uint64_t timecode;

	std::string device_name;

	uint64_t timestamp;

	int timelapse;

	uint32_t fix;
	int utc_ms;
	struct tm utc_time;
	std::string date;

	double lat;
	double lon;
	double ele;
};


class GPMFDecoder {
public:
	enum gpmd_type {
		GPMF_TYPE_STRING_ASCII = 'c',       // single byte 'c' style character string
		GPMF_TYPE_SIGNED_BYTE = 'b',        // single byte signed number
		GPMF_TYPE_UNSIGNED_BYTE = 'B',      // single byte unsigned number
		GPMF_TYPE_SIGNED_SHORT = 's',       // 16-bit integer
		GPMF_TYPE_UNSIGNED_SHORT = 'S',     // 16-bit integer
		GPMF_TYPE_FLOAT = 'f',              // 32-bit single precision float (IEEE 754)
		GPMF_TYPE_FOURCC = 'F',             // 32-bit four character tag 
		GPMF_TYPE_SIGNED_LONG = 'l',        // 32-bit integer
		GPMF_TYPE_UNSIGNED_LONG = 'L',      // 32-bit integer
		GPMF_TYPE_Q15_16_FIXED_POINT = 'q', // Q number Q15.16 - 16-bit signed integer (A) with 16-bit fixed point (B) for A.B value (range -32768.0 to 32767.99998). 
		GPMF_TYPE_Q31_32_FIXED_POINT = 'Q', // Q number Q31.32 - 32-bit signed integer (A) with 32-bit fixed point (B) for A.B value. 
		GPMF_TYPE_SIGNED_64BIT_INT = 'j',   // 64 bit signed long
		GPMF_TYPE_UNSIGNED_64BIT_INT = 'J', // 64 bit unsigned long  
		GPMF_TYPE_DOUBLE = 'd',             // 64 bit double precision float (IEEE 754)
		GPMF_TYPE_STRING_UTF8 = 'u',        // UTF-8 formatted text string.  As the character storage size varies, the size is in bytes, not UTF characters.
		GPMF_TYPE_UTC_DATE_TIME = 'U',      // 128-bit ASCII Date + UTC Time format yymmddhhmmss.sss - 16 bytes ASCII (years 20xx covered)
		GPMF_TYPE_GUID = 'G',               // 128-bit ID (like UUID)

		GPMF_TYPE_COMPLEX = '?',            // for sample with complex data structures, base size in bytes.  Data is either opaque, or the stream has a TYPE structure field for the sample.
		GPMF_TYPE_COMPRESSED = '#',         // Huffman compression STRM payloads.  4-CC <type><size><rpt> <data ...> is compressed as 4-CC '#'<new size/rpt> <type><size><rpt> <compresse

		GPMF_TYPE_NEST = 0,                 // used to nest more GPMF formatted metadata 

		/* ------------- Internal usage only ------------- */
		GPMF_TYPE_EMPTY = 0xfe,             // used to distinguish between grouped metadata (like FACE) with no data (no faces detected) and an empty payload (FACE device reported no sampl
		GPMF_TYPE_ERROR = 0xff              // used to report an error
	};

	struct gpmd_header {
		uint8_t label[4];
		uint8_t type;
		uint8_t size;
		uint16_t count;
	} __attribute__((__packed__));

	union gpmd_value {
		char string[0];
		int16_t s16[0];
		uint16_t u16[0];
		int32_t s32[0];
		uint32_t u32[0];
		int64_t s64[0];
		uint64_t u64[0];
		float real[0];
	} __attribute__((__packed__));

	struct gpmd_data {
		struct gpmd_header header;
		union gpmd_value value;
	} __attribute__((__packed__));

	virtual ~GPMFDecoder();

	static GPMFDecoder * create(void);

	bool open(StreamPtr stream);
	int getPacket(AVPacket *packet);
	void close(void);

	bool retrieveData(GPMFData &data, AVRational timecode);
	AVPacket * retrievePacketData(const int64_t& target_ts);
	bool parseData(GPMFData &data, uint8_t *buffer, size_t size);

protected:
	StreamPtr stream(void) const {
		return stream_;
	}

	bool open(const std::string &filename, const int &index);

private:
	GPMFDecoder();

	StreamPtr stream_;

	AVFormatContext *fmt_ctx_;

	AVStream *avstream_;

	uint64_t n_;
	int64_t pts_;
};

#endif

