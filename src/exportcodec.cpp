#include "log.h"
#include "ffmpegutils.h"
#include "exportcodec.h"


std::string ExportCodec::getCodecName(Codec codec) {
	switch (codec) {
	case ExportCodec::CodecH264:
		return "H.264";
	case ExportCodec::CodecHEVC:
		return "HEVC";
	
	case ExportCodec::CodecAAC:
		return "AAC";

	case ExportCodec::CodecCount:
	default:
		break;
	}

	return "Unknown";
}

