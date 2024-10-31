#include "oiioutils.h"


VideoParams::Format OIIOUtils::getFormatFromOIIOBaseType(OIIO::TypeDesc::BASETYPE type) {
	switch (type) {
	case OIIO::TypeDesc::UNKNOWN:
	case OIIO::TypeDesc::NONE:
		break;

	case OIIO::TypeDesc::INT8:
	case OIIO::TypeDesc::INT16:
	case OIIO::TypeDesc::INT32:
	case OIIO::TypeDesc::UINT32:
	case OIIO::TypeDesc::INT64:
	case OIIO::TypeDesc::UINT64:
	case OIIO::TypeDesc::STRING:
#if OIIO_VERSION >= OIIO_MAKE_VERSION(2,5,0)
	case OIIO::TypeDesc::USTRINGHASH:
#endif
	case OIIO::TypeDesc::PTR:
	case OIIO::TypeDesc::LASTBASE:
	case OIIO::TypeDesc::DOUBLE:
		fprintf(stderr, "Tried to use unknown OIIO base type\n");
		break;

	case OIIO::TypeDesc::UINT8:
		return VideoParams::FormatUnsigned8;
	case OIIO::TypeDesc::UINT16:
		return VideoParams::FormatUnsigned16;
	case OIIO::TypeDesc::HALF:
		return VideoParams::FormatFloat16;
	case OIIO::TypeDesc::FLOAT:
		return VideoParams::FormatFloat32;
	}

	return VideoParams::FormatInvalid;
}


OIIO::TypeDesc::BASETYPE OIIOUtils::getOIIOBaseTypeFromFormat(enum VideoParams::Format format) {
	switch (format) {
	case VideoParams::FormatUnsigned8:
		return OIIO::TypeDesc::UINT8;
	case VideoParams::FormatUnsigned16:
		return OIIO::TypeDesc::UINT16;
	case VideoParams::FormatFloat16:
		return OIIO::TypeDesc::HALF;
	case VideoParams::FormatFloat32:
		return OIIO::TypeDesc::FLOAT;
	case VideoParams::FormatInvalid:
	case VideoParams::FormatCount:
		break;
	}

	return OIIO::TypeDesc::UNKNOWN;
}


void OIIOUtils::frameToBuffer(const Frame *frame, OIIO::ImageBuf *buf) {
	buf->set_pixels(OIIO::ROI(), 
			buf->spec().format, 
			frame->constData(), 
			OIIO::AutoStride, 
			frame->linesizeBytes());
}


void OIIOUtils::bufferToFrame(OIIO::ImageBuf *buf, const Frame *frame) {
	buf->get_pixels(OIIO::ROI(), 
			buf->spec().format, 
			frame->data(), 
			OIIO::AutoStride, 
			frame->linesizeBytes());
}

