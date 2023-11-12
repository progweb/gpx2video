#include "log.h"
#include "ffmpegutils.h"
#include "exportcodec.h"


std::string ExportCodec::getCodecName(Codec codec) {
	switch (codec) {
	case ExportCodec::CodecH264:
		return "H.264";
	case ExportCodec::CodecNVEncH264:
		return "NVidia H.264";
	case ExportCodec::CodecVAAPIH264:
		return "Intel H.264 via VAAPI";
	case ExportCodec::CodecQSVH264:
		return "Intel QSV H.264";
	case ExportCodec::CodecHEVC:
		return "HEVC";
	case ExportCodec::CodecNVEncHEVC:
		return "NVidia HEVC";
	case ExportCodec::CodecQSVHEVC:
		return "Intel QSV HEVC";
	
	case ExportCodec::CodecAAC:
		return "AAC";

	case ExportCodec::CodecCount:
	default:
		break;
	}

	return "Unknown";
}

