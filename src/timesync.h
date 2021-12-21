#ifndef __GPX2VIDEO__TIMESYNC_H__
#define __GPX2VIDEO__TIMESYNC_H__

#include <string>
#include <vector>

#include "media.h"
#include "decoder.h"
#include "gpx2video.h"


class GPMD {
public:
	uint32_t fix;
	std::string date;
};


class TimeSync : public GPX2Video::Task {
public:
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
		float real[0];
	} __attribute__((__packed__));

	struct gpmd_data {
		struct gpmd_header header;
		union gpmd_value value;
	} __attribute__((__packed__));

	virtual ~TimeSync();

	static TimeSync * create(GPX2Video &app);

	void run(void);

	const int& offset(void) const {
		return offset_;
	}

	static void parse(GPMD &gpmd, uint8_t *buffer, size_t size, bool dump=false);

private:
	GPX2Video &app_;

	MediaContainer *container_;

	TimeSync(GPX2Video &app);

	void init(void);

	int offset_;
};

#endif

