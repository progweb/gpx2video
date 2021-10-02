#ifndef __GPX2VIDEO__OIIOUTILS_H__
#define __GPX2VIDEO__OIIOUTILS_H__

#include <string>
#include <vector>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include <OpenImageIO/imageio.h>
#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>

#include "videoparams.h"
#include "frame.h"


class OIIOUtils {
public:
	static VideoParams::Format getFormatFromOIIOBaseType(OIIO::TypeDesc::BASETYPE type);
	static OIIO::TypeDesc::BASETYPE getOIIOBaseTypeFromFormat(enum VideoParams::Format format);

	static void frameToBuffer(const Frame* frame, OIIO::ImageBuf *buf);
	static void bufferToFrame(OIIO::ImageBuf *buf, const Frame *frame);
};

#endif

