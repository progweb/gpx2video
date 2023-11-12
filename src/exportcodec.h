#ifndef __GPX2VIDEO__EXPORTCODEC_H__
#define __GPX2VIDEO__EXPORTCODEC_H__

#include <iostream>
#include <memory>
#include <string>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}


class ExportCodec {
public:
	enum Codec {
		// Video codecs
		CodecH264,
		CodecNVEncH264,
		CodecHEVC,
		CodecNVEncHEVC,

		// Audio codecs
		CodecAAC,

		CodecCount,
	};

	static std::string getCodecName(Codec codec);
};

#endif

